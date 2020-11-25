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

/**
 * @file
 */

#pragma once

#include "hal_core/defines.h"

#include <functional>
#include <set>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

namespace hal
{
    /**
     * @ingroup utilities
     */
    namespace utils
    {
        /**
         * Get a single bit of an integer value.
         *
         * @param[in] value - The integer value.
         * @param[in] index - The bit position.
         * @returns The specified bit (0 or 1).
         */
        CORE_API inline u64 get_bit(const u64 value, const u64 index)
        {
            return (value >> index) & 1;
        }

        /**
         * Set a single bit of an integer to 1.
         *
         * @param[in] value - The integer value.
         * @param[in] index - The bit position.
         * @returns The manipulated value.
         */
        CORE_API inline u64 set_bit(const u64 value, const u64 index)
        {
            return value | ((u64)1 << index);
        }

        /**
         * Clear a single bit of an integer to 0.
         *
         * @param[in] value - The integer value.
         * @param[in] index - The bit position.
         * @returns The manipulated value.
         */
        CORE_API inline u64 clear_bit(const u64 value, const u64 index)
        {
            return value & ~((u64)1 << index);
        }

        /**
         * Toggle a single bit of an integer.
         *
         * @param[in] value - The integer value.
         * @param[in] index - The bit position.
         * @returns The manipulated value.
         */
        CORE_API inline u64 toggle_bit(const u64 value, const u64 index)
        {
            return value ^ ((u64)1 << index);
        }

        /**
         * Checks whether a string ends with another string.
         *
         * @param[in] s - The string to analyze.
         * @param[in] end - The ending to check for.
         * @returns True if \p s ends with \p end, false otherwise.
         */
        template<typename T>
        CORE_API bool ends_with(const T& s, const T& end)
        {
            if (s.length() >= end.length())
            {
                return (0 == s.compare(s.length() - end.length(), end.length(), end));
            }
            else
            {
                return false;
            }
        }

        /**
         * Checks whether a string begins with another string.
         *
         * @param[in] s - The string to analyze.
         * @param[in] start - The beginning to check for.
         * @returns True if \p s begins with \p start, false otherwise.
         */
        template<typename T>
        CORE_API bool starts_with(const T& s, const T& start)
        {
            if (s.length() >= start.length())
            {
                return (0 == s.compare(0, start.length(), start));
            }
            else
            {
                return false;
            }
        }

