#include "z3Wrapper.h"

#include "converter/cpp_converter.h"
#include "converter/verilog_converter.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"

#include <array>
#include <filesystem>
#include <fstream>
#include <memory>
#include <omp.h>
#include <stdlib.h>

namespace hal
{
    namespace z3_utils
    {
        z3Wrapper::z3Wrapper(std::unique_ptr<z3::context> ctx, std::unique_ptr<z3::expr> expr) : m_z3_wrapper_id(create_id()), m_ctx(std::move(ctx)), m_expr(std::move(expr))
        {
            this->extract_function_inputs();
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

                    if (start_index == std::string::npos + 1 || end_index == std::string::npos) {
                        log_debug("z3_utils", "Some variables seem to be of other format than net ids. Some wrapper functions are specifically desigend for net ids. Be careful when using the wrapper with this expression.");
                        continue;
                    }

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

        std::unordered_map<u32, double> z3Wrapper::get_boolean_influence(const u32 num_evaluations) const
        {
            std::unordered_map<u32, double> influences;

            // compile the c file
            std::string directory = "/tmp/boolean_influence_tmp/";
            std::filesystem::create_directory(directory);

            log_debug("z3_utils", "directory created");

            std::string filename = directory + "boolean_func_" + std::to_string(omp_get_thread_num()) + "_" + std::to_string(m_z3_wrapper_id) + ".c";

            log_debug("z3_utils", "creating file: {}", filename);

            if (!this->write_c_file(filename))
            {
                log_error("z3_utils", "error during file creation for boolean influence");
                return std::unordered_map<u32, double>();
            }

            log_debug("z3_utils", "file created: {}", filename);

            const std::string program_name    = filename.substr(0, filename.size() - 2);
            const std::string compile_command = "g++ -o " + program_name + " " + filename + " -O3";
            int res = system(compile_command.c_str());
            UNUSED(res);

            log_debug("z3_utils", "{}", compile_command);

            // run boolean function program for every input
            for (auto it = m_inputs_net_ids.begin(); it < m_inputs_net_ids.end(); it++)
            {
                const std::string run_command = program_name + " " + std::to_string(*it) + " " + std::to_string(num_evaluations) + " 2>&1";

                std::array<char, 128> buffer;
                std::string result;

                log_debug("z3_utils", "{}", run_command);

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
                double cv       =  (double)(count) / (double)(num_evaluations);

                log_debug("z3_utils", "calculation done");

                influences.insert({*it, cv});
            }

            // delete files and temp directory
            std::remove(filename.c_str());
            std::remove(program_name.c_str());

            //std::filesystem::remove(directory);

            log_debug("z3_utils", "returning influences");

            return influences;
        }

        std::string z3Wrapper::get_smt2_string() const
        {
            auto s = z3::solver(*m_ctx);
            s.add(*m_expr == m_ctx->bv_val(0, 1));
            return s.to_smt2();
        }

        bool z3Wrapper::write_verilog_file(const std::filesystem::path& path, const std::map<u32, bool>& control_mapping) const
        {
            // Convert z3 expr to boolean function in verilog
            auto converter    = VerilogConverter();
            converter.set_control_mapping(control_mapping);
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

            m_expr = std::make_unique<z3::expr>(m_expr->simplify());
        }

        void z3Wrapper::remove_static_inputs(const Netlist* nl) {
            auto new_expr = *m_expr;
            for (const auto& v : nl->get_vcc_gates()) {
                auto id = v->get_fan_out_nets().front()->get_id();
                z3::expr v_ex = m_ctx->bv_const(std::to_string(id).c_str(), 1);

                z3::expr_vector from_vec(*m_ctx);
                z3::expr_vector to_vec(*m_ctx);

                from_vec.push_back(m_ctx->bv_val(1, 1));
                to_vec.push_back(v_ex);

                new_expr = new_expr.substitute(from_vec, to_vec);
            }

            for (const auto& g : nl->get_gnd_gates()) {
                auto id = g->get_fan_out_nets().front()->get_id();
                z3::expr g_ex = m_ctx->bv_const(std::to_string(id).c_str(), 1);

                z3::expr_vector from_vec(*m_ctx);
                z3::expr_vector to_vec(*m_ctx);

                from_vec.push_back(m_ctx->bv_val(0, 1));
                to_vec.push_back(g_ex);

                new_expr = new_expr.substitute(from_vec, to_vec);
            }

            m_expr = std::make_unique<z3::expr>(new_expr);
        }

        z3::expr z3Wrapper::get_expr() const
        {
            return *m_expr;
        }
    }    // namespace z3_utils
}    // namespace hal
