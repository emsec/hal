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

#include "def.h"

#include <string>

namespace hal
{
// operating systems independent type definitions for function and handle access
#ifdef _WIN32
#include <windows.h>
    using lib_fn_ptr_t = FARPROC;
    using handle_ptr_t = HINSTANCE;
#else
#include <dlfcn.h>
    using lib_fn_ptr_t = void (*)();
    using handle_ptr_t = void*;
#endif

    /**
     * @ingroup core
     */
    class CORE_API LibraryLoader
    {
    public:
        LibraryLoader();

        ~LibraryLoader();

        /**
         * Loads a library.
         *
         * @param[in] file_name - Name of the library.
         * @returns True on success.
         */
        bool load_library(const std::string& file_name);

        /**
         * Gets the file name of the loaded library.
         *
         * @returns The file name.
         */
        std::string get_file_name() const;

        /**
         * Unloads the library.
         *
         * @returns True on success.
         */
        bool unload_library();

        /**
         * Gets the function pointer to a function by name.
         *
         * @param[in] function_name - Name of the function.
         * @returns The function pointer.
         */
        lib_fn_ptr_t get_function(const std::string& function_name);

    private:
        // stores the file name of the library
        std::string m_file_name;

        // stores the handle_ptr_t to the library
        handle_ptr_t m_handle;
    };
}    // namespace hal