        /**
         * Checks whether a string represents an integer.
         *
         * @param[in] s - The string to analyze.
         * @returns True if \p s contains an integer, false otherwise.
         */
        template<typename T>
        CORE_API bool is_integer(const T& s)
        {
            if (s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+')))
            {
                return false;
            }

            char* p;
            strtol(s.c_str(), &p, 10);

            return (*p == 0);
        }

        /**
         * Checks whether a string represents a real number.
         *
         * @param[in] s - The string to analyze.
         * @returns True if \p s contains a real number, false otherwise.
         */
        template<typename T>
        CORE_API bool is_floating_point(const T& s)
        {
            std::stringstream ss(s.c_str());
            float f;
            ss >> f;
            return (ss.eof() && !ss.fail());
        }

        /**
         * Split a string into a vector of strings. The split delimiter can be specified by the user.<br>
         * The delimiters are removed in the splitting process.<br>
         * To avoid splitting within brackets, set \p obey_brackets to \p true.
         *
         * @param[in] s - The string to split.
         * @param[in] delim - The delimiter, indicating where to split.
         * @param[in] obey_brackets - Flag to indicate whether brackets are obeyed.
         * @returns The vector of strings.
         */
        template<typename T>
        CORE_API std::vector<T> split(const T& s, const char delim, bool obey_brackets = false)
        {
            std::vector<T> result;
            T item = "";

            if (obey_brackets)
            {
                int bracket_level = 0;

                for (size_t i = 0; i < s.length(); ++i)
                {
                    char c = s.at(i);
                    switch (c)
                    {
                        case '(':
                        case '{':
                        case '[':
                            ++bracket_level;
                            break;
                        case ')':
                        case '}':
                        case ']':
                            --bracket_level;
                            break;
                        default:
                            break;
                    }
                    if (bracket_level < 0)
                    {
                        bracket_level = 0;
                    }
                    if (c == delim)
                    {
                        // No constant expression, therefore not usable in switch case
                        if (bracket_level == 0)
                        {
                            result.push_back(item);
                            item = "";
                        }
                        else
                        {
                            item.push_back(c);
                        }
                    }
                    else
                    {
                        item.push_back(c);
                    }
                }
                if (!item.empty())
                {
                    result.push_back(item);
                }
            }
            else
            {
                std::stringstream ss(s);
                while (std::getline(ss, item, delim))
                {
                    result.push_back(item);
                }
            }
            if (s.back() == delim)
            {
                result.push_back("");
            }
            return result;
        }

        /**
         * Remove any leading whitespace characters from the beginning of a string.
         *
         * @param[in] s - The string to trim.
         * @param[in] to_remove - The characters to remove.
         * @returns The trimmed string.
         */
        template<typename T>
        CORE_API T ltrim(const T& s, const char* to_remove = " \t\r\n")
        {
            size_t start = s.find_first_not_of(to_remove);

            if (start != std::string::npos)
            {
                return s.substr(start, s.size() - start);
            }
            else
            {
                return "";
            }
        }

        /**
         * Remove any trailing whitespace characters from the end of a string.
         *
         * @param[in] s - The string to trim.
         * @param[in] to_remove - The characters to remove.
         * @returns The trimmed string.
         */
        template<typename T>
        CORE_API T rtrim(const T& s, const char* to_remove = " \t\r\n")
        {
            size_t end = s.find_last_not_of(to_remove);

            if (end != std::string::npos)
            {
                return s.substr(0, end + 1);
            }
            else
            {
                return "";
            }
        }

        /**
         * Remove any leading and trailing whitespace characters from the beginning and the end of a string.
         *
         * @param[in] s - The string to trim.
         * @param[in] to_remove - The characters to remove.
         * @returns The trimmed string.
         */
        template<typename T>
        CORE_API T trim(const T& s, const char* to_remove = " \t\r\n")
        {
            size_t start = s.find_first_not_of(to_remove);
            size_t end   = s.find_last_not_of(to_remove);

            if (start != T::npos)
            {
                return s.substr(start, end - start + 1);
            }
            else
            {
                return "";
            }
        }

        /**
         * Replace all occurences of a substring within a string with another string.
         *
         * @param[in] str - The initial string.
         * @param[in] search - The targetted substring.
         * @param[in] replace - The replacement string.
         * @returns String with replaced substrings.
         */
        template<typename T>
        CORE_API T replace(const T& str, const T& search, const T& replace)
        {
            auto s     = str;
            size_t pos = 0;

            if (search.empty())
            {
                return str;
            }

            while ((pos = s.find(search, pos)) != T::npos)
            {
                s.replace(pos, search.length(), replace);
                pos += replace.length();
            }

            return s;
        }

        /**
         * Join all elements of a iterator range with a joiner-string.
         * A transformation is applied to all elements of the collections before joining.
         *
         * @param[in] joiner - The string joining the elements.
         * @param[in] begin - The start iterator.
         * @param[in] end - The end iterator.
         * @param[in] transform - The transformation function.
         * @returns The joined string.
         */
        template<typename Iterator, class Transform>
        CORE_API std::string join(const std::string& joiner, const Iterator& begin, const Iterator& end, const Transform& transform)
        {
            std::stringstream ss;
            bool first = true;
            for (auto it = begin; it != end; ++it)
            {
                if (!first)
                {
                    ss << joiner;
                }
                first = false;
                ss << transform(*it);
            }
            return ss.str();
        }

        /**
         * Join all elements of a collection with a joiner-string.
         * A transformation is applied to all elements of the collections before joining.
         *
         * @param[in] joiner - The string joining the elements.
         * @param[in] items - The collection of elements to join.
         * @param[in] transform - The transformation function.
         * @returns The joined string.
         */
        template<typename T, class Transform>
        CORE_API std::string join(const std::string& joiner, const T& items, const Transform& transform)
        {
            return join(joiner, items.begin(), items.end(), transform);
        }

        /**
         * Join all elements of a collection with a joiner-string.
         *
         * @param[in] joiner - The string joining the elements.
         * @param[in] items - The collection of elements to join.
         * @returns The joined string.
         */
        template<typename T>
        CORE_API std::string join(const std::string& joiner, const T& items)
        {
            return join(joiner, items.begin(), items.end(), [](const auto& v) { return v; });
        }

        /**
         * Convert a string to upper case.
         *
         * @param[in] s - The string to convert.
         * @returns The upper case string.
         */
        template<typename T>
        CORE_API T to_upper(const T& s)
        {
            T result = s;
            std::transform(result.begin(), result.end(), result.begin(), [](char c) { return std::toupper(c); });
            return result;
        }

        /**
         * Convert a string to lower case.
         *
         * @param[in] s - The string to convert.
         * @returns The lower case string.
         */
        template<typename T>
        CORE_API T to_lower(const T& s)
        {
            T result = s;
            std::transform(result.begin(), result.end(), result.begin(), [](char c) { return std::tolower(c); });
            return result;
        }

        /**
         * Count occurences of a substring in a string.
         *
         * @param[in] s - The string containing the substring.
         * @param[in] substr - The substring.
         * @returns The number of occurrences.
         */
        template<typename T>
        CORE_API u32 num_of_occurrences(const T& s, const T& substr)
        {
            u32 num_of_occurrences = 0;
            auto position          = s.find(substr, 0);

            while (position != std::string::npos)
            {
                num_of_occurrences++;
                position = s.find(substr, position + 1);
            }

            return num_of_occurrences;
        }

        /**
         * Turn a given iterable collection into a vector.
         *
         * @param[in] container - The input collection.
         * @returns The vector containing all items copied from the collection.
         */
        template<typename T, template<typename, typename...> class Container, typename... Args>
        CORE_API inline std::vector<T> to_vector(const Container<T, Args...>& container)
        {
            return std::vector<T>(container.begin(), container.end());
        }

        /**
         * Check whether a file exists.
         *
         * @param[in] filename - The file to check.
         * @returns True if file exists, false otherwise.
         */
        CORE_API bool file_exists(const std::string& filename);

        /**
         * Check whether a directory exists and access rights are granted.
         *
         * @param[in] path - The directory to check.
         * @returns True if \p path exists and is accessible, false otherwise.
         */
        CORE_API bool folder_exists_and_is_accessible(const std::filesystem::path& path);

        /**
         * Locate an executable in the given path environment.
         *
         * @param[in] name - The name of the executable.
         * @param[in] path - The search path (defaults to PATH environment variable).
         * @returns The path if an executable is found, an empty path otherwise.
         */
        CORE_API std::filesystem::path which(const std::string& name, const std::string& path = "");

        /**
         * Get the path to the executable of HAL.
         *
         * @returns The path.
         */
        CORE_API std::filesystem::path get_binary_directory();

        /**
         * Get the base path to the HAL installation.<br>
         * 1. Use Environment Variable HAL_BASE_PATH<br>
         * 2. If current executable is hal (not e.g. python3 interpreter) use it's path to determine base path.<br>
         * 3. Try to find hal executable in path and use its base path.
         *
         * @returns The path.
         */
        CORE_API std::filesystem::path get_base_directory();

        /**
         * Get the path to the shared and static libraries of HAL.<br>
         * The resulting path is relative to the binary directory.
         *
         * @returns The path.
         */
        CORE_API std::filesystem::path get_library_directory();

        /**
         * Get the path to the shared objects of HAL.<br>
         * The resulting path is relative to the binary directory.
         *
         * @returns The path.
         */
        CORE_API std::filesystem::path get_share_directory();

        /**
         * Get the path to shared objects and files provided by the user.<br>
         *
         * @returns The path.
         */
        CORE_API std::filesystem::path get_user_share_directory();

        /**
         * Get the path to the read-only global configuration directory of HAL.<br>
         * The resulting path is relative to the binary directory.
         *
         * @returns The path.
         */
        CORE_API std::filesystem::path get_config_directory();

        /**
         * Get the path to the configuration directory of the user.<br>
         *
         * @returns The path.
         */
        CORE_API std::filesystem::path get_user_config_directory();

        /**
         * Get the path to the default directory for log files.<br>
         * If an netlist source file is provided, the function returns the parent directory.
         *
         * @param[in] source_file - The netlist source file.
         * @returns The path.
         */
        CORE_API std::filesystem::path get_default_log_directory(std::filesystem::path source_file = "");

        /**
         * Get the paths to the locations where gate libraries are searched.<br>
         *
         * @returns A vector of paths.
         */
        CORE_API std::vector<std::filesystem::path> get_gate_library_directories();

        /**
         * Get the paths to the locations where plugins are searched.<br>
         *
         * @returns A vector of paths.
         */
        CORE_API std::vector<std::filesystem::path> get_plugin_directories();

        /**
         * Get the first directory out of all the path hints that exists (searched in order).
         *
         * @param[in] path_hints - A vector of hints.
         * @returns The first existing path or an empty string if none of them exists.
         */
        CORE_API std::filesystem::path get_first_directory_exists(std::vector<std::filesystem::path> path_hints);

        /**
         * Search for the given filename in all paths provided by the path hints (searched in order).
         *
         * @param[in] file_name - The filename to search for.
         * @param[in] path_hints - A vector of hints.
         * @returns The first to match or an empty string if no match is found.
         */
        CORE_API std::filesystem::path get_file(std::string file_name, std::vector<std::filesystem::path> path_hints);

        /**
         * Get the licenses of all embedded OpenSource Projects.
         *
         * @returns The open source licenses.
         */
        CORE_API std::string get_open_source_licenses();

        /**
         * A recursive range of directories starting at a top level root directory.
         */
        class CORE_API RecursiveDirectoryRange
        {
        public:
            using iterator = std::filesystem::recursive_directory_iterator;

            /**
             * Construct a recursive directory range from a top level directory.
             *
             * @param[in] path - The path to the top level directory.
             */
            RecursiveDirectoryRange(std::filesystem::path path) : p_(path)
            {
            }

            /**
             * Get an iterator pointing to the first element in the top level directory.<br>
             * The iterator will iterate recursively through all elements inside the top level container.
             *
             * @returns The iterator.
             */
            iterator begin()
            {
                return std::filesystem::recursive_directory_iterator(p_);
            }

            /**
             * Get an iterator pointing behind the last element in the top level directory.
             *
             * @returns The iterator.
             */
            iterator end()
            {
                return std::filesystem::recursive_directory_iterator();
            }

        private:
            std::filesystem::path p_;
        };

        /**
         * A range of directories within the top level root directory.
         */
        class CORE_API DirectoryRange
        {
        public:
            using iterator = std::filesystem::directory_iterator;

            /**
             * Construct a directory range from a top level directory.
             *
             * @param[in] path - The top level directory.
             */
            DirectoryRange(std::filesystem::path path) : p_(path)
            {
            }

            /**
             * Get an iterator pointing to the first element in the top level directory.<br>
             * The iterator will iterate through all elements inside the top level container.<br>
             * Lower levels are ignored.
             *
             * @returns The iterator.
             */
            iterator begin()
            {
                return std::filesystem::directory_iterator(p_);
            }

            /**
             * Gets an iterator pointing behind the last element in the top level directory.
             *
             * @returns The iterator.
             */
            iterator end()
            {
                return std::filesystem::directory_iterator();
            }

        private:
            std::filesystem::path p_;
        };
    }    // namespace utils
}    // namespace hal
