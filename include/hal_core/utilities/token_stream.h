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

#include "hal_core/def.h"

#include <algorithm>
#include <map>
#include <vector>

namespace hal
{
    /**
     * @ingroup core
     */
    template<typename T>
    struct CORE_API Token
    {
        /**
         * Base Token class that holds a string and a line number.
         * Can be set to be case insensitive in all string comparisons (default).
         *
         * @param[in] n - the line number
         * @param[in] s - the string
         */
        Token(u32 n, const T s) : number(n), string(s)
        {
        }

        // the line number
        u32 number;

        // the contained string
        T string;

        /**
         * A Token is implicitly cast to string if possible.
         */
        operator T() const
        {
            return string;
        }

        /**
         * Assigns a new string to this Token.
         *
         * @param[in] s - the new string
         * @returns A reference to this Token.
         */
        Token<T>& operator=(const T& s)
        {
            this->string = s;
            return *this;
        }

        /**
         * Extends the string in this Token.
         *
         * @param[in] s - the string to add
         * @returns A reference to this Token.
         */
        Token<T>& operator+=(const T& s)
        {
            this->string += s;
            return *this;
        }

        /**
         * Checks if the string in this Token is equal to another string.
         * If the case_sensitive member is true, the comparison is case sensitive.
         *
         * @param[in] s - the string to check against
         * @returns True if both strings are equal.
         */
        bool operator==(const T& s) const
        {
            return string == s;
        }

        /**
         * Checks if the string in this Token is unequal to another string.
         * If the case_sensitive member is true, the comparison is case sensitive.
         *
         * @param[in] s - the string to check against
         * @returns True if both strings are not equal.
         */
        bool operator!=(const T& s) const
        {
            return !(*this == s);
        }
    };

    /**
     * @ingroup core
     */
    template<typename T>
    class NETLIST_API TokenStream
    {
    public:
        struct TokenStreamException
        {
            T message;
            u32 line_number;
        };

        // constant that can be returned by find next.
        static const u32 END_OF_STREAM = 0xFFFFFFFF;

        /**
         * Constructor for an empty Token stream.
         * The increase-level and decrease-level tokens are used for level-aware iteration.
         * If active, only tokens on level 0 are matched
         * Example: consuming until "b" in 'a,(,b,),b,c' would consume 'a,(,b,)', if "(" and ")" are level increase/decrease tokens.
         *
         * @param[in] decrease_level_tokens - the tokens that mark the start of a new level, i.e., increase the level.
         * @param[in] increase_level_tokens - the tokens that mark the end of a level, i.e., decrease the level.
         */
        TokenStream(const std::vector<T>& increase_level_tokens = {"("}, const std::vector<T>& decrease_level_tokens = {")"})
        {
            m_pos                   = 0;
            m_increase_level_tokens = increase_level_tokens;
            m_decrease_level_tokens = decrease_level_tokens;
        }

        /**
         * Initialization constructor.
         * The increase-level and decrease-level tokens are used for level-aware iteration.
         * If active, only tokens on level 0 are matched
         * Example: consuming until "b" in 'a,(,b,),b,c' would consume 'a,(,b,)', if "(" and ")" are level increase/decrease tokens.
         *
         * @param[in] init - a Token vector to initialize with
         * @param[in] decrease_level_tokens - the tokens that mark the start of a new level, i.e., increase the level.
         * @param[in] increase_level_tokens - the tokens that mark the end of a level, i.e., decrease the level.
         */
        TokenStream(const std::vector<Token<T>>& init, const std::vector<T>& increase_level_tokens = {"("}, const std::vector<T>& decrease_level_tokens = {")"})
            : TokenStream(increase_level_tokens, decrease_level_tokens)
        {
            m_data = init;
        }

        /**
         * Copy constructor.
         *
         * @param[in] other - the Token stream to copy
         */
        TokenStream(const TokenStream<T>& other)
        {
            m_pos                   = other.m_pos;
            m_data                  = other.m_data;
            m_increase_level_tokens = other.m_increase_level_tokens;
            m_decrease_level_tokens = other.m_decrease_level_tokens;
        }

        /**
         * Assigns a tokenstream.
         *
         * @param[in] other - the tokenstream
         * @returns A reference to this tokenstream.
         */
        TokenStream<T>& operator=(const TokenStream<T>& other)
        {
            m_pos                   = other.m_pos;
            m_data                  = other.m_data;
            m_increase_level_tokens = other.m_increase_level_tokens;
            m_decrease_level_tokens = other.m_decrease_level_tokens;
            return *this;
        }

        /**
         * Consume the next Token(s) in the stream.
         * Advances the stream by the given number and returns the last consumed Token.
         *
         * Throws a std::out_of_range exception if more tokens are consumed than are available.
         *
         * @param[in] num - the amount of tokens to consume
         * @returns The last consumed Token.
         */
        Token<T> consume(u32 num = 1)
        {
            m_pos += num;
            return at(m_pos - 1);
        }

