//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include <string>
#include <vector>

namespace hal
{
    class python_console_history
    {
    public:
        /**
         * Constructs a new python_console_history object.
         *
         */
        python_console_history();

        ~python_console_history();

        /**
         * Add command to history and writes it back to the history file.
         *
         * @param command - Command to remember
         */
        void add_history(const std::string& command);

        /**
         * Returns the history item at index.
         *
         * @param index - Index of the history item to resolve.
         * @return command at index.
         */
        const std::string& get_history_item(const int& index) const;

        /**
         * Get the current size of the index;
         *
         * @return - History size.
         */
        int size() const;

        /**
         * Reloads the history from the history file.
         *
         */
        void update_from_file();

    private:
        void append_to_file(const std::string& command);

        std::string m_file_name;

        std::vector<std::string> m_history;
    };
}
