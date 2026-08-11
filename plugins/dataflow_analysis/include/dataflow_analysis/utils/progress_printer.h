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

namespace hal
{
    namespace dataflow
    {
        /**
         * Reports the progress of a long-running operation, either to stderr or to the progress indicator of the GUI.
         */
        class ProgressPrinter
        {
        public:
            ProgressPrinter(u32 max_message_size = 0);

            void print_progress_to_stderr(float progress, const std::string& message = "");

            // Safe to call from any thread, any lock state: the registered indicator posts a queued event to the GUI.
            void print_progress_to_gui(int percent = -1); // use m_last_percentage if negative

            void print_message_to_gui(const std::string& message);

            void clear();

            void reset();

        private:
            // returns -1 if terminal width cannot be determined
            int get_terminal_width();

            /** The number of progress bar segments that have been printed so far. */
            u32 m_printed_progress;

            /** The message that was printed last, kept so that it can be overwritten. */
            std::string m_last_message;

            /** The message that was last forwarded to the GUI. Not every message is relevant for the GUI. */
            std::string m_gui_message;

            /** The width of the progress bar in characters. */
            u32 m_bar_width;

            /** The maximum length of a message, longer messages are truncated. */
            u32 m_max_message_size;

            /** The percentage that was printed last, kept so that unchanged values are not reprinted. */
            int m_last_percentage;

            /** The width of the terminal in characters, negative if no terminal could be determined. */
            int m_terminal_width;
        };
    }    // namespace dataflow
}    // namespace hal
