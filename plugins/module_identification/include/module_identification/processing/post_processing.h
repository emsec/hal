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
 * @file post_processing.h 
 * @brief This file contains the function to perform post-processing on verified candidates to identify the best candidate for module identification.
 */

#pragma once

#include <vector>

namespace hal
{
    class Gate;
    class Netlist;

    namespace module_identification
    {
        class VerifiedCandidate;

        /**
         * @brief Performs post-processing on a set of verified candidates to identify the best candidate for module identification.
         *
         * This function processes the provided verified candidates and selects the best candidate based on several criteria, such as the number of input and output signals,
         * the number of control signals, and the number of gates in the candidate. The best candidate is then merged with other candidates sharing the same control mapping
         * to create a final verified candidate.
         *
         * @param[in] verified_candidates - A vector of verified candidates to process.
         * @param[in] nl - The netlist containing the candidates.
         * @param[in] dana_cache - A cache of gate vectors used for various calculations during the selection and merging process.
         * @returns OK() and the best verified candidate on success, an error otherwise.
         */
        VerifiedCandidate post_processing(const std::vector<VerifiedCandidate>& verified_candidates, const Netlist* nl, const std::vector<std::vector<Gate*>>& dana_cache);
    }    // namespace module_identification
}    // namespace hal
