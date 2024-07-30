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
 * @file multithreading_types.h
 * @brief This file contains the enum class for multithreading strategies in the module identification process.
 */

#pragma once

#include "hal_core/utilities/enums.h"

namespace hal
{
    namespace module_identification
    {
        /**
         * @enum MultithreadingPriority
         * @brief Specifies the strategy for multithreading in the module identification process.
         * 
         * This enum class defines the strategies for managing multithreading in the module identification plugin. 
         * The strategies determine how resources are allocated and prioritized when performing multithreaded operations.
         */
        enum class MultithreadingPriority
        {
            /**
             * @brief Prioritize time efficiency in multithreading.
             * 
             * This option specifies that multithreading should be handled with a priority on time efficiency, 
             * aiming to complete tasks as quickly as possible.
             * This means that first all structural candidates are processed and afterwards all functional candidates are checked.
             * In doing so we prevent threads waiting for the generation of functional candidates to check when there are no more stuctural candidates in the quque.
             * However, generating all functional candidates first can lead to massive(!) RAM overhead.
             */
            time_priority,

            /**
             * @brief Prioritize memory efficiency in multithreading.
             * 
             * This option specifies that multithreading should be handled with a priority on memory efficiency,
             * aiming to minimize memory usage even if it results in longer execution times.
             * This means that as soon as any functional candidates are generated they are prioritized to get verified to get them out of memory.
             */
            memory_priority,
        };
    }    // namespace module_identification
}    // namespace hal
