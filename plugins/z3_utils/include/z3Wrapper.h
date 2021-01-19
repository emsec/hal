#pragma once

#include "hal_core/defines.h"
#include "z3++.h"

#include <atomic>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace hal
{
    /* forward declaration */
    class Netlist;
    
    namespace z3_utils 
    {
        class z3Wrapper
        {
        public:
            z3Wrapper(std::unique_ptr<z3::context> ctx, z3::expr& expr);

            // Specifc Function used to optimize z3 epxr with the help of ABC
            // bool optimize();

            std::unordered_map<u32, double> get_boolean_influence() const;

            std::string get_smt2_string() const;

            u32 get_id() const;

            z3::expr get_expr() const; ////////////

            z3::expr get_expr_in_ctx(z3::context& ctx) const;

            std::unordered_map<u32, z3::expr> get_input_mapping_hal_to_z3() const;
            std::vector<z3::expr> get_inputs_z3_expr() const;
            std::vector<u32> get_inputs_net_ids() const;

            bool write_verilog_file(const std::filesystem::path& path) const;

            bool write_c_file(const std::filesystem::path& path) const;

            void remove_global_inputs(const Netlist* nl);

        private:
            u32 m_z3_wrapper_id;
            static u32 create_id();

            bool extract_function_inputs();

            std::unique_ptr<z3::context> m_ctx;
            z3::expr m_expr;

            std::unordered_map<u32, z3::expr> m_input_mapping_hal_to_z3;
            std::vector<z3::expr> m_inputs_z3_expr;
            std::vector<u32> m_inputs_net_ids;
        };
    }    // namespace z3_utils
}    // namespace hal