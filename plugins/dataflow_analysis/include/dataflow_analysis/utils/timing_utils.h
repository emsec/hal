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

#include "hal_core/utilities/log.h"

#include <chrono>

#define MERGE_(a, b) a##b
#define LABEL_(a) MERGE_(measure_block_, a)
#define UNIQUE_NAME LABEL_(__LINE__)

#define measure_block_time(X) measure_block_time_t UNIQUE_NAME(X);

#define seconds_since(X) ((double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - (X)).count() / 1000)

namespace hal
{
    namespace dataflow
    {
        class measure_block_time_t
        {
        public:
            measure_block_time_t(const std::string& section_name)
            {
                m_name       = section_name;
                m_begin_time = std::chrono::high_resolution_clock::now();
            }

            ~measure_block_time_t()
            {
                log_info("dataflow", "{} took {:3.2f}s", m_name, seconds_since(m_begin_time));
            }

        private:
            std::string m_name;
            std::chrono::time_point<std::chrono::high_resolution_clock> m_begin_time;
        };
    }    // namespace dataflow
}