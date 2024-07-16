// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All rights reserved.
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

/**
 * @file structural_candidate.h
 * @brief This file contains the class for defining and managing structural candidates within the module identification plugin.
 */

#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/gate.h"
#include "module_identification/candidates/candidate_context.h"

#include <memory>
#include <vector>

namespace hal
{
    namespace module_identification
    {
        class BaseCandidate;

        /**
         * @class StructuralCandidate
         * @brief A class representing a structural candidate for module identification.
         * 
         * This class holds information about a structural candidate, including its base candidate, the gates involved, and the candidate context.
         * It serves as a structural variant built around the base candidate, which is used for further analysis and identification of modules within a netlist.
         */
        class StructuralCandidate
        {
        public:
            /**
             * @brief Constructor for StructuralCandidate.
             * 
             * Initializes a structural candidate with a given base candidate and a vector of gates. It also creates a candidate context.
             * 
             * @param[in] base_candidate_in - Pointer to the base candidate.
             * @param[in] gates_in - Vector of gates that form the structural candidate.
             */
            StructuralCandidate(BaseCandidate* base_candidate_in, const std::vector<Gate*>& gates_in)
                : base_candidate{base_candidate_in}, m_gates{gates_in}, ctx{CandidateContext{gates_in.back()->get_netlist(), gates_in}} {};

            /**
             * @brief Pointer to the base candidate.
             */
            BaseCandidate* base_candidate;

            /**
             * @brief Vector of gates that form the structural candidate.
             */
            std::vector<Gate*> m_gates;

            /**
             * @brief Candidate context for the structural candidate.
             */
            CandidateContext ctx;
        };
    }    // namespace module_identification
}    // namespace hal
