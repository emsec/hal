#pragma once

#include "hal_core/defines.h"

#include "converter/converter.h"

namespace hal
{
    namespace z3_utils 
    {
        class VerilogConverter : public Converter
        {
        public:
            void set_control_mapping(const std::map<u32, bool>& control_mapping);

        private:
            // VIRTUAL METHODS
            std::string build_operand(const std::string& operand) const override;
            std::string build_operation(const Operation& operation, const std::vector<std::string>& operands) const override;
            std::string generate_assignment(const std::string& l) const override;
            std::string generate_initialization(const std::vector<u32>& inputs) const override;
            std::string construct_function(const std::string& assignments, const std::string& initalization, const std::vector<u32>& inputs) const override;

            //std::vector<OPERATION> m_operations;

            std::map<u32, bool> m_control_mapping;
        };
    }  // namespace z3_utils
}    // namespace hal
