#pragma once

#include <string>

namespace core_strings
{
    template<class T>
    inline std::string to_std_string(const T& str)
    {
        if constexpr (std::is_same<T, std::string>::value)
        {
            return str;
        }
        else
        {
            return std::string(str.data());
        }
    }

    template<class T>
    inline T from_std_string(const std::string& str)
    {
        if constexpr (std::is_same<T, std::string>::value)
        {
            return str;
        }
        else
        {
            return T(str.data());
        }
    }

    struct case_insensitive_char_traits : public std::char_traits<char>
    {
        static bool eq(char c1, char c2)
        {
            return toupper(c1) == toupper(c2);
        }

        static bool ne(char c1, char c2)
        {
            return toupper(c1) != toupper(c2);
        }

        static bool lt(char c1, char c2)
        {
            return toupper(c1) < toupper(c2);
        }

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
    using case_insensitive_string = std::basic_string<char, case_insensitive_char_traits>;
}    // namespace core_strings

namespace std
{
    template<>
    struct hash<core_strings::case_insensitive_string>
    {
        std::size_t operator()(const core_strings::case_insensitive_string& str) const
        {
            return std::hash<std::string>{}(std::string(str.data()));
        }
    };

}    // namespace std