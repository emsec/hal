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

#include "pragma_once.h"
#ifndef __HAL_TOKE_STREAM_H__
#define __HAL_TOKE_STREAM_H__

#include "def.h"

#include <map>

/**
 * @ingroup hdl_parsers
 */

struct HDL_PARSER_API token
{
    /**
     * Base token class that holds a string and a line number.
     * Can be set to be case insensitive in all string comparisons (default).
     *
     * @param[in] n - the line number
     * @param[in] s - the string
     * @param[in] cs - if true, string comparisons are case sensitive
     */
    token(u32 n, const std::string s, bool cs = true);

    // the line number
    u32 number;

    // the contained string
    std::string string;

    // if true, string comparisons are case sensitive
    bool case_sensitive;

    /*
     * A token is implicitly cast to string if possible.
     */
    operator std::string() const;

    /**
     * Assigns a new string to this token.
     *
     * @param[in] s - the new string
     * @returns A reference to this token.
     */
    token& operator=(const std::string& s);

    /**
     * Extends the string in this token.
     *
     * @param[in] s - the string to add
     * @returns A reference to this token.
     */
    token& operator+=(const std::string& s);

    /**
     * Checks if the string in this token is equal to another string.
     * If the case_sensitive member is true, the comparison is case sensitive.
     *
     * @param[in] s - the string to check against
     * @returns True if both strings are equal.
     */
    bool operator==(const std::string& s) const;

    /**
     * Checks if the string in this token is unequal to another string.
     * If the case_sensitive member is true, the comparison is case sensitive.
     *
     * @param[in] s - the string to check against
     * @returns True if both strings are not equal.
     */
    bool operator!=(const std::string& s) const;
};

class HDL_PARSER_API token_stream
{
public:
    struct token_stream_exception
    {
        std::string message;
        u32 line_number;
    };

    // constant that can be returned by find next.
    static const u32 END_OF_STREAM = 0xFFFFFFFF;

    /**
     * Constructor for an empty token stream.
     * The increase-level and decrease-level tokens are used for level-aware iteration.
     * If active, only tokens on level 0 are matched
     * Example: consuming until "b" in 'a,(,b,),b,c' would consume 'a,(,b,)', if "(" and ")" are level increase/decrease tokens.
     *
     * @param[in] decrease_level_tokens - the tokens that mark the start of a new level, i.e., increase the level.
     * @param[in] increase_level_tokens - the tokens that mark the end of a level, i.e., decrease the level.
     */
    token_stream(const std::vector<std::string>& increase_level_tokens = {"("}, const std::vector<std::string>& decrease_level_tokens = {")"});

    /**
     * Initialization constructor.
     * The increase-level and decrease-level tokens are used for level-aware iteration.
     * If active, only tokens on level 0 are matched
     * Example: consuming until "b" in 'a,(,b,),b,c' would consume 'a,(,b,)', if "(" and ")" are level increase/decrease tokens.
     *
     * @param[in] init - a token vector to initialize with
     * @param[in] decrease_level_tokens - the tokens that mark the start of a new level, i.e., increase the level.
     * @param[in] increase_level_tokens - the tokens that mark the end of a level, i.e., decrease the level.
     */
    token_stream(const std::vector<token>& init, const std::vector<std::string>& increase_level_tokens = {"("}, const std::vector<std::string>& decrease_level_tokens = {")"});

    /**
     * Copy constructor.
     *
     * @param[in] other - the token stream to copy
     */
    token_stream(const token_stream& other);

    /**
     * Consume the next token(s) in the stream.
     * Advances the stream by the given number and returns the last consumed token.
     *
     * Throws a std::out_of_range exception if more tokens are consumed than are available.
     *
     * @param[in] num - the amount of tokens to consume
     * @returns The last consumed token.
     */
    token consume(u32 num = 1);

    /**
     * Consume the next token in the stream *if* it matches the given expected value.
     * If the token does not match, the stream is unchanged.
     *
     * @param[in] expected - the expected token
     * @param[in] throw_on_error - if true, throws an std::invalid_argument instead of returning false
     * @returns True, if the next token matches the expected string, false otherwise or if no more tokens are available.
     */
    bool consume(const std::string& expected, bool throw_on_error = false);

    /**
     * Consume the next tokens in the stream until a token matches a given string.
     * This final token is not consumed, i.e., it is now the next token in the stream.
     * Consumes until the given end position if no token matches the given string.
     * Can be set to be level-aware with respect to the configured level-down and level-up tokens.
     *
     * @param[in] match - the string on which to end.
     * @param[in] end - the absolute position in the stream on which to stop, even if match was not found until this point.
     * @param[in] level_aware - if false, tokens are also matched if they are not at the top-level.
     * @returns The last consumed token.
     */
    token consume_until(const std::string& match, u32 end = END_OF_STREAM, bool level_aware = true, bool throw_on_error = false);

