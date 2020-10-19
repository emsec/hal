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

namespace hal
{
    /**
     * @ingroup utilities
     */
    namespace core_strings
    {
        /**
         * Converts one kind of string into another kind of string.
         * 
         * @param[in] str - The string to convert.
         * @returns The converted string.
         */
        template<class S, class T>
        inline T convert_string(const S& str)
        {
            if constexpr (std::is_same<S, T>::value)
            {
                return str;
            }
            else
            {
                return T(str.data());
            }
        }

        /**
         * Char traits that ignore the case of a string.
         */
        struct CaseInsensitiveCharTraits : public std::char_traits<char>
        {
            /**
             * Checks for equality of two characters.
             * 
             * @param[in] c1 - The first character.
             * @param[in] c2 - The second character.
             * @returns True if equal, false otherwise.
             */
            static bool eq(char c1, char c2)
            {
                return toupper(c1) == toupper(c2);
            }

            /**
             * Checks for inequality of two characters.
             * 
             * @param[in] c1 - The first character.
             * @param[in] c2 - The second character.
             * @returns True if unequal, false otherwise.
             */
            static bool ne(char c1, char c2)
            {
                return toupper(c1) != toupper(c2);
            }

            /**
             * Checks if one character is numerically lower than the other one.
             * 
             * @param[in] c1 - The first character.
             * @param[in] c2 - The second character.
             * @returns True if lower, false otherwise.
             */
            static bool lt(char c1, char c2)
            {
                return toupper(c1) < toupper(c2);
            }

            /**
             * Compares to character arrays of size 'n'.
             * 
             * @param[in] s1 - First character array.
             * @param[in] s2 - Second character array.
             * @param[in] n - Size of the character arrays.
             * @returns '0' if 's1 == s2', '1' if 's1 > s2', or '-1' if 's1 < s2'.
             */
            static int compare(const char* s1, const char* s2, size_t n)
            {
                while (n-- != 0)
                {
                    if (toupper(*s1) < toupper(*s2))
                        return -1;
                    if (toupper(*s1) > toupper(*s2))
                        return 1;
                    ++s1;
                    ++s2;
                }
                return 0;
            }

            /**
             * Finds a character within a character array of size 'n'.
             * 
             * @param[in] s - The character array to search in.
             * @param[in] n - The size of the character array.
             * @param[in] a - The character to search for.
             * @returns A pointer to the position of 'a' within 's' or a 'nullptr'.
             */
            static const char* find(const char* s, int n, char a)
            {
                while (n-- > 0)
                {
                    if (toupper(*s) == toupper(a))
                    {
                        return s;
                    }
                    ++s;
                }
                return nullptr;
            }
        };

        using CaseInsensitiveString = std::basic_string<char, CaseInsensitiveCharTraits>;
    }    // namespace core_strings
}    // namespace hal

namespace std
{
    template<>
    struct hash<hal::core_strings::CaseInsensitiveString>
    {
        /**
         * Hashes the given string.
         * 
         * @param[in] s - The string to hash.
         * @returns The hash value.
         */
        std::size_t operator()(const hal::core_strings::CaseInsensitiveString& s) const
        {
            return std::hash<std::string>{}(std::string(s.data()));
        }
    };

}    // namespace std
