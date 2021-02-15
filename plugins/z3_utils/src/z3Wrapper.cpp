#include "z3Wrapper.h"

#include "converter/cpp_converter.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"
#include "converter/cpp_converter.h"
#include "converter/verilog_converter.h"

#include <array>
#include <fstream>
#include <memory>
#include <omp.h>

namespace hal
{
    namespace z3_utils
    {
        z3Wrapper::z3Wrapper(std::unique_ptr<z3::context> ctx, std::unique_ptr<z3::expr> expr) : m_ctx(std::move(ctx)), m_expr(std::move(expr)), m_z3_wrapper_id(create_id())
        {
            this->extract_function_inputs();
        }

        bool z3Wrapper::operator==(const z3Wrapper& other) const
        {
            if (m_inputs_net_ids.size() != other.m_inputs_net_ids.size()) {
                return false;
            }

            /*
             * This acts a timeout value.
             */
            u32 max_guesses = 1;

            // Have you heard of the boolean influence?      
            /*
            auto bf_x = get_boolean_influence();
            auto bf_y = other.get_boolean_influence();

            std::cout << "Boolean influence X:" << std::endl;
            for (const auto& [net_id, inf] : bf_x) {
                std::cout << net_id << ": " << inf << std::endl;
            }

            std::cout << "Boolean influence Y:" << std::endl;
            for (const auto& [net_id, inf] : bf_y) {
                std::cout << net_id << ": " << inf << std::endl;
            }
            */
           
           z3::context comp_ctx;

           z3::expr other_trans_expr(comp_ctx);
           z3::expr original_trans_expr(comp_ctx);

#pragma omp critical 
{
            other_trans_expr = other.get_expr_in_ctx(comp_ctx);
            original_trans_expr = this->get_expr_in_ctx(comp_ctx);
}
            z3::solver s = {comp_ctx};

            // Create a solver using "qe" and "smt" tactics
            /*
            z3::solver s = 
                (z3::tactic(comp_ctx, "qe") &
                z3::tactic(comp_ctx, "smt")).mk_solver();
            */

            z3::expr_vector x_ids(comp_ctx);
            z3::expr x_vals(comp_ctx);

            std::vector<Z3_app> vars;

            z3::expr constraint_expr(comp_ctx);

            // Build mapping constraint
            for (const auto& x_id : m_inputs_net_ids) {
                auto x_expr = comp_ctx.bv_const(std::to_string(x_id).c_str(), 1);

                std::string x_int_name = std::to_string(x_id) + "_id";

                auto x_id_expr = comp_ctx.int_const(x_int_name.c_str());

                z3::expr n_expr(comp_ctx);
                for (const auto& y_id : other.m_inputs_net_ids) {
                    z3::expr y_expr = comp_ctx.bv_const(std::to_string(y_id).c_str(), 1);

                    std::string y_int_name = std::to_string(y_id) + "_id";

                    //auto y_id_expr = comp_ctx.int_const(y_int_name.c_str());
                    auto y_id_val = comp_ctx.int_val(y_id);

                    z3::expr value_constraint = x_expr == y_expr;
                    z3::expr id_constraint = x_id_expr == y_id_val;

                    if (n_expr.to_string() == "null") {
                        n_expr = (value_constraint && id_constraint); 
                    } else {
                        n_expr = n_expr || (value_constraint && id_constraint);
                    }
                }
                
                if (constraint_expr.to_string() == "null") {
                    constraint_expr = n_expr;
                } else {
                    constraint_expr = constraint_expr && n_expr;
                }

                if (x_vals.to_string() == "null") {
                    x_vals = x_expr;
                } else {
                    x_vals = z3::concat(x_vals, x_expr);
                }

                //vars.push_back((Z3_app)x_expr);
                x_ids.push_back(x_id_expr);
            }


            //std::cout << x_vals << std::endl;
            //std::cout << z3::distinct(x_ids) << std::endl;
            //std::cout << "Constraint: " << constraint_expr << std::endl;

            //std::cout << "LHS: " << m_expr->simplify().to_string() << std::endl;
            //std::cout << "RHS: " << o_trans_expr.simplify().to_string() << std::endl;

            s.add(constraint_expr);
            s.add(z3::distinct(x_ids));

            // in order to prevent the solver from finding only edgecases where the functions behave identically check random testcases..
            srand(0x1337);
            z3::expr test_vals = comp_ctx.bv_val(rand()%2, 1);
            for (u32 i = 1; i < m_inputs_net_ids.size(); i++) {
                test_vals = z3::concat(test_vals, comp_ctx.bv_val(rand()%2, 1));
            }

            z3::expr f = original_trans_expr == other_trans_expr;

            // We have to use the C API directly for creating quantified formulas.
            /*
            z3::expr qf = z3::to_expr(*m_ctx, Z3_mk_forall_const(*m_ctx, 0, vars.size(), vars.data(),
                                                                0, 0, // no pattern
                                                                (Z3_ast)f));
            */

            // Here for the for all shit
            // s.add(qf);
            // auto c1 = s.check();
            // std::cout << s.to_smt2() << std::endl;
            // std::cout << c1 << std::endl;
            /*
            if (c1 == z3::sat) {
                return true;
            }
            
            return false;
            */

            u32 guesses = 0;
            while (true) {
                guesses++;

                if (guesses % 1000 == 0) {
                    std::cout << guesses << std::endl;
                }

                if (guesses > max_guesses) {
                    log_debug("z3_utils", "Timeouted.");
                    return false;
                }

                s.push();

                s.add(x_vals == test_vals);
                s.add(original_trans_expr == other_trans_expr);

                z3::check_result c1 = s.check();

                if (c1 != z3::sat) {
                    // z3::model m = s.get_model();
                    // //std::cout << m << std::endl;
                    log_debug("z3_utils", "Cannot find a valid permutation under which the expressions are equal.");

                    return false;
                }

                log_debug("z3_utils", "Found valid mapping.");

                // evaluate found mapping
                std::vector<std::pair<z3::expr, z3::expr>> found_mapping;

                z3::model m1 = s.get_model();

                // std::cout << "Mapping: " << std::endl;
                for (const auto& x_i : x_ids) {
                    found_mapping.push_back({x_i, m1.eval(x_i)});
                    // std::cout << x_i << ": " << m1.eval(x_i) << std::endl;
                }

                s.pop();
                s.push();

                // try to verify found mapping
                for (const auto& p : found_mapping) {
                    s.add(p.first == p.second);
                }

                s.add(original_trans_expr != other_trans_expr);

                z3::check_result c2 = s.check();
                if (c2 == z3::unsat) {
                    log_info("z3_utils", "Took {} guesses to find correct permutation.", guesses);
                    break;
                }

                log_debug("z3_utils", "Failed to verify that the found permutation leads to equal expressions.");

                // set the next testval to the val the last permutation converged for
                z3::model m2 = s.get_model();
                test_vals = m2.eval(x_vals);

                s.pop();

                // forbid found mapping
                z3::expr forbidden_mapping(comp_ctx);
                for (const auto& p : found_mapping) {
                    if (forbidden_mapping.to_string() == "null") {
                        forbidden_mapping = p.first != p.second;
                    } else {
                        forbidden_mapping = forbidden_mapping || p.first != p.second;
                    }
                }
                s.add(forbidden_mapping);
            }

            return true;
        }

