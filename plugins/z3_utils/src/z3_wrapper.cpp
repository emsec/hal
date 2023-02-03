#include "z3_wrapper.h"

#include "plugin_z3_utils.h"

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
            m_var_names.clear();
            m_inputs_z3_expr.clear();

            m_var_names = utils::to_vector(z3_utils::get_variable_names(*m_expr));

            for (const auto& var : m_var_names)
            {
                z3::expr z3_expr = m_ctx->bv_const(var.c_str(), 1);
                m_inputs_z3_expr.push_back(z3_expr);
                m_input_mapping_hal_to_z3.insert({var, z3_expr});
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

        /*
        std::unordered_map<std::string, z3::expr> z3Wrapper::get_input_mapping_hal_to_z3() const
        {
            return m_input_mapping_hal_to_z3;
        }
        */

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

        std::vector<std::string> z3Wrapper::get_variable_names() const
        {
            return m_var_names;
        }

        /* 
        std::unordered_map<std::string, double> z3Wrapper::get_boolean_influence(const u32 num_evaluations) const
        {
            const auto res = BooleanInfluencePlugin::get_boolean_influence(*m_expr, num_evaluations);

            if (res.is_error())
            {
                log_error("z3_utils", "{}", res.get_error().get());
                return {};
            }

            return res.get();
        }
        */

        std::string z3Wrapper::get_smt2_string() const
        {
            /*
            auto s = z3::solver(*m_ctx);
            s.add(*m_expr == m_ctx->bv_val(0, 1));
            return s.to_smt2();
            */
            return z3_utils::to_smt2(*m_expr);
        }

        bool z3Wrapper::write_verilog_file(const std::filesystem::path& path, const std::map<std::string, bool>& control_mapping) const
        {
            // Convert z3 expr to boolean function in verilog
            auto verilog_file = z3_utils::to_verilog(*m_expr, control_mapping);

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
            // Convert z3 expr to boolean function in cpp
            const auto c_file = z3_utils::to_cpp(*m_expr);

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

        /*
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
            extract_function_inputs();
        }
        */

        void z3Wrapper::remove_static_inputs(const Netlist* nl)
        {
            auto new_expr = *m_expr;
            for (const auto& v : nl->get_vcc_gates())
            {
                auto id       = v->get_fan_out_nets().front()->get_id();
                z3::expr v_ex = m_ctx->bv_const(std::to_string(id).c_str(), 1);

                z3::expr_vector from_vec(*m_ctx);
                z3::expr_vector to_vec(*m_ctx);

                from_vec.push_back(m_ctx->bv_val(1, 1));
                to_vec.push_back(v_ex);

                new_expr = new_expr.substitute(to_vec, from_vec);
            }

            for (const auto& g : nl->get_gnd_gates())
            {
                auto id       = g->get_fan_out_nets().front()->get_id();
                z3::expr g_ex = m_ctx->bv_const(std::to_string(id).c_str(), 1);

                z3::expr_vector from_vec(*m_ctx);
                z3::expr_vector to_vec(*m_ctx);

                from_vec.push_back(m_ctx->bv_val(0, 1));
                to_vec.push_back(g_ex);

                new_expr = new_expr.substitute(to_vec, from_vec);
            }

            m_expr = std::make_unique<z3::expr>(new_expr.simplify());
            extract_function_inputs();
        }

        std::vector<BooleanFunction::Value> z3Wrapper::generate_truth_table() const
        {
            std::vector<BooleanFunction::Value> tt(1 << m_var_names.size());

            for (u32 input = 0; input < (u32)(1 << m_var_names.size()); input++)
            {
                z3::expr eval_expr = *m_expr;

                z3::expr_vector from_vec(*m_ctx);
                z3::expr_vector to_vec(*m_ctx);

                for (u32 index = 0; index < m_var_names.size(); index++)
                {
                    z3::expr val = ((input >> index) & 1) ? m_ctx->bv_val(1, 1) : m_ctx->bv_val(0, 1);
                    z3::expr var = m_input_mapping_hal_to_z3.at(m_var_names.at(index));

                    from_vec.push_back(val);
                    to_vec.push_back(var);
                }

                eval_expr = eval_expr.substitute(to_vec, from_vec).simplify();

                if (!eval_expr.is_numeral())
                {
                    log_error("z3_utils", "after substituting every input the expr is not constant: {}", eval_expr.to_string());
                }

                BooleanFunction::Value eval = (eval_expr.get_numeral_uint() == 1) ? BooleanFunction::Value::ONE : BooleanFunction::Value::ZERO;
                tt.at(input)                = eval;
            }

            return tt;
        }

        z3::expr z3Wrapper::get_expr() const
        {
            return *m_expr;
        }
    }    // namespace z3_utils
}    // namespace hal
