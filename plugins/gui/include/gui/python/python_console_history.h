//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
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
    /**
     * @ingroup python-console
     * @brief Stores the history of python commands.
     *
     * This class manages the history of python console inputs. It can be used to access previous inputs.
     */
    class PythonConsoleHistory
    {
    public:
        /**
         * Constructs a new PythonConsoleHistory object.
         *
         */
        PythonConsoleHistory();

        /**
          * The destructor.
          */
        ~PythonConsoleHistory();

        /**
         * Add command to history and writes it back to the history file.
         *
         * @param command - Command to remember
         */
        void addHistory(const std::string& command);

        /**
         * Returns the history item at index.
         *
         * @param index - Index of the history item to resolve.
         * @return command at index.
         */
        const std::string& getHistoryItem(const int& index) const;

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
        void updateFromFile();

    private:
        void appendToFile(const std::string& command);

        std::string mFileName;

        std::vector<std::string> mHistory;
    };
}