        u32 z3Wrapper::create_id()
        {
            static std::atomic<std::uint32_t> uid{0};
            return ++uid;
        }

        u32 z3Wrapper::get_id() const
        {
            return m_z3_wrapper_id;
        }

        z3::expr z3Wrapper::get_expr_in_ctx(z3::context& ctx) const
        {
            auto expr_vec = ctx.parse_string(get_smt2_string().c_str());
            return expr_vec.back().arg(0).simplify();
        }

        bool z3Wrapper::extract_function_inputs()
        {
            // clear input maps and vectors
            m_input_mapping_hal_to_z3.clear();
            m_inputs_net_ids.clear();
            m_inputs_z3_expr.clear();
            //m_input_mapping_z3_to_hal.clear();

            // get inputs from smt2 string, much faster than iterating over z3 things
            auto smt = get_smt2_string();

            std::istringstream iss(smt);
            for (std::string line; std::getline(iss, line);)
            {
                if (line.find("declare-fun") != std::string::npos)
                {
                    auto start_index = line.find_first_of('|') + 1;    // variable name starts after the '|'
                    auto end_index   = line.find_first_of('|', start_index);

                    auto var_name = line.substr(start_index, end_index - start_index);
                    auto net_id   = std::stoi(var_name);
                    auto z3_expr  = m_ctx->bv_const(var_name.c_str(), 1);
                    m_inputs_net_ids.push_back(net_id);
                    m_inputs_z3_expr.push_back(z3_expr);
                    m_input_mapping_hal_to_z3.insert({net_id, z3_expr});
                    //m_input_mapping_z3_to_hal.insert({z3_expr, net_id});
                }
            }

            return true;
        }

        /*
        bool z3Wrapper::optimize()
        {
            if (m_expr->id() == m_ctx->bv_val(0, 1).id() || m_expr->id() == m_ctx->bv_val(1, 1).id()) {
                return true;
            }

            std::string filename = "func_" + std::to_string(omp_get_thread_num()) + "_" + std::to_string(m_z3_wrapper_id) + ".v";
            if (!this->write_verilog_file(filename))
            {
                log_error("z3_utils", "error during file creation for optimize, nothing has been optimized");
                return false;
            }

            // call abc
            OptimizeHelper::call_abc(filename.c_str());

            // set optimized function
            m_expr = OptimizeHelper::eqn_to_z3_expr(m_ctx.get(), filename + ".eqn", m_inputs_z3_expr);

            this->extract_function_inputs();
            return true;
        }
        */

        std::unordered_map<u32, z3::expr> z3Wrapper::get_input_mapping_hal_to_z3() const
        {
            return m_input_mapping_hal_to_z3;
        }

        /*
        std::unordered_map<z3::expr, u32> z3Wrapper::get_input_mapping_z3_to_hal() const
        {
            return m_input_mapping_z3_to_hal;
        }
        */

