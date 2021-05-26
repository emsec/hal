//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
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

#include "hal_core/defines.h"

#include <algorithm>
#include <map>
#include <vector>

namespace hal
{
    /**
     * A token holds a string and a line number and may, for example, be used during parsing of input files.
     * 
     * @ingroup utilities
     */
    template<typename T>
    struct CORE_API Token
    {
        /**
         * Construct a token from a line number and a string.
         *
         * @param[in] n - The line number.
         * @param[in] s - The string.
         */
        Token(u32 n, const T s) : number(n), string(s)
        {
        }

        /**
         * The line number.
         */
        u32 number;

        /**
         * The string.
         */
        T string;

        /**
         * The token is implicitly cast to a string when being used instead of a string.
         */
        operator T() const
        {
            return string;
        }

        /**
         * Assign a new string to the token.
         *
         * @param[in] s - The new string.
         * @returns A reference to the token.
         */
        Token<T>& operator=(const T& s)
        {
            this->string = s;
            return *this;
        }

        /**
         * Append a string to the one stored within the token.
         *
         * @param[in] s - The string to append.
         * @returns A reference to the token.
         */
        Token<T>& operator+=(const T& s)
        {
            this->string += s;
            return *this;
        }

        /**
         * Check if the string stored within the token is equal to another string.
         *
         * @param[in] s - The string to compare against.
         * @returns True if both strings are equal, false otherwise.
         */
        bool operator==(const T& s) const
        {
            return string == s;
        }

        /**
         * Check if the string stored within the token is unequal to another string.
         *
         * @param[in] s - The string to compare against.
         * @returns True if both strings are not equal, false otherwise.
         */
        bool operator!=(const T& s) const
        {
            return !(*this == s);
        }
    };

    /**
     * A token stream comprises a sequence of tokens that may, for example, have been read from a file.
     * 
     * @ingroup utilities
     */
    template<typename T>
    class NETLIST_API TokenStream
    {
    public:
        /**
         * The exception that is raised on any kind of error that occurs while working on the tokens of the stream.
         */
        struct TokenStreamException
        {
            /**
             * The message that is displayed to the user.
             */
            T message;

            /**
             * The affected line number.
             */
            u32 line_number;
        };

        /**
         * Constant value that marks the end of the token stream.
         */
        static const u32 END_OF_STREAM = 0xFFFFFFFF;

        /**
         * Construct an empty token stream.<br>
         * The increase-level and decrease-level tokens are used for level-aware iteration. If active, all operations are only executed on tokens on level 0.<br>
         * Example: consuming until "b" in 'a,(,b,),b,c' would consume 'a,(,b,)', if "(" and ")" are level increase/decrease tokens.
         *
         * @param[in] decrease_level_tokens - A vector of tokens that mark the start of a new level, i.e., increase the level.
         * @param[in] increase_level_tokens - A vector of tokens that mark the end of a level, i.e., decrease the level.
         */
        TokenStream(const std::vector<T>& increase_level_tokens = {"("}, const std::vector<T>& decrease_level_tokens = {")"})
        {
            m_pos                   = 0;
            m_increase_level_tokens = increase_level_tokens;
            m_decrease_level_tokens = decrease_level_tokens;
        }

        /**
         * Construct a token stream from a vector of tokens.<br>
         * The increase-level and decrease-level tokens are used for level-aware iteration. If active, all operations are only executed on tokens on level 0.<br>
         * Example: consuming until "b" in 'a,(,b,),b,c' would consume 'a,(,b,)', if "(" and ")" are level increase/decrease tokens.
         *
         * @param[in] init - The vector of tokens.
         * @param[in] decrease_level_tokens - A vector of tokens that mark the start of a new level, i.e., increase the level.
         * @param[in] increase_level_tokens - A vector of tokens that mark the end of a level, i.e., decrease the level.
         */
        TokenStream(const std::vector<Token<T>>& init, const std::vector<T>& increase_level_tokens = {"("}, const std::vector<T>& decrease_level_tokens = {")"})
            : TokenStream(increase_level_tokens, decrease_level_tokens)
        {
            m_data = init;
        }

