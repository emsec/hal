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
 * @file vhdl_syntax.h
 * @brief The recursive-descent parser that turns VHDL tokens into the syntax tree.
 */

#pragma once

#include "hal_core/utilities/result.h"
#include "vhdl_parser/vhdl_ast.h"
#include "vhdl_parser/vhdl_lexer.h"

#include <filesystem>
#include <vector>

namespace hal
{
    namespace vhdl
    {
        /**
         * Parse tokens into the syntax tree of the structural VHDL subset: library and use clauses, entities with
         * generics and ports, architectures with signal, constant, component, type and attribute declarations,
         * component instantiations in every form with generic and port maps, concurrent signal assignments, packages
         * with component declarations, and configurations with `for all` bindings. Functions and procedures are
         * skipped; processes, blocks, generate statements and variables are rejected with an error that names them.
         *
         * @param[in] tokens - The tokens, ending with `EndOfFile`.
         * @param[in] file - The file name to record.
         * @returns The syntax tree on success, an error with line and column otherwise.
         */
        Result<ast::SourceFile> parse_tokens(const std::vector<Token>& tokens, const std::string& file);

        /**
         * Tokenize and parse a string.
         *
         * @param[in] text - The text.
         * @param[in] file - The path to report in messages.
         * @returns The syntax tree on success, an error otherwise.
         */
        Result<ast::SourceFile> parse_string(const std::string& text, const std::filesystem::path& file = "<string>");

        /**
         * Tokenize and parse a file.
         *
         * @param[in] file - The path of the file.
         * @returns The syntax tree on success, an error otherwise.
         */
        Result<ast::SourceFile> parse_file(const std::filesystem::path& file);
    }    // namespace vhdl
}    // namespace hal
