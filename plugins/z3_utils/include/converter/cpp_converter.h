//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include "z3_utils/include/converter/converter.h"
#include "hal_core/defines.h"

namespace hal
{
    namespace z3_utils
    {
        class Cpp_Converter : public Converter
        {
        private:
            // VIRTUAL METHODS
            std::string build_operand(const std::string& operand) const override;
            std::string build_operation(const Operation& operation, const std::vector<std::string>& operands) const override;
            std::string generate_assignment(const std::string& l) const override;
            std::string generate_initialization(const std::vector<u32>& inputs) const override;
            std::string construct_function(const std::string& assignments, const std::string& initalization, const std::vector<u32>& inputs) const override;

            std::string m_function_corpus = R"(
    #include <stdbool.h> 
    #include <stdio.h>
    #include <stdlib.h>

    static unsigned long x=123456789, y=362436069, z=521288629;

    unsigned long xorshf96(void) {          //period 2^96-1
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

    int input_ids[] = {
    <INIT>
    };

    int inputs_len = sizeof(input_ids)/sizeof(int);

    bool func(bool* values) {
    <ASSIGNMENTS>
    return <RETURN>;
    }

    void build_values(bool* values) {
        for (int i = 0; i < inputs_len; i++) {
            int input = input_ids[i];
            bool random_value = xorshf96() % 2;
            values[input] = random_value;
        }

        return;
    }

    int main(int argc, char *argv[]) {
        int b = atoi(argv[1]);
        int num = atoi(argv[2]);
        int count = 0;
        const int input_size = <INPUT_SIZE>;

        bool values[input_size];
        for (int i = 0; i < num; i++) {
            build_values(values);

            values[b] = true;
            bool r1 = func(values);

            values[b] = false;
            bool r2 = func(values);

            if (r1 != r2) {
                count++;
            }
        }

        printf("%d\n", count);

        return count;
    })";
        };
    }    //namespace z3_utils
}    // namespace hal