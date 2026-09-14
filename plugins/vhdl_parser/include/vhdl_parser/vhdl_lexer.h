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

/**
 * @file vhdl_lexer.h
 * @brief The lexer of the VHDL netlist parser.
 */

#pragma once

#include "hal_core/defines.h"
#include "hal_core/utilities/result.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace hal
{
    namespace vhdl
    {
        /**
         * The kinds of token the lexer produces.
         */
        enum class TokenKind
        {
            Identifier,            /**< A basic identifier, case-insensitive, which may be a keyword; `text` as written. */
            ExtendedIdentifier,    /**< An extended identifier `\...\`, case-sensitive; `text` without the backslashes. */
            Character,             /**< A character literal `'0'`; `text` is the character. */
            String,                /**< A string literal; `text` without the quotes, `""` resolved to `"`. */
            BitString,             /**< A bit string literal such as `X"AB"`, `B"01_10"`, `8X"F"`; `text` as written. */
            Number,                /**< An integer or real, `12`, `1_000`, `1.5`, `1e3`, `16#FF#`; `text` as written. */
            Symbol,                /**< Punctuation or an operator, `(`, `=>`, `<=`, `:=`, `&`, `'`, ... */
            EndOfFile,
        };

        /**
         * A token with its position in the source.
         */
        struct Token
        {
            TokenKind kind = TokenKind::EndOfFile;
            std::string text;
            u32 line   = 0;
            u32 column = 0;

            /** The source file, shared by every token of that file. */
            std::shared_ptr<const std::string> file;

            /**
             * Get the name of the source file, or an empty string if unknown.
             */
            const std::string& file_name() const;

            /**
             * Check whether the token is the given keyword or basic identifier, compared without regard to case.
             *
             * @param[in] keyword - The keyword in lower case.
             */
            bool is(const char* keyword) const;

            /**
             * Check whether the token is the given symbol.
             */
            bool is_symbol(const char* symbol) const;

            /**
             * Check whether the token is a basic or an extended identifier.
             */
            bool is_identifier() const;

            /**
             * The key under which the identifier is looked up: the lower-case spelling of a basic identifier, the
             * exact spelling between backslashes of an extended one. Two identifiers name the same thing exactly when
             * their keys are equal.
             */
            std::string folded() const;

            /**
             * Format the position for a message, e.g. `line 12, column 4`.
             */
            std::string location() const;
        };

        /**
         * Fold a basic identifier the way `Token::folded` does, for names that do not come from a token.
         *
         * @param[in] name - The name as written.
         * @param[in] extended - Whether it is an extended identifier.
         * @returns The key.
         */
        std::string fold(const std::string& name, bool extended = false);

        /**
         * Split VHDL text into tokens. Line comments and block comments are dropped, `\r` counts as whitespace, a
         * byte order mark at the start is skipped. The last token is always `EndOfFile`.
         *
         * @param[in] text - The text.
         * @param[in] file - The path to report in messages.
         * @returns The tokens on success, an error otherwise (an unterminated string, extended identifier, or block comment).
         */
        Result<std::vector<Token>> lex_string(const std::string& text, const std::filesystem::path& file = "<string>");

        /**
         * Read and tokenize a file.
         *
         * @param[in] file - The path of the file.
         * @returns The tokens on success, an error otherwise.
         */
        Result<std::vector<Token>> lex_file(const std::filesystem::path& file);
    }    // namespace vhdl
}    // namespace hal
