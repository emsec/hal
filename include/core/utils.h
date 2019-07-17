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

#include "pragma_once.h"

#ifndef __HAL_UTILS_H__
#define __HAL_UTILS_H__

#include "def.h"

#include <set>
#include <string>
#include <vector>

/**
 * @ingroup core
 * @{
 */

/**
 * Returns a single bit of an integer value.
 *
 * @param[in] value - The integer.
 * @param[in] index - The bit's position.
 * @returns The bits value (0 or 1).
 */
#define get_bit(value, index) ((value >> index) & 1)

/**
 * Sets a single bit of an integer to 1.
 *
 * @param[in] value - The integer.
 * @param[in] index - The bit's position.
 */
#define set_bit(value, index)           \
    do                                  \
    {                                   \
        (value) |= ((u64)1 << (index)); \
    } while (0);

/**
 * Clears a single bit of an integer to 0.
 *
 * @param[in] value - The integer.
 * @param[in] index - The bit's position.
 */
#define clear_bit(value, index)          \
    do                                   \
    {                                    \
        (value) &= ~((u64)1 << (index)); \
    } while (0);

///@}

/**
 * @ingroup core
 */
namespace core_utils
{
    /**
     * Checks whether a string ends with another string.
     *
     * @param[in] s - The string to analyze.
     * @param[in] ending - The ending to check for.
     * @returns True, if \p s ends with \p ending.
     */
    CORE_API bool ends_with(const std::string& s, const std::string& ending);

    /**
     * Checks whether a string begins with another string.
     *
     * @param[in] s - The string to analyze.
     * @param[in] start - The beginning to check for.
     * @returns True, if \p s begins with \p start.
     */
    CORE_API bool starts_with(const std::string& s, const std::string& start);

    /**
     * Checks whether a string represents an integer.
     *
     * @param[in] s - The string to analyze.
     * @returns True, if \p s contains an integer.
     */
    CORE_API bool is_integer(const std::string& s);

    /**
     * Checks whether a string represents a real number.
     *
     * @param[in] s - The string to analyze.
     * @returns True, if \p s contains a real number.
     */
    CORE_API bool is_floating_point(const std::string& s);

    /**
     * Splits a string into a vector of strings. The split delimiter can be specified.
     * The delimiters are removed in the splitting process.
     *
     * @param[in] s - The string to split.
     * @param[in] delim - The delimiter, indicating where to split.
     * @param[in] obey_brackets - Flag to indicate whether brackets are obeyed.
     * @returns The string parts.
     */
    CORE_API std::vector<std::string> split(const std::string& s, const char delim, bool obey_brackets = false);

    /**
     * Removes any whitespace characters from the beginning of a string.
     *
     * @param[in] s - The string to trim.
     * @param[in] to_remove - All chars that should be removed.
     * @returns The trimmed string.
     */
    CORE_API std::string ltrim(const std::string& s, const char* to_remove = " \t\r\n");

    /**
     * Removes any whitespace characters from the end of a string.
     *
     * @param[in] s - The string to trim.
     * @param[in] to_remove - All chars that should be removed.
     * @returns The trimmed string.
     */
    CORE_API std::string rtrim(const std::string& s, const char* to_remove = " \t\r\n");

    /**
     * Removes any whitespace characters from the beginning and the end of a string.
     *
     * @param[in] s - The string to trim.
     * @param[in] to_remove - All chars that should be removed.
     * @returns The trimmed string.
     */
    CORE_API std::string trim(const std::string& s, const char* to_remove = " \t\r\n");

    /**
     * Replaces substring from begin to end of a string.
     *
     * @param[in] str - The string which is subject to replacement.
     * @param[in] search - The search substring which should be replaced.
     * @param[in] replace - The replacement string which replaces all occurrences of search.
     * @returns String with replaced substring.
     */
    CORE_API std::string replace(const std::string& str, const std::string& search, const std::string& replace);

    /**
     * Joins all elements of a vector with a joiner-string.
     *
     * @param[in] joiner - The string to put between the elements.
     * @param[in] items - The vector of elements to join.
     * @returns The combined string.
     */
    CORE_API std::string join(const std::string& joiner, const std::vector<std::string>& items);

    /**
     * Joins all elements of a set with a joiner-string.
     *
     * @param[in] joiner - The string to put between the elements.
     * @param[in] items - The set of elements to join.
     * @returns The combined string.
     */
    CORE_API std::string join(const std::string& joiner, const std::set<std::string>& items);

    /**
     * Convert a string to upper case.
     *
     * @param[in] s - The string to convert.
     * @returns The converted string.
     */
    CORE_API std::string to_upper(const std::string& s);

    /**
     * Convert a string to lower case.
     *
     * @param[in] s - The string to convert.
     * @returns The converted string.
     */
    CORE_API std::string to_lower(const std::string& s);

    /**
     * Counts number of substring occurrences in a string.
     *
     * @param[in] substr - Substring.
     * @param[in] str - String containing the substring.
     * @returns The number of occurrences.
     */
    CORE_API u32 num_of_occurrences(const std::string& str, const std::string& substr);

