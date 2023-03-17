#include "boolean_influence/plugin_boolean_influence.h"

#include "hal_core/netlist/decorators/boolean_function_net_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/netlist_utils.h"
#include "z3_utils/include/plugin_z3_utils.h"

#include <filesystem>
#include <fstream>

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<BooleanInfluencePlugin>();
    }

    std::string BooleanInfluencePlugin::get_name() const
    {
        return std::string("boolean_influence");
    }

    std::string BooleanInfluencePlugin::get_version() const
    {
        return std::string("0.1");
    }

    void BooleanInfluencePlugin::initialize()
    {
    }

    namespace
    {
        void add_inputs(Gate* gate, std::unordered_set<Gate*>& gates)
        {
            if (!gate->get_type()->has_property(GateTypeProperty::combinational) || gate->is_vcc_gate() || gate->is_gnd_gate())
            {
                return;
            }

            gates.insert(gate);
            for (const auto& pre : gate->get_predecessors())
            {
                if (pre && pre->get_gate() && gates.find(pre->get_gate()) == gates.end())
                {
                    add_inputs(pre->get_gate(), gates);
                }
            }
            return;
        }

        std::vector<Gate*> extract_function_gates(const Gate* start, const GatePin* pin)
        {
            std::unordered_set<Gate*> function_gates;

            auto pre = start->get_predecessor(pin);
            if (pre != nullptr && pre->get_gate() != nullptr)
            {
                add_inputs(pre->get_gate(), function_gates);
            }

            return {function_gates.begin(), function_gates.end()};
        }

    }    // namespace

    const std::string BooleanInfluencePlugin::probabilistic_function = R"(
#include <stdbool.h> 
#include <stdio.h>
#include <stdlib.h>

static unsigned long x=123456789, y=362436069, z=521288629;

// period 2^96-1
unsigned long xorshf96(void) {          
    unsigned long t;

    x ^= x << 16;
    x ^= x >> 5;
    x ^= x << 1;

    t = x;
    x = y;
    y = z;
    z = t ^ x ^ y;

    return z;
}

<C_FUNCTION>

const int input_size = <INPUT_SIZE>;

void build_values(bool* values) {
    for (int i = 0; i < input_size; i++) {
        bool random_value = xorshf96() % 2;
        values[i] = random_value;
    }

    return;
}

int main(int argc, char *argv[]) {
    unsigned long long b     = strtoull(argv[1], 0, 10);
    unsigned long long num   = strtoull(argv[2], 0, 10);
    unsigned long long count = 0;

    bool values[input_size];
    for (unsigned long long i = 0; i < num; i++) {
        build_values(values);

        values[b] = true;
        bool r1 = func(values);

        values[b] = false;
        bool r2 = func(values);

        if (r1 != r2) {
            count++;
        }
    }

    printf("%lld\n", count);

    return int(count);
})";

    const std::string BooleanInfluencePlugin::deterministic_function = R"(
#include <stdbool.h> 
#include <stdio.h>
#include <stdlib.h>

<C_FUNCTION>

const int input_size = <INPUT_SIZE>;

void build_values(bool* values, unsigned long long val) {
    for (int idx = 0; idx < input_size; idx++) {
        values[idx] = (val >> idx) & 0x1;
    }

    return;
}

