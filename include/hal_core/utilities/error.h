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
    class Error final
    {
    public:
        Error() = default;
        // Error(const Error&) = default;
        Error(Error&&) = default;
        Error& operator=(const Error&) = default;
        Error& operator=(Error&&) = default;

        /**
         * Construct an error from a file path, a line number and an error message.
         * 
         * @param[in] file - Path to the file in which the error occured.
         * @param[in] line - Line number at which the error occured.
         * @param[in] message - The error message.
         */
        explicit Error(const std::string& file, u32 line, const std::string& message) : m_message("[" + file + ":" + std::to_string(line) + "] " + message){};

        /**
         * Construct an error from an existing error by copying the original error message.
         * 
         * @param[in] error - The existing error.
         */
        Error(const Error& error) : m_message(error.get())
        {
        }

        /**
         * Construct an error from an existing error by copying the original error message and appending the current file and line number.
         * 
         * @param[in] file - Path to the file in which the error occured.
         * @param[in] line - Line number at which the error occured.
         * @param[in] error - The existing error.
         */
        Error(const std::string& file, u32 line, const Error& error) : m_message("[" + file + ":" + std::to_string(line) + "]\n" + error.get()){};

        /**
         * Construct an error from an existing error by copying the original error message and appending a new message as well as the current file and line number.
         * 
         * @param[in] file - Path to the file in which the error occured.
         * @param[in] line - Line number at which the error occured.
         * @param[in] error - The existing error.
         * @param[in] message - The error message to append.
         */
        Error(const std::string& file, u32 line, const Error& error, const std::string& message) : m_message("[" + file + ":" + std::to_string(line) + "] " + message + "\n" + error.get()){};

        /**
         * Get the error message.
         * 
         * @returns The error message.
         */
        const std::string& get() const
        {
            return m_message;
        }

    private:
        std::string m_message = "";
    };
}    // namespace hal