        /**
         * Consume the next Token in the stream *if* it matches the given expected value.
         * If the Token does not match, the stream is unchanged.
         *
         * @param[in] expected - the expected Token
         * @param[in] throw_on_error - if true, throws an TokenStreamException instead of returning false
         * @returns True, if the next Token matches the expected string, false otherwise or if no more tokens are available.
         */
        bool consume(const T& expected, bool throw_on_error = false)
        {
            if (m_pos >= size())
            {
                if (throw_on_error)
                {
                    throw TokenStreamException({"expected Token '" + expected + "' but reached the end of the stream", get_current_line_number()});
                }
                return false;
            }

            if (at(m_pos) != expected)
            {
                if (throw_on_error)
                {
                    throw TokenStreamException({"expected Token '" + expected + "' but got '" + at(m_pos).string + "'", get_current_line_number()});
                }
                return false;
            }

            m_pos++;
            return true;
        }

        /**
         * Consume the next tokens in the stream until a Token matches a given string.
         * This final Token is not consumed, i.e., it is now the next Token in the stream.
         * Consumes until the given end position if no Token matches the given string.
         * Can be set to be level-aware with respect to the configured level-down and level-up tokens.
         *
         * @param[in] match - the string on which to end.
         * @param[in] end - the absolute position in the stream on which to stop, even if match was not found until this point.
         * @param[in] level_aware - if false, tokens are also matched if they are not at the top-level.
         * @param[in] throw_on_error - if true, throws an TokenStreamException instead of returning false
         * @returns The last consumed Token.
         */
        Token<T> consume_until(const T& match, u32 end = END_OF_STREAM, bool level_aware = true, bool throw_on_error = false)
        {
            auto found = find_next(match, end, level_aware);
            if (found > size() && throw_on_error)
            {
                throw TokenStreamException({"match Token '" + match + "' not found", get_current_line_number()});
            }
            m_pos = std::min(size(), found);
            return at(m_pos - 1);
        }

        /**
         * Consume the next tokens in the stream until a Token matches the given string.
         * This final Token is not consumed, i.e., it is now the next Token in the stream.
         * All consumed tokens are returned as a new Token stream.
         * Consumes until the given end position if no Token matches the given string, i.e., the entire rest of the stream is returned as a new stream.
         * Can be set to be level-aware with respect to the configured level-down and level-up tokens.
         *
         * @param[in] match - the string on which to end.
         * @param[in] end - the absolute position in the stream on which to stop, even if match was not found until this point.
         * @param[in] level_aware - if false, tokens are also matched if they are not at the top-level.
         * @param[in] throw_on_error - if true, throws an TokenStreamException instead of returning false
         * @returns All consumed tokens in a new Token stream.
         */
        TokenStream<T> extract_until(const T& match, u32 end = END_OF_STREAM, bool level_aware = true, bool throw_on_error = false)
        {
            auto found = find_next(match, end, level_aware);
            if (found > size() && throw_on_error)
            {
                throw TokenStreamException({"match Token '" + match + "' not found", get_current_line_number()});
            }
            auto end_pos = std::min(size(), found);
            TokenStream res(m_increase_level_tokens, m_decrease_level_tokens);
            res.m_data.reserve(res.m_data.size() + (end_pos - m_pos));
            res.m_data.insert(res.m_data.begin(), m_data.begin() + m_pos, m_data.begin() + end_pos);
            m_pos = end_pos;
            return res;
        }

        /**
         * Consume the next tokens in the stream until a Token matches the given string.
         * This final Token is not consumed, i.e., it is now the next Token in the stream.
         * The strings of all consumed tokens are joined with the given joiner string and returned as a new Token.
         * The returned Token has the line number of the first consumed Token.
         * Joins until the given end position if no Token matches the given string until that point.
         * Can be set to be level-aware with respect to the configured level-down and level-up tokens.
         *
         * @param[in] match - the string on which to end.
         * @param[in] joiner - the string used to join consumed tokens.
         * @param[in] end - the absolute position in the stream on which to stop, even if match was not found until this point.
         * @param[in] level_aware - if false, tokens are also matched if they are not at the top-level.
         * @param[in] throw_on_error - if true, throws an TokenStreamException instead of returning false
         * @returns The joined Token.
         */
        Token<T> join_until(const T& match, const T& joiner, u32 end = END_OF_STREAM, bool level_aware = true, bool throw_on_error = false)
        {
            u32 start_line = get_current_line_number();
            auto found     = find_next(match, end, level_aware);
            if (found > size() && throw_on_error)
            {
                throw TokenStreamException({"match Token '" + match + "' not found", start_line});
            }
            auto end_pos = std::min(size(), found);
            T result;
            while (m_pos < end_pos && remaining() > 0)
            {
                if (!result.empty())
                {
                    result += joiner;
                }
                result += consume();
            }
            return {start_line, result};
        }