        /**
         * Construct a token stream from another one (i.e., copy constructor).
         *
         * @param[in] other - The token stream to copy.
         */
        TokenStream(const TokenStream<T>& other)
        {
            m_pos                   = other.m_pos;
            m_data                  = other.m_data;
            m_increase_level_tokens = other.m_increase_level_tokens;
            m_decrease_level_tokens = other.m_decrease_level_tokens;
        }

        /**
         * Assign a token stream.
         *
         * @param[in] other - The token stream.
         * @returns A reference to the token stream.
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
         * Consume the next token(s) in the stream.<br>
         * Advances the stream by the given number and returns the last consumed token.
         *
         * Throws a std::out_of_range exception if more tokens are consumed than are available.
         *
         * @param[in] num - The amount of tokens to consume.
         * @returns The last consumed token.
         */
        Token<T> consume(u32 num = 1)
        {
            m_pos += num;
            return at(m_pos - 1);
        }

        /**
         * Consume the next token in the stream if it matches the given \p expected string.<br>
         * If the token does not match, the stream remains unchanged.
         *
         * @param[in] expected - The expected token.
         * @param[in] throw_on_error - If true, throws an TokenStreamException instead of returning false on error.
         * @returns True if the next Token matches the expected string, false otherwise or if no more tokens are available.
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
         * Consume the next tokens in the stream until a token matches the given \p expected string.<br>
         * This final token is not consumed, i.e., it is now the next token in the stream.<br>
         * Consumes tokens until reaching the given end position, if no token matches the given string.<br>
         * Can be set to be level-aware with respect to the configured increase and decrease level tokens.
         *
         * @param[in] expected - The string on which to end.
         * @param[in] end - The absolute position in the stream on which to stop if no match was found until this point.
         * @param[in] level_aware - If false, tokens are also matched if they are not at level 0.
         * @param[in] throw_on_error - If true, throws an TokenStreamException instead of returning false on error.
         * @returns The last consumed token or the last token of the stream.
         */
        Token<T> consume_until(const T& expected, u32 end = END_OF_STREAM, bool level_aware = true, bool throw_on_error = false)
        {
            auto found = find_next(expected, end, level_aware);
            if (found > size() && throw_on_error)
            {
                throw TokenStreamException({"expected Token '" + expected + "' not found", get_current_line_number()});
            }
            m_pos = std::min(size(), found);
            return at(m_pos - 1);
        }

        /**
         * Consume the entire current line and returns the last consumed token.
         *
         * @returns The last consumed token or the last token of the stream.
         */
        Token<T> consume_current_line()
        {
            u32 line = at(m_pos).number;

            u32 i = m_pos;
            while (++i < size() && at(i).number == line)
            {
            }
            m_pos = i;

            return at(m_pos - 1);
        }

        /**
         * Consume the next tokens in the stream until a token matches the given \p expected string.<br>
         * This final token is not consumed, i.e., it is now the next token in the stream.<br>
         * All consumed tokens are returned as a new token stream.<br>
         * Consumes tokens until reaching the given end position, if no token matches the given string.<br>
         * Can be set to be level-aware with respect to the configured increase and decrease level tokens.
         *
         * @param[in] expected - The string on which to end.
         * @param[in] end - The absolute position in the stream on which to stop if no match was not found until this point.
         * @param[in] level_aware - If false, tokens are also matched if they are not at level 0.
         * @param[in] throw_on_error - If true, throws an TokenStreamException instead of returning false on error.
         * @returns All consumed tokens in form of a new token stream.
         */
        TokenStream<T> extract_until(const T& expected, u32 end = END_OF_STREAM, bool level_aware = true, bool throw_on_error = false)
        {
            auto found = find_next(expected, end, level_aware);
            if (found > size() && throw_on_error)
            {
                throw TokenStreamException({"expected Token '" + expected + "' not found", get_current_line_number()});
            }
            auto end_pos = std::min(size(), found);
            TokenStream res(m_increase_level_tokens, m_decrease_level_tokens);
            res.m_data.reserve(res.m_data.size() + (end_pos - m_pos));
            res.m_data.insert(res.m_data.begin(), m_data.begin() + m_pos, m_data.begin() + end_pos);
            m_pos = end_pos;
            return res;
        }

