// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "z3_utils/converter/converter.h"

namespace hal
{
    namespace z3_utils
    {
        /**
         * Converts a Boolean function given in SMT-LIB notation into Verilog source code.
         */
        class VerilogConverter : public Converter
        {
        public:
            /**
             * Set the values that the control signals are fixed to while the function is translated.
             *
             * @param[in] control_mapping - A map from the name of a control signal to the value it is fixed to.
             */
            void set_control_mapping(const std::map<std::string, bool>& control_mapping);

        private:
            // VIRTUAL METHODS
            /**
             * Translate a single operand into Verilog.
             *
             * @param[in] operand - The operand in SMT-LIB notation.
             * @returns The operand in Verilog.
             */
            std::string build_operand(const std::string& operand) const override;

            /**
             * Translate a single operation and its operands into Verilog.
             *
             * @param[in] operation - The operation to translate.
             * @param[in] operands - The operands of the operation, already translated.
             * @returns The operation in Verilog.
             */
            std::string build_operation(const Operation& operation, const std::vector<std::string>& operands) const override;

            /**
             * Translate a single assignment of the SMT-LIB representation into Verilog.
             *
             * @param[in] l - The line holding the assignment.
             * @returns The assignment in Verilog.
             */
            std::string generate_assignment(const std::string& l) const override;

            /**
             * Generate the Verilog code that reads the input variables before the translated function is evaluated.
             *
             * @param[in] input_vars - The input variables of the function.
             * @returns The initialization code in Verilog.
             */
            std::string generate_initialization(const std::vector<std::string>& input_vars) const override;

            /**
             * Assemble the complete Verilog function from its parts.
             *
             * @param[in] assignments - The translated assignments that make up the body.
             * @param[in] initalization - The translated initialization code.
             * @param[in] input_vars - The input variables of the function.
             * @returns The complete function in Verilog.
             */
            std::string construct_function(const std::string& assignments, const std::string& initalization, const std::vector<std::string>& input_vars) const override;

            /** A map from the name of a control signal to the value it is fixed to. */
            std::map<std::string, bool> m_control_mapping;
        };
    }    // namespace z3_utils
}    // namespace hal
