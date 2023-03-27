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

#include "converter/converter.h"

namespace hal
{
    namespace z3_utils
    {
        class VerilogConverter : public Converter
        {
        public:
            void set_control_mapping(const std::map<std::string, bool>& control_mapping);

        private:
            // VIRTUAL METHODS
            std::string build_operand(const std::string& operand) const override;
            std::string build_operation(const Operation& operation, const std::vector<std::string>& operands) const override;
            std::string generate_assignment(const std::string& l) const override;
            std::string generate_initialization(const std::vector<std::string>& input_vars) const override;
            std::string construct_function(const std::string& assignments, const std::string& initalization, const std::vector<std::string>& input_vars) const override;

            //std::vector<OPERATION> m_operations;

            std::map<std::string, bool> m_control_mapping;
        };
    }    // namespace z3_utils
}    // namespace hal