        /**
         * Consume all remaining tokens in the stream.
         * The strings of all consumed tokens are joined with the given joiner string and returned as a new Token.
         * The returned Token has the line number of the first consumed Token.
         *
         * @param[in] joiner - the string used to join consumed tokens.
         * @returns The joined Token.
         */
        Token<T> join(const T& joiner)
        {
            u32 start_line = get_current_line_number();
            T result;
            while (remaining() > 0)
            {
                if (!result.empty())
                {
                    result += joiner;
                }
                result += consume();
            }
            return {start_line, result};
        }

        /**
         * Return a Token at a *relative* position in the stream.
         * Can access tokens that have already been consumed.
         * Position 0 is the first element of the stream.
         *
         * Throws a std::out_of_range exception if the position is not contained in the stream.
         *
         * @param[in] offset - the relative offset from the current position in the stream
         * @returns The Token at the queried position.
         */
        Token<T>& peek(i32 offset = 0)
        {
            return at(m_pos + offset);
        }

        /**
         * @copydoc peek(i32)
         */
        const Token<T>& peek(i32 offset = 0) const
        {
            return at(m_pos + offset);
        }

        /**
         * Return a Token at an *absolute* position in the stream.
         * Can access tokens that have already been consumed.
         * Position 0 is the first element of the stream.
         *
         * Throws a std::out_of_range exception if the position is not contained in the stream.
         *
         * @param[in] position - the absolute position in the stream
         * @returns The Token at the queried position.
         */
        Token<T>& at(u32 position)
        {
            if (position > m_data.size())
            {
                throw TokenStreamException({"reached the end of the stream", get_current_line_number()});
            }
            return m_data[position];
        }

        /**
         * @copydoc at(i32)
         */
        const Token<T>& at(u32 position) const
        {
            if (position > m_data.size())
            {
                throw TokenStreamException({"reached the end of the stream", get_current_line_number()});
            }
            return m_data[position];
        }

        /**
         * Get the *absolute* position in the stream of the next matching Token.
         * If no Token is found until the given end position, this end position is returned.
         * Can be set to be level-aware with respect to the configured level-down and level-up tokens.
         *
         * @param[in] match - the string to match
         * @param[in] end - the absolute position in the stream on which to stop, even if match was not found until this point.
         * @param[in] level_aware - if false, tokens are also matched if they are not at the top-level.
         * @returns The Token at the queried position, or END_OF_STREAM if not found.
         */
        u32 find_next(const T& match, u32 end = END_OF_STREAM, bool level_aware = true) const
        {
            u32 level = 0;
            for (u32 i = m_pos; i < size() && i < end; ++i)
            {
                const auto& Token = at(i);
                if ((!level_aware || level == 0) && Token == match)
                {
                    return i;
                }
                else if (level_aware && std::find_if(m_increase_level_tokens.begin(), m_increase_level_tokens.end(), [&Token](const auto& x) { return Token == x; }) != m_increase_level_tokens.end())
                {
                    level++;
                }
                else if (level_aware && level > 0
                         && std::find_if(m_decrease_level_tokens.begin(), m_decrease_level_tokens.end(), [&Token](const auto& x) { return Token == x; }) != m_decrease_level_tokens.end())
                {
                    level--;
                }
            }
            return end;
        }

        /**
         * Get the total number of elements in the Token stream, regardless of how many have been consumed.
         *
         * @returns The total size of the Token stream.
         */
        u32 size() const
        {
            return m_data.size();
        }

        /**
         * Get the total number of consumed elements in the Token stream.
         *
         * @returns The total number of consumed tokens.
         */
        u32 consumed() const
        {
            return m_pos;
        }

        /**
         * Get the total number of remaining elements in the Token stream.
         *
         * @returns The number of remaining tokens.
         */
        u32 remaining() const
        {
            return size() - consumed();
        }

        /**
         * Get the current absolute position in the Token stream.
         *
         * @returns The current position.
         */
        u32 position() const
        {
            return m_pos;
        }

        /**
         * Set the current absolute position in the Token stream.
         *
         * @param[in] p - the new position
         */
        void set_position(u32 p)
        {
            m_pos = p;
        }

    private:
        std::vector<T> m_increase_level_tokens;
        std::vector<T> m_decrease_level_tokens;
        std::vector<Token<T>> m_data;
        u32 m_pos;

        u32 get_current_line_number() const
        {
            if (m_pos < m_data.size())
            {
                return m_data[m_pos].number;
            }
            else if (!m_data.empty())
            {
                return m_data.back().number;
            }
            return END_OF_STREAM;
        }
    };
}    // namespace hal