        std::vector<z3::expr> z3Wrapper::get_inputs_z3_expr() const
        {
            return m_inputs_z3_expr;
        }

        std::vector<u32> z3Wrapper::get_inputs_net_ids() const
        {
            return m_inputs_net_ids;
        }

        std::unordered_map<u32, double> z3Wrapper::get_boolean_influence() const
        {
            std::unordered_map<u32, double> influences;
            const u32 evaluation_count = 32000;

            // compile the c file
            std::string directory = "/tmp/boolean_influence_tmp/";
            std::filesystem::create_directory(directory);

            log_debug("boolean_function", "directory created");

            std::string filename = directory + "boolean_func_" + std::to_string(omp_get_thread_num()) + "_" + std::to_string(m_z3_wrapper_id) + ".c";

            log_debug("boolean_function", "creating file: {}", filename);

            if (!this->write_c_file(filename))
            {
                log_error("z3_utils", "error during file creation for boolean influence");
                return std::unordered_map<u32, double>();
            }

            log_debug("boolean_function", "file created: {}", filename);

            const std::string program_name    = filename.substr(0, filename.size() - 2);
            const std::string compile_command = "g++ -o " + program_name + " " + filename + " -O3";
            system(compile_command.c_str());

            log_debug("boolean_function", "{}", compile_command);

            // run boolean function program for every input
            for (auto it = m_inputs_net_ids.begin(); it < m_inputs_net_ids.end(); it++)
            {
                const std::string run_command = program_name + " " + std::to_string(*it) + " " + std::to_string(evaluation_count) + " 2>&1";

                std::array<char, 128> buffer;
                std::string result;

                log_debug("boolean_function", "{}", run_command);

                FILE* pipe = popen(run_command.c_str(), "r");
                if (!pipe)
                {
                    log_error("z3_utils", "error during execution of program for boolean influence");
                }
                while (fgets(buffer.data(), 128, pipe) != NULL)
                {
                    result += buffer.data();
                }

                pclose(pipe);

                const u32 count = std::stoi(result);
                double cv       = double(count) / double(evaluation_count);

                log_debug("boolean_function", "calculation done");

                influences.insert({*it, cv});
            }

            // delete files and temp directory
            std::remove(filename.c_str());
            std::remove(program_name.c_str());

            //std::filesystem::remove(directory);

            log_debug("boolean_function", "returning influences");

            return influences;
        }

        std::string z3Wrapper::get_smt2_string() const
        {
            auto s = z3::solver(*m_ctx);
            s.add(*m_expr == m_ctx->bv_val(0, 1));
            return s.to_smt2();
        }

        bool z3Wrapper::write_verilog_file(const std::filesystem::path& path) const
        {
            // Convert z3 expr to boolean function in verilog
            auto converter    = VerilogConverter();
            auto verilog_file = converter.convert_z3_expr_to_func(*this);

            // parse out verilog function into file
            std::ofstream ofs(path);
            if (!ofs.is_open())
            {
                log_error("z3_utils", "could not open file '{}' for writing.", path.string());
                return false;
            }

            ofs << verilog_file;

            ofs.close();
            return true;
        }

        bool z3Wrapper::write_c_file(const std::filesystem::path& path) const
        {
            // Convert z3 expr to boolean function in c
            const auto converter = Cpp_Converter();
            std::string c_file   = converter.convert_z3_expr_to_func(*this);

            log_debug("z3_utils", "trying to write file");

            // parse out c funtion into file
            std::ofstream ofs(path);
            if (!ofs.is_open())
            {
                log_error("z3_utils", "could not open file '{}' for writing.", path.string());
                return false;
            }
            ofs << c_file;

            ofs.close();
            return true;
        }

        void z3Wrapper::remove_global_inputs(const Netlist* nl)
        {
            auto vcc_gates = nl->get_vcc_gates();
            auto gnd_gates = nl->get_gnd_gates();

            std::map<z3::expr, z3::expr> replacements;

            for (const auto& gate : vcc_gates)
            {
                u32 net_id    = gate->get_fan_out_nets().front()->get_id();
                auto net_expr = m_ctx->bv_const(std::to_string(net_id).c_str(), 1);
                replacements.insert({net_expr, m_ctx->bv_val(1, 1)});
            }

            for (const auto& gate : gnd_gates)
            {
                u32 net_id    = gate->get_fan_out_nets().front()->get_id();
                auto net_expr = m_ctx->bv_const(std::to_string(net_id).c_str(), 1);
                replacements.insert({net_expr, m_ctx->bv_val(0, 1)});
            }

            for (const auto& [from, to] : replacements)
            {
                z3::expr_vector from_vec(*m_ctx);
                z3::expr_vector to_vec(*m_ctx);

                from_vec.push_back(from);
                to_vec.push_back(to);
                m_expr = std::make_unique<z3::expr>(m_expr->substitute(from_vec, to_vec));
            }
        }

        z3::expr z3Wrapper::get_expr() const
        {
            return *m_expr;
        }
    }    // namespace z3_utils
}    // namespace hal
