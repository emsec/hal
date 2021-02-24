#pragma once

#include "hal_core/defines.h"

#include <iostream>

namespace hal
{
    /**
     * Container storing the string description of an enum.
     * Must be declared for every enum to be supported.
     */
    template<typename T>
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
    template<typename T>
    std::string enum_to_string(T const& e)
    {
        return EnumStrings<T>::data.at(static_cast<int>(e));
    }

    /**
     * Translates a string into an enum value if possible.
     * Assumes the last enum entry to represent an invalid state, as it defaults to this value.
     * 
     * @param[in] str - The string.
     * @returns The enum value.
     */
    template<typename T>
    T enum_from_string(const std::string& str)
    {
        u32 i;
        for (i = 0; i < EnumStrings<T>::data.size()-1; i++)
        {
            if (EnumStrings<T>::data.at(i) == str)
            {
                break;
            }
        }

        return static_cast<T>(i);
    }

    /**
     * Inserts the string representation of an enum value into an output stream.
     * 
     * @param[in] os - The original output stream.
     * @param[in] e - The enum value.
     * @returns The output stream including the appended string representation.
     */
    template<typename T, typename std::enable_if<std::is_enum<T>::value>::type* = nullptr>
    std::ostream& operator<<(std::ostream& os, const T& e)
    {
        os << enum_to_string(e);

        return os;
    }
}    // namespace hal