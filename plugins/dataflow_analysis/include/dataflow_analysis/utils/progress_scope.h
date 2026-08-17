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

#include <atomic>
#include <string>

namespace hal
{
    namespace dataflow
    {
        /**
         * Brackets a run of dataflow analysis in the progress indicator of the GUI.
         *
         * Shows the indicator on construction and dismisses it again on destruction, so that it is dismissed no matter
         * how the analysis was invoked and no matter how it returned. Scopes may be nested, in which case only the
         * outermost one takes effect: an analysis started from the GUI thereby keeps the indicator up while its caller
         * is still writing reports or creating modules.
         *
         * Without a GUI, i.e., when no progress indicator has been registered, the scope does nothing.
         */
        class ProgressScope
        {
        public:
            /**
             * Construct a progress scope and show the progress indicator of the GUI at zero percent.
             *
             * @param[in] message - The message to display alongside the progress.
             */
            explicit ProgressScope(const std::string& message);

            ~ProgressScope();

            ProgressScope(const ProgressScope&)            = delete;
            ProgressScope& operator=(const ProgressScope&) = delete;

        private:
            static std::atomic<u32> s_depth;
        };
    }    // namespace dataflow
}    // namespace hal
