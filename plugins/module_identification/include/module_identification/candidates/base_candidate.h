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
 * @file base_candidate.h
 * @brief This file contains the definition of the BaseCandidate class, which represents a base candidate in the module identification process.
 */

#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/gate.h"

#include <vector>

namespace hal
{
    namespace module_identification
    {
        /**
         * @class BaseCandidate
         * @brief Represents a base candidate in the module identification process.
         *
         * The BaseCandidate class holds a collection of gates that serve as the starting point for further structural and functional analysis
         * in the module identification process. The identification process aims to recognize groups of gates fulfilling specific functions within
         * a netlist, primarily focusing on arithmetic operations.
         */
        class BaseCandidate
        {
        public:
            /**
             * @brief Constructs a BaseCandidate with the given gates.
             * 
             * @param[in] gates A vector of Gate pointers representing the gates included in this base candidate.
             */
            BaseCandidate(const std::vector<Gate*>& gates) : m_gates{gates} {};

            /**
             * @brief A vector of Gate pointers representing the gates included in this base candidate.
             */
            std::vector<Gate*> m_gates;
        };
    }    // namespace module_identification
}    // namespace hal