    /**
     * Checks whether a file exists.
     *
     * @param[in] filename - The file to check.
     * @returns True, if file exists
     */
    CORE_API bool file_exists(const std::string& filename);

    /**
     * Checks whether a directory exists and access rights are available.
     *
     * @param[in] folder - The directory to check.
     * @returns True, if \p folder exists and is accessible.
     */
    CORE_API bool folder_exists_and_is_accessible(const hal::path& folder);

    /**
	 * Locate an executable in the given path environment
	 *
	 * @param[in] name - The executable name
	 * @param[in] path - PATH to search (defaults to PATH environment variable)
	 * @returns path if executable is found else empty hal::path.
	 */
    CORE_API hal::path which(const std::string& name, const std::string& path = "");

    /**
     * Get the path to the executable of HAL.
     *
     * @returns The path.
     */
    CORE_API hal::path get_binary_directory();

    /**
     * Get the base path to the HAL installation.
     * 1. Use Environment Variable HAL_BASE_PATH
     * 2. If current executable is hal (not e.g. python3 interpreter) use it's path to determine base path.
     * 3. Try to find hal executable in path and use its base path.
     *
     * @returns The path.
     */
    CORE_API hal::path get_base_directory();

    /**
     * Get the path to the shared and static libraries of HAL.<br>
     * Relative to the binary directory.
     *
     * @returns The path.
     */
    CORE_API hal::path get_library_directory();

    /**
     * Get the path to the shared objects of HAL.<br>
     * Relative to the binary directory.
     *
     * @returns The path.
     */
    CORE_API hal::path get_share_directory();

    /**
     * Get the path to shared objects and files provided by the user.<br>
     * home/.local/share for Unix
     *
     * @returns The path.
     */
    CORE_API hal::path get_user_share_directory();

    /**
     * Get the path to the configuration directory of the user.<br>
     * home/.config/hal for Unix
     *
     * @returns The path.
     */
    CORE_API hal::path get_user_config_directory();

    /**
     * Get the path to the default directory for log files.<br>
     * If an hdl source file is provided, the function returns the parent directory, otherwise get_user_share_directory() / "log".
     *
     * @param[in] source_file - The hdl source file.
     * @returns The path.
     */
    CORE_API hal::path get_default_log_directory(hal::path source_file = "");

    /**
     * Get the paths where gate libraries are searched.<br>
     * Contains the share and user share directories.
     *
     * @returns A vector of paths.
     */
    CORE_API std::vector<hal::path> get_gate_library_directories();

    /**
     * Get the paths where plugins are searched.<br>
     * Contains the library and user share directories.
     *
     * @returns A vector of paths.
     */
    CORE_API std::vector<hal::path> get_plugin_directories();

    /**
     * Returns the first directory of all path hints that exists.
     *
     * @param[in] path_hints - A vector of hints.
     * @returns The first existing path. If none exists an empty string is returned.
     */
    CORE_API hal::path get_first_directory_exists(std::vector<hal::path> path_hints);

    /**
     * Search in all paths provided by path hints (used in order) for a given filename.
     *
     * @param[in] file_name - The filename to search for.
     * @param[in] path_hints - A vector of hints.
     * @returns The first to match. If none matches an empty string is returned.
     */
    CORE_API hal::path get_file(std::string file_name, std::vector<hal::path> path_hints);

    /**
     * Get all licenses of used OpenSource Projects.
     *
     * @returns All open source licenses;
     */
    CORE_API std::string get_open_source_licenses();

    class CORE_API recursive_directory_range
    {
    public:
        typedef hal::fs::recursive_directory_iterator iterator;

        /**
        * @param[in] p - The top level directory.
        */
        recursive_directory_range(hal::path p) : p_(p)
        {
        }

        /**
        * Gets an iterator pointing to the first element in the top level directory.<br>
        * The iterator will iterate recursively through all elements inside of the top level container.
        *
        * @returns The iterator.
        */
        iterator begin()
        {
            return hal::fs::recursive_directory_iterator(p_);
        }

        /**
        * Gets an iterator pointing behind the last element in the top level directory.
        *
        * @returns The iterator.
        */
        iterator end()
        {
            return hal::fs::recursive_directory_iterator();
        }

    private:
        hal::path p_;
    };

    class CORE_API directory_range
    {
    public:
        typedef hal::fs::directory_iterator iterator;

        /**
        * @param[in] p - The top level directory.
        */
        directory_range(hal::path p) : p_(p)
        {
        }

        /**
        * Gets an iterator pointing to the first element in the top level directory.<br>
        * The iterator will iterate through all elements inside of the top level container.<br>
        * Lower levels are ignored.
        *
        * @returns The iterator.
        */
        iterator begin()
        {
            return hal::fs::directory_iterator(p_);
        }

        /**
        * Gets an iterator pointing behind the last element in the top level directory.
        *
        * @returns The iterator.
        */
        iterator end()
        {
            return hal::fs::directory_iterator();
        }

    private:
        hal::path p_;
    };
}    // namespace core_utils

#endif    //__HAL_UTILS_H__