    /**
     * Consume the next tokens in the stream until a token matches the given string.
     * This final token is not consumed, i.e., it is now the next token in the stream.
     * All consumed tokens are returned as a new token stream.
     * Consumes until the given end position if no token matches the given string, i.e., the entire rest of the stream is returned as a new stream.
     * Can be set to be level-aware with respect to the configured level-down and level-up tokens.
     *
     * @param[in] match - the string on which to end.
     * @param[in] end - the absolute position in the stream on which to stop, even if match was not found until this point.
     * @param[in] level_aware - if false, tokens are also matched if they are not at the top-level.
     * @returns All consumed tokens in a new token stream.
     */
    token_stream extract_until(const std::string& match, u32 end = END_OF_STREAM, bool level_aware = true, bool throw_on_error = false);

    /**
     * Consume the next tokens in the stream until a token matches the given string.
     * This final token is not consumed, i.e., it is now the next token in the stream.
     * The strings of all consumed tokens are joined with the given joiner string and returned as a new token.
     * The returned token has the line number of the first consumed token.
     * Joins until the given end position if no token matches the given string until that point.
     * Can be set to be level-aware with respect to the configured level-down and level-up tokens.
     *
     * @param[in] match - the string on which to end.
     * @param[in] joiner - the string used to join consumed tokens.
     * @param[in] end - the absolute position in the stream on which to stop, even if match was not found until this point.
     * @param[in] level_aware - if false, tokens are also matched if they are not at the top-level.
     * @returns The joined token.
     */
    token join_until(const std::string& match, const std::string& joiner, u32 end = END_OF_STREAM, bool level_aware = true, bool throw_on_error = false);

    /**
     * Consume all remaining tokens in the stream.
     * The strings of all consumed tokens are joined with the given joiner string and returned as a new token.
     * The returned token has the line number of the first consumed token.
     *
     * @param[in] joiner - the string used to join consumed tokens.
     * @returns The joined token.
     */
    token join(const std::string& joiner);

    /**
     * Return a token at a *relative* position in the stream.
     * Can access tokens that have already been consumed.
     * Position 0 is the first element of the stream.
     *
     * Throws a std::out_of_range exception if the position is not contained in the stream.
     *
     * @param[in] offset - the relative offset from the current position in the stream
     * @returns The token at the queried position.
     */
    token& peek(i32 offset = 0);
    /**
     * @copydoc peek(i32)
     */
    const token& peek(i32 offset = 0) const;

    /**
     * Return a token at an *absolute* position in the stream.
     * Can access tokens that have already been consumed.
     * Position 0 is the first element of the stream.
     *
     * Throws a std::out_of_range exception if the position is not contained in the stream.
     *
     * @param[in] position - the absolute position in the stream
     * @returns The token at the queried position.
     */
    token& at(u32 position);
    /**
     * @copydoc at(i32)
     */
    const token& at(u32 position) const;

    /**
     * Get the *absolute* position in the stream of the next matching token.
     * If no token is found until the given end position, this end position is returned.
     * Can be set to be level-aware with respect to the configured level-down and level-up tokens.
     *
     * @param[in] match - the string to match
     * @param[in] end - the absolute position in the stream on which to stop, even if match was not found until this point.
     * @param[in] level_aware - if false, tokens are also matched if they are not at the top-level.
     * @returns The token at the queried position, or END_OF_STREAM if not found.
     */
    u32 find_next(const std::string& match, u32 end = END_OF_STREAM, bool level_aware = true) const;

    /**
     * Get the total number of elements in the token stream, regardless of how many have been consumed.
     *
     * @returns The total size of the token stream.
     */
    u32 size() const;

    /**
     * Get the total number of consumed elements in the token stream.
     *
     * @returns The total number of consumed tokens.
     */
    u32 consumed() const;

    /**
     * Get the total number of remaining elements in the token stream.
     *
     * @returns The number of remaining tokens.
     */
    u32 remaining() const;

    /**
     * Get the current absolute position in the token stream.
     *
     * @returns The current position.
     */
    u32 position() const;

    /**
     * Set the current absolute position in the token stream.
     *
     * @param[in] p - the new position
     */
    void set_position(u32 p);

private:
    u32 get_current_line_number() const;
    std::vector<std::string> m_increase_level_tokens;
    std::vector<std::string> m_decrease_level_tokens;
    std::vector<token> m_data;
    u32 m_pos;
};

#endif /* __HAL_HDL_PARSER_VHDL_H__ */
