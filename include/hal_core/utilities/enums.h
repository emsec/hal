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

#include <iostream>
#include <map>

namespace hal
{
    /**
     * Container storing the string description of an enum.
     * Must be declared for every enum to be supported.
     */
    template<typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
    struct EnumStrings
    {
        static std::map<T, std::string> data;
    };

    /**
     * Translates an enum value into its string representation.
     *
     * @param[in] e - The enum value.
     * @returns The string representation.
     */
    template<typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
    std::string enum_to_string(T e)
    {
        if (auto it = EnumStrings<T>::data.find(e); it == EnumStrings<T>::data.end())
        {
            throw std::runtime_error("no string for enum with value '" + std::to_string(static_cast<size_t>(e)) + "'.");
        }
        else
        {
            return it->second;
        }
    }

    /**
     * Translates a string into an enum value if possible.
     * Throws an exception if no matching enum value is found.
     *
     * @param[in] str - The string.
     * @returns The enum value.
     */
    template<typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
    T enum_from_string(const std::string& str)
    {
        for (const auto& [e, s] : EnumStrings<T>::data)
        {
            if (s == str)
            {
                return e;
            }
        }

        throw std::runtime_error("no enum value for string `" + str + "` available.");
    }

    /**
     * Translates a string into an enum value if possible.
     * Defaults to the given default value if no matching enum value is found.
     *
     * @param[in] str - The string.
     * @param[in] default_val - The default value.
     * @returns The enum value.
     */
    template<typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
    T enum_from_string(const std::string& str, const T default_val) noexcept
    {
        for (const auto& [e, s] : EnumStrings<T>::data)
        {
            if (s == str)
            {
                return e;
            }
        }

        return default_val;
    }

    /**
     * Checks whether a string has a valid translation into the enum type T.
     *
     * @param[in] str - The string.
     * @returns True if string has valid translation, false otherwise.
     */
    template<typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
    bool is_valid_enum(const std::string& str) noexcept
    {
        for (const auto& [e, s] : EnumStrings<T>::data)
        {
            if (s == str)
            {
                return true;
            }
        }

        return false;
    }

    /**
     * Inserts the string representation of an enum value into an output stream.
     *
     * @param[in] os - The output stream.
     * @param[in] e - The enum value.
     * @returns Reference to the provided stream.
     */
    template<typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
    std::ostream& operator<<(std::ostream& os, T e)
    {
        return os << enum_to_string(e);
    }
}    // namespace hal
