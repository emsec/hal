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
 * @file lattice_ice40.h 
 * @brief This file contains the function to generate structural candidates for Lattice iCE40 FPGAs.
 */

#pragma once

#include "module_identification/candidates/base_candidate.h"
#include "module_identification/candidates/structural_candidate.h"

#include <map>
#include <memory>
#include <vector>

namespace hal
{
    class Gate;
    class Netlist;

    namespace module_identification
    {
        class BaseCandidate;
        class StructuralCandidate;

        namespace lattice_ice40
        {
            /**
             * @brief Generate structural candidates for a given netlist.
             * 
             * This function generates structural candidates by analyzing the netlist of a Lattice iCE40 FPGA.
             * The structural candidares are generated based on the identification of carry chains that are often used to implement arithmetic oeprations on FPGAs.
             * It returns a vector of pairs, where each pair contains a unique pointer to a `BaseCandidate` and a vector of unique pointers to `StructuralCandidate` objects.
             * 
             * @param[in] nl - The netlist to operate on.
             * @returns A vector of pairs, each containing a `BaseCandidate` and a vector of corresponding `StructuralCandidate` objects.
             */
            std::vector<std::pair<std::unique_ptr<BaseCandidate>, std::vector<std::unique_ptr<StructuralCandidate>>>> generate_structural_candidates(const Netlist* nl);
        }
    }    // namespace module_identification
}    // namespace hal
