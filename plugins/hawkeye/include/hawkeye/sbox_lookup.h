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

#include "hal_core/defines.h"
#include "hal_core/utilities/result.h"
#include "hawkeye/sbox_database.h"

#include <set>
#include <vector>

namespace hal
{
    class Gate;

    namespace hawkeye
    {

        class RoundCandidate;

        /**
         * Stores all information related to an S-box candidate such as the `RoundCandidate` it belongs to, the connected component it is part of, and its input and output gates.
         */
        class SBoxCandidate
        {
        public:
            /**
             * The `RoundCandidate` that the S-box candidate belongs to.
             */
            const RoundCandidate* m_candidate;

            /**
             * The gates of the component which the S-box candidate is part of.
             */
            std::vector<Gate*> m_component;

            /**
             * The input gates of the S-box candidate (will be flip-flops).
             */
            std::set<Gate*> m_input_gates;

            /**
             * The output gates of the S-box candidate (usually combinational logic that is input to the linear layer).
             */
            std::set<Gate*> m_output_gates;
        };

        /**
         * Tries to locate S-box candidates within the combinational next-state logic of the round function candidate. 
         * 
         * @param[in] candidate - A round function candidate.
         * @returns A vector of S-box candidates on success, an error otherwise.
         */
        Result<std::vector<SBoxCandidate>> locate_sboxes(const RoundCandidate* candidate);

        /**
         * Tries to identify an S-box candidate by matching it against a database of known S-boxes under affine equivalence.
         * 
         * @param[in] sbox_candidate - An S-box candidate.
         * @param[in] db - A database of known S-boxes.
         * @returns The name of the S-box on success, an error otherwise.
         */
        Result<std::string> identify_sbox(const SBoxCandidate& sbox_candidate, const SBoxDatabase& db);
    }    // namespace hawkeye
}    // namespace hal