int main(int argc, char *argv[]) {
    unsigned long long b     = strtoull(argv[1], 0, 10);
    unsigned long long count = 0;

    bool values[input_size];
    for (unsigned long long i = 0; i < (1 << input_size); i++) {
        build_values(values, i);

        values[b] = true;
        bool r1 = func(values);

        values[b] = false;
        bool r2 = func(values);

        if (r1 != r2) {
            count++;
        }
    }

    printf("%lld\n", count);

    return int(count);
})";

    Result<std::unordered_map<std::string, double>>
        BooleanInfluencePlugin::get_boolean_influence_internal(const z3::expr& expr, const u32 num_evaluations, const bool deterministic, const std::string& unique_identifier)
    {
        const auto to_replacement_var = [](const u32 var_idx) -> std::string { return "var_" + std::to_string(var_idx); };

        const auto extract_index = [](const std::string& var) -> Result<u32> {
            if (!utils::starts_with(var, std::string("var_")))
            {
                return ERR("variable " + var + " does not start with prefix 'var_', so we cannot extract an index.");
            }

            try
            {
                return OK(u32(std::stoul(var.substr(4))));
            }
            catch (const std::invalid_argument& e)
            {
                return ERR("could not get index from string '" + var + "': " + e.what());
            }
            catch (const std::out_of_range& e)
            {
                return ERR("could not get index from string '" + var + "': " + e.what());
            }
        };

        const auto to_original_var = [extract_index](const std::string& replacement_var, const std::vector<std::string>& original_vars) -> Result<std::string> {
            const auto var_idx_res = extract_index(replacement_var);
            if (var_idx_res.is_error())
            {
                return ERR_APPEND(var_idx_res.get_error(), "unable to reconstruct original variable from replacement variable" + replacement_var + ": failed to extract index.");
            }
            const auto var_idx = var_idx_res.get();

            if (var_idx > original_vars.size())
            {
                return ERR("unable to reconstruct original variable from replacement variable" + replacement_var + ": extracted index " + std::to_string(var_idx)
                           + " is bigger than the size of the original vars (" + std::to_string(original_vars.size()) + ").");
            }

            return OK(original_vars.at(var_idx));
        };

        std::unordered_map<std::string, double> influences;

        // substitute all variables in the expression to be of the format var_<INDEX>.
        // This allows for a more efficient translation into a c function
        std::vector<std::string> input_vars = utils::to_vector(z3_utils::get_variable_names(expr));

        if (deterministic && input_vars.size() > 16)
        {
            return ERR("unable to generate Boolean influence: Cannot evaluate Boolean function deterministically for more than 16 variables but got " + std::to_string(input_vars.size()));
        }

        std::vector<std::string> replacement_vars;

        z3::expr_vector from_vec(expr.ctx());
        z3::expr_vector to_vec(expr.ctx());

        for (u32 var_idx = 0; var_idx < input_vars.size(); var_idx++)
        {
            replacement_vars.push_back(to_replacement_var(var_idx));

            z3::expr from = expr.ctx().bv_const(input_vars.at(var_idx).c_str(), 1);
            z3::expr to   = expr.ctx().bv_const(to_replacement_var(var_idx).c_str(), 1);

            from_vec.push_back(from);
            to_vec.push_back(to);
        }

        auto replaced_e = expr;
        replaced_e      = replaced_e.substitute(from_vec, to_vec);

        // translate expression into a c program
        const std::filesystem::path directory = "/tmp/boolean_influence_tmp/";
        std::filesystem::create_directory(directory);

        const std::filesystem::path file_path = directory / (unique_identifier.empty() ? ("boolean_func.cpp") : ("boolean_func_" + unique_identifier + ".cpp"));

        std::string cpp_program = deterministic ? deterministic_function : probabilistic_function;
        cpp_program             = utils::replace(cpp_program, std::string("<C_FUNCTION>"), z3_utils::to_cpp(replaced_e));
        cpp_program             = utils::replace(cpp_program, std::string("<INPUT_SIZE>"), std::to_string(replacement_vars.size()));

        // write cpp program to file
        std::ofstream ofs(file_path);
        if (!ofs.is_open())
        {
            return ERR("unable to generate Boolean influence: could not open file " + file_path.string() + ".");
        }
        ofs << cpp_program;

        ofs.close();

        // compile the cpp file
        const std::string program_name    = file_path.string().substr(0, file_path.string().size() - 4);
        const std::string compile_command = "g++ -o " + program_name + " " + file_path.string() + " -O3";
        int res                           = system(compile_command.c_str());
        UNUSED(res);

        // run boolean function program for every input
        for (const auto& var : replacement_vars)
        {
            const auto idx_res = extract_index(var);
            if (idx_res.is_error())
            {
                return ERR_APPEND(idx_res.get_error(), "unable to generate Boolean influence: failed to extract index from variable " + var + ".");
            }
            const auto idx = idx_res.get();

            const std::string num_evaluations_str = deterministic ? "" : std::to_string(num_evaluations);

            const std::string run_command = program_name + " " + std::to_string(idx) + " " + num_evaluations_str + " 2>&1";

            std::array<char, 128> buffer;
            std::string result;

            FILE* pipe = popen(run_command.c_str(), "r");
            if (!pipe)
            {
                return ERR("unable to generate Boolean influence: error during execution of compiled boolean program");
            }
            while (fgets(buffer.data(), 128, pipe) != NULL)
            {
                result += buffer.data();
            }

            pclose(pipe);

            const u32 count            = std::stoi(result);
            const u32 real_evaluations = deterministic ? (1 << input_vars.size()) : num_evaluations;
            double cv                  = (double)(count) / (double)(real_evaluations);

            const auto org_var_res = to_original_var(var, input_vars);
            if (org_var_res.is_error())
            {
                return ERR_APPEND(org_var_res.get_error(), "unable to generate Boolean influence: failed to reconstruct original variable from replacement variable " + var + ".");
            }
            const auto org_var = org_var_res.get();

            influences.insert({org_var, cv});
        }

        // delete files and temp directory
        std::remove(file_path.string().c_str());
        std::remove(program_name.c_str());

        //std::filesystem::remove(directory);

        return OK(influences);
    }

    Result<std::map<Net*, double>> BooleanInfluencePlugin::get_boolean_influences_of_subcircuit_internal(const std::vector<Gate*>& gates,
                                                                                                         const Net* start_net,
                                                                                                         const u32 num_evaluations,
                                                                                                         const bool deterministic,
                                                                                                         const std::string& unique_identifier)
    {
        for (const auto* gate : gates)
        {
            if (!gate->get_type()->has_property(GateTypeProperty::combinational) || gate->is_vcc_gate() || gate->is_gnd_gate())
            {
                return ERR("unable to get Boolean influence for net " + start_net->get_name() + " with ID " + std::to_string(start_net->get_id()) + ": sub circuit gates include gate "
                           + gate->get_name() + " with ID " + std::to_string(gate->get_id()) + " that is either not a combinational gate or is a VCC / GND gate.");
            }
        }

        // Generate function for the data port
        auto ctx  = z3::context();
        auto func = z3::expr(ctx);

        if (!gates.empty())
        {
            const auto func_res = z3_utils::get_subgraph_z3_function(gates, start_net, ctx);
            if (func_res.is_error())
            {
                return ERR_APPEND(func_res.get_error(),
                                  "unable to get Boolean influence for net " + start_net->get_name() + " with ID " + std::to_string(start_net->get_id()) + ": failed to build subgraph function");
            }
            func = func_res.get();
        }
        // edge case if the gates are empty
        else
        {
            func = ctx.bv_const(BooleanFunctionNetDecorator(*start_net).get_boolean_variable_name().c_str(), 1);
        }

        // Generate Boolean influences
        const auto inf_res = get_boolean_influence_internal(func, num_evaluations, deterministic, unique_identifier);
        if (inf_res.is_error())
        {
            return ERR_APPEND(inf_res.get_error(),
                              "unable to get Boolean influence for net " + start_net->get_name() + " with ID " + std::to_string(start_net->get_id()) + ": failed to get boolean influence for net "
                                  + start_net->get_name() + " with ID " + std::to_string(start_net->get_id()) + ".");
        }
        const std::unordered_map<std::string, double> var_names_to_inf = inf_res.get();

        // translate net_ids back to nets
        std::map<Net*, double> nets_to_inf;

        Netlist* nl = start_net->get_netlist();
        for (const auto& [var_name, inf] : var_names_to_inf)
        {
            const auto net_res = BooleanFunctionNetDecorator::get_net_from(nl, var_name);
            if (net_res.is_error())
            {
                return ERR_APPEND(net_res.get_error(),
                                  "unable to get Boolean influence for net " + start_net->get_name() + " with ID " + std::to_string(start_net->get_id()) + ": failed to reconstruct net from variable "
                                      + var_name + ".");
            }
            const auto net = net_res.get();

            nets_to_inf.insert({net, inf});
        }

        return OK(nets_to_inf);
    }

    Result<std::map<Net*, double>>
        BooleanInfluencePlugin::get_boolean_influences_of_gate_internal(const Gate* gate, const u32 num_evaluations, const bool deterministic, const std::string& unique_identifier)
    {
        if (!gate->get_type()->has_property(GateTypeProperty::ff))
        {
            return ERR("unable to get Boolean influence for gate " + gate->get_name() + " with ID " + std::to_string(gate->get_id()) + ": can only handle flip-flops but found gate type "
                       + gate->get_type()->get_name() + ".");
        }

        // Check for the data port pin
        auto d_pins = gate->get_type()->get_pins([](const GatePin* p) { return p->get_direction() == PinDirection::input && p->get_type() == PinType::data; });
        if (d_pins.size() != 1)
        {
            return ERR("unable to get Boolean influence for gate " + gate->get_name() + " with ID " + std::to_string(gate->get_id())
                       + ": can only handle flip-flops with exactly one data port, but found " + std::to_string(d_pins.size()) + ".");
        }
        const GatePin* data_pin = d_pins.front();

        // Extract all gates in front of the data port and iterate backwards until another flip flop is found.
        const auto function_gates = extract_function_gates(gate, data_pin);
        const auto in_net         = gate->get_fan_in_net(data_pin);

        // Generate Boolean influences
        const auto inf_res = get_boolean_influences_of_subcircuit_internal(function_gates, in_net, num_evaluations, deterministic, unique_identifier);
        if (inf_res.is_error())
        {
            return ERR_APPEND(inf_res.get_error(),
                              "unable to get Boolean influence for gate " + gate->get_name() + " with ID " + std::to_string(gate->get_id()) + ": failed to get Boolean influence for data net "
                                  + in_net->get_name() + " with ID " + std::to_string(in_net->get_id()) + ".");
        }

        return inf_res;
    }

    Result<std::unordered_map<std::string, double>> BooleanInfluencePlugin::get_boolean_influence(const BooleanFunction& bf, const u32 num_evaluations, const std::string& unique_identifier)
    {
        auto ctx         = z3::context();
        const auto z3_bf = z3_utils::from_bf(bf, ctx);

        return get_boolean_influence(z3_bf, num_evaluations, unique_identifier);
    }

    Result<std::unordered_map<std::string, double>> BooleanInfluencePlugin::get_boolean_influence(const z3::expr& expr, const u32 num_evaluations, const std::string& unique_identifier)
    {
        return get_boolean_influence_internal(expr, num_evaluations, false, unique_identifier);
    }

    Result<std::unordered_map<std::string, double>> BooleanInfluencePlugin::get_boolean_influence_deterministic(const BooleanFunction& bf, const std::string& unique_identifier)
    {
        auto ctx         = z3::context();
        const auto z3_bf = z3_utils::from_bf(bf, ctx);

        return get_boolean_influence_deterministic(z3_bf, unique_identifier);
    }

    Result<std::unordered_map<std::string, double>> BooleanInfluencePlugin::get_boolean_influence_deterministic(const z3::expr& expr, const std::string& unique_identifier)
    {
        return get_boolean_influence_internal(expr, 0, true, unique_identifier);
    }

    Result<std::map<Net*, double>>
        BooleanInfluencePlugin::get_boolean_influences_of_subcircuit(const std::vector<Gate*>& gates, const Net* start_net, const u32 num_evaluations, const std::string& unique_identifier)
    {
        return get_boolean_influences_of_subcircuit_internal(gates, start_net, num_evaluations, false, unique_identifier);
    }

    Result<std::map<Net*, double>> BooleanInfluencePlugin::get_boolean_influences_of_gate(const Gate* gate, const u32 num_evaluations, const std::string& unique_identifier)
    {
        return get_boolean_influences_of_gate_internal(gate, num_evaluations, false, unique_identifier);
    }

    Result<std::map<Net*, double>>
        BooleanInfluencePlugin::get_boolean_influences_of_subcircuit_deterministic(const std::vector<Gate*>& gates, const Net* start_net, const std::string& unique_identifier)
    {
        return get_boolean_influences_of_subcircuit_internal(gates, start_net, 0, true, unique_identifier);
    }

    Result<std::map<Net*, double>> BooleanInfluencePlugin::get_boolean_influences_of_gate_deterministic(const Gate* gate, const std::string& unique_identifier)
    {
        return get_boolean_influences_of_gate_internal(gate, 0, true, unique_identifier);
    }

    Result<std::pair<std::map<u32, Gate*>, std::vector<std::vector<double>>>> BooleanInfluencePlugin::get_ff_dependency_matrix(const Netlist* nl, bool with_boolean_influence)
    {
        std::map<u32, Gate*> matrix_id_to_gate;
        std::map<Gate*, u32> gate_to_matrix_id;
        std::vector<std::vector<double>> matrix;

        std::unordered_map<u32, std::vector<Gate*>> cache;

        u32 matrix_gates = 0;
        for (const auto& gate : nl->get_gates())
        {
            if (!gate->get_type()->has_property(GateTypeProperty::ff))
            {
                continue;
            }
            gate_to_matrix_id[gate]         = matrix_gates;
            matrix_id_to_gate[matrix_gates] = gate;
            matrix_gates++;
        }

        u32 status_counter = 0;
        for (const auto& [id, gate] : matrix_id_to_gate)
        {
            if (status_counter % 100 == 0)
            {
                log_info("boolean_influence", "status {}/{} processed", status_counter, matrix_id_to_gate.size());
            }
            status_counter++;
            std::vector<double> line_of_matrix;

            std::set<u32> gates_to_add;
            for (const auto& pred_gate : netlist_utils::get_next_sequential_gates(gate, false, cache))
            {
                gates_to_add.insert(gate_to_matrix_id[pred_gate]);
            }
            std::map<Net*, double> boolean_influence_for_gate;
            if (with_boolean_influence)
            {
                const auto inf_res = get_boolean_influences_of_gate(gate);
                if (inf_res.is_error())
                {
                    return ERR_APPEND(inf_res.get_error(),
                                      "unable to generate ff dependency matrix: failed to generate Boolean influence for gate " + gate->get_name() + " with ID " + std::to_string(gate->get_id())
                                          + ".");
                }
                boolean_influence_for_gate = inf_res.get();
            }

            for (u32 i = 0; i < matrix_gates; i++)
            {
                if (gates_to_add.find(i) != gates_to_add.end())
                {
                    if (with_boolean_influence)
                    {
                        double influence = 0.0;

                        Gate* pred_ff = matrix_id_to_gate[i];

                        for (const auto& output_net : pred_ff->get_fan_out_nets())
                        {
                            if (boolean_influence_for_gate.find(output_net) != boolean_influence_for_gate.end())
                            {
                                influence += boolean_influence_for_gate[output_net];
                            }
                        }

                        line_of_matrix.push_back(influence);
                    }
                    else
                    {
                        line_of_matrix.push_back(1.0);
                    }
                }
                else
                {
                    line_of_matrix.push_back(0.0);
                }
            }
            matrix.push_back(line_of_matrix);
        }

        return OK(std::make_pair(matrix_id_to_gate, matrix));
    }

}    // namespace hal