        /**
         * Consume the next tokens in the stream until a token matches the given \p expected string.<br>
         * This final token is not consumed, i.e., it is now the next token in the stream.<br>
         * The strings of all consumed tokens are joined with the given joiner string and returned as a new Token.<br>
         * The returned token has the line number of the first consumed token.<br>
         * Joins tokens until reaching the given end position, if no token matches the given string.<br>
         * Can be set to be level-aware with respect to the configured increase and decrease level tokens.
         *
         * @param[in] match - The string on which to end.
         * @param[in] joiner - The string used to join consumed tokens.
         * @param[in] end - The absolute position in the stream on which to stop if no match was not found until this point.
         * @param[in] level_aware - If false, tokens are also matched if they are not at level 0.
         * @param[in] throw_on_error - If true, throws an TokenStreamException instead of returning false on error.
         * @returns The joined token.
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
         * Consume all remaining tokens in the stream.<br>
         * The strings of all consumed tokens are joined with the given joiner string and returned as a new token.<br>
         * The returned token has the line number of the first consumed Token.
         *
         * @param[in] joiner - The string used to join consumed tokens.
         * @returns The joined token.
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
         * Return a token at a relative position in the stream without consuming it.<br>
         * Can also access tokens that have already been consumed by using negative offsets.<br>
         * An offset of 0 referse to the next element of the stream.
         *
         * Throws a std::out_of_range exception if the position is not contained in the stream.
         *
         * @param[in] offset - The relative offset from the current position in the stream.
         * @returns The token at the queried position.
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
         * Return a Token at an absolute position in the stream without consuming it.<br>
         * Can also access tokens that have already been consumed.<br>
         * A position of 0 referse to the first element of the stream.
         *
         * Throws a std::out_of_range exception if the position is not contained in the stream.
         *
         * @param[in] position - The absolute position in the stream.
         * @returns The token at the queried position.
         */
        Token<T>& at(u32 position)
        {
            if (position >= m_data.size())
            {
                throw TokenStreamException({"reached the end of the stream", get_current_line_number()});
            }
            return m_data[position];
        }

        /**
         * @copydoc at(u32)
         */
        const Token<T>& at(u32 position) const
        {
            if (position >= m_data.size())
            {
                throw TokenStreamException({"reached the end of the stream", get_current_line_number()});
            }
            return m_data[position];
        }

        /**
         * Get the absolute position in the stream of the next matching token.<br>
         * If no token is found until the given end position, the end position is returned.<br>
         * Can be set to be level-aware with respect to the configured increase and decrease level tokens.
         *
         * @param[in] match - The string to match.
         * @param[in] end - The absolute position in the stream on which to stop if no match was not found until this point.
         * @param[in] level_aware - If false, tokens are also matched if they are not at level 0.
         * @returns The token at the queried position or END_OF_STREAM if not found.
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
         * Get the total number of elements in the token stream regardless of how many tokens have been consumed.
         *
         * @returns The size of the token stream.
         */
        u32 size() const
        {
            return m_data.size();
        }

        /**
         * Get the total number of consumed elements in the token stream.
         *
         * @returns The number of consumed tokens.
         */
        u32 consumed() const
        {
            return m_pos;
        }

        /**
         * Get the total number of remaining elements in the token stream.
         *
         * @returns The number of remaining tokens.
         */
        u32 remaining() const
        {
            return size() - consumed();
        }

        /**
         * Get the current absolute position in the token stream.
         *
         * @returns The current position.
         */
        u32 position() const
        {
            return m_pos;
        }

        /**
         * Set the current absolute position in the token stream.
         *
         * @param[in] p - The new position.
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
