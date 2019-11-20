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

#if defined(__GNUC__)
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
// #include <experimental/filesystem>
#else
#include <filesystem>
#endif


#include <inttypes.h>
#include <stdexcept>
#include <stdlib.h>

typedef int8_t i8;

typedef int16_t i16;

typedef int32_t i32;

typedef int64_t i64;

typedef uint8_t u8;

typedef uint16_t u16;

typedef uint32_t u32;

typedef uint64_t u64;

namespace hal
{
#if defined(__GNUC__)
    namespace fs = boost::filesystem;
    //    namespace fs = std::experimental::filesystem;
    
    typedef boost::system::error_code error_code;
#else
    namespace fs = std::filesystem;
    typedef std::error_code error_code;
#endif
    typedef fs::path path;
}    // namespace hal

#define DEBUG

#define UNUSED(expr) (void)expr

#ifdef _WIN32
#include "arch_win.h"
#elif __APPLE__ && __MACH__
#include "arch_mac.h"

#elif __linux__
#include "arch_linux.h"
#endif

#if defined(_MSC_VER)
/* Microsoft */
#define EXPORT __declspec(dllexport)
#define IMPORT __declspec(dllimport)
#else
/* other */
#define EXPORTED __attribute__((visibility("default")))
#define NOT_EXPORTED __attribute__((visibility("hidden")))
#define PROTECTED __attribute__((visibility("protected")))
#define INTERNAL __attribute__((visibility("internal")))
#endif

#if defined(__clang__)
#define COMPILER_CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
#define COMPILER_GCC
#else
#define COMPILER_OTHER
#endif
