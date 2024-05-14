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

#include "hal_core/utilities/result.h"

#include <set>

namespace hal
{
    class Netlist;
    class Gate;

    namespace hawkeye
    {
        class Candidate
        {
        public:
            Candidate()  = default;
            ~Candidate() = default;

            /**
             * Construct a crypto candidate from the state register of a round-based implementation.
             * 
             * @param[in] round_reg - The state register.
             */
            Candidate(const std::set<Gate*>& round_reg);

            /**
             * Construct a crypto candidate from the state register of a round-based implementation.
             * 
             * @param[in] round_reg - The state register.
             */
            Candidate(std::set<Gate*>&& round_reg);

            /**
             * Construct a crypto candidate from the input and output registers from a round of a pipelined implementation.
             * 
             * @param[in] in_reg - The input register.
             * @param[in] out_reg - The output register.
             */
            Candidate(const std::set<Gate*>& in_reg, const std::set<Gate*>& out_reg);

            /**
             * Construct a crypto candidate from the input and output registers from a round of a pipelined implementation.
             * 
             * @param[in] in_reg - The input register.
             * @param[in] out_reg - The output register.
             */
            Candidate(std::set<Gate*>&& in_reg, std::set<Gate*>&& out_reg);

            bool operator==(const Candidate& rhs) const;
            bool operator<(const Candidate& rhs) const;

            /** 
             * Get the netlist associated with the candidate.
             * 
             * @return The netlist of the candidate.
             */
            Netlist* get_netlist() const;

            /**
             * Get the size of the candidate, i.e., the width of its registers.
             * 
             * @returns The size of the candidate.
             */
            u32 get_size() const;

            /**
             * Check if the candidate is round-based, i.e., input and output register are the same.
             * 
             * @returns `true` if the candidate is round-based, `false` otherwise. 
             */
            bool is_round_based() const;

            /**
             * Get the candidate's input register.
             * 
             * @returns The input register of the candidate.
             */
            const std::set<Gate*>& get_input_reg() const;

            /**
             * Get the candidate's output register.
             * 
             * @returns The output register of the candidate.
             */
            const std::set<Gate*>& get_output_reg() const;

        private:
            /**
             * The netlist to which the candidate belongs.
             */
            Netlist* m_netlist;

            /**
             * The bit-size of the candidate.
             */
            u32 m_size;

            /**
             * Is `true` when the the candidate is round-based, i.e., input and output register are the same. 
             */
            bool m_is_round_based;

            /**
             * The candidate input register.
             */
            std::set<Gate*> m_in_reg;

            /**
             * The candidate output register. May be equal to `m_in_reg` for round-based implementations.
             */
            std::set<Gate*> m_out_reg;
        };
    }    // namespace hawkeye
}    // namespace hal