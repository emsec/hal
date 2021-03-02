#pragma once

#include "hal_core/defines.h"

#include <iostream>
#include <vector>

namespace hal
{
    /**
     * Container storing the string description of an enum.
     * Must be declared for every enum to be supported.
     */
    template<typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
    struct EnumStrings
    {
        static std::vector<std::string> data;
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
        auto index = static_cast<size_t>(e);
        if (index >= EnumStrings<T>::data.size())
        {
            throw std::runtime_error("no string for enum with value '" + std::to_string(index) + "'.");
        }
        return EnumStrings<T>::data.at(index);
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
        for (size_t i = 0; i < EnumStrings<T>::data.size(); i++)
        {
            if (EnumStrings<T>::data.at(i) == str)
            {
                return static_cast<T>(i);
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
        for (size_t i = 0; i < EnumStrings<T>::data.size(); i++)
        {
            if (EnumStrings<T>::data.at(i) == str)
            {
                return static_cast<T>(i);
            }
        }

        return default_val;
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
