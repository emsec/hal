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
 * @file verilog_lexer.h
 * @brief The preprocessor and the lexer of the Verilog netlist parser.
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
    namespace verilog
    {
        /**
         * One line of preprocessed text together with where it came from.
         */
        struct Line
        {
            std::string text;
            u32 line = 0;     /**< The line in the source file, starting at 1. */
            std::string file; /**< The source file; differs from the top-level file inside an included file. */
        };

        /**
         * Run the compiler directives over a Verilog file.
         *
         * Handles `define (without arguments), `undef, `ifdef, `ifndef, `elsif, `else, `endif, `include (relative to
         * the file's folder), and macro uses; ignores `timescale, `celldefine, `endcelldefine, `default_nettype,
         * `resetall, `line, and the SystemVerilog `begin_keywords/`end_keywords. Any other directive is an error. Text
         * inside comments and strings is left alone. Lines removed by a directive stay as empty lines, so every line
         * of the result carries the number of the source line it came from.
         *
         * @param[in] text - The file contents.
         * @param[in] file - The path of the file, used for `include and for messages.
         * @returns The preprocessed lines on success, an error otherwise.
         */
        Result<std::vector<Line>> preprocess(const std::string& text, const std::filesystem::path& file);

        /**
         * The kinds of token the lexer produces.
         */
        enum class TokenKind
        {
            Identifier,        /**< A simple identifier, which may be a keyword. */
            EscapedIdentifier, /**< An escaped identifier `\...`, given without the backslash and never a keyword. */
            SystemIdentifier,  /**< A system task or function name, `$display`. */
            Number,            /**< A number in any Verilog form, `12`, `4'b10_1x`, `'h_ff`, `'0`, `1.5`. */
            String,            /**< A string literal, given without the quotes and with escapes resolved. */
            Symbol,            /**< Punctuation or an operator, `(`, `(*`, `#(`, `=`, `&`, ... */
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
             * Check whether the token is the given simple identifier or keyword.
             */
            bool is(const char* identifier) const;

            /**
             * Check whether the token is the given symbol.
             */
            bool is_symbol(const char* symbol) const;

            /**
             * Format the position for a message, e.g. `line 12, column 4`.
             */
            std::string location() const;
        };

        /**
         * Split preprocessed lines into tokens. Comments are dropped, escaped identifiers end at whitespace, a
         * `\r` counts as whitespace. The last token is always `EndOfFile`.
         *
         * @param[in] lines - The preprocessed lines.
         * @returns The tokens on success, an error otherwise (an unterminated string or block comment).
         */
        Result<std::vector<Token>> tokenize(const std::vector<Line>& lines);

        /**
         * Preprocess and tokenize a file.
         *
         * @param[in] file - The path of the file.
         * @returns The tokens on success, an error otherwise.
         */
        Result<std::vector<Token>> lex_file(const std::filesystem::path& file);

        /**
         * Preprocess and tokenize a string.
         *
         * @param[in] text - The text.
         * @param[in] file - The path to report in messages and to resolve `include against.
         * @returns The tokens on success, an error otherwise.
         */
        Result<std::vector<Token>> lex_string(const std::string& text, const std::filesystem::path& file = "<string>");
    }    // namespace verilog
}    // namespace hal
