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

#pragma once

#include "hal_core/netlist/boolean_function.h"

namespace hal
{
    namespace BooleanFunctionParser
    {
        /// ParserType refers to the parser identifier
        enum class ParserType
        {
            /// refers to the 'Liberty' Boolean function parser
            Liberty,
            /// refers to the 'Standard' Boolean function parser
            Standard,
            /// as 'Liberty' above, but removing space first
            LibertyNoSpace
        };

        /// TokenType refers to a token identifier for a Boolean function string.
        enum class TokenType
        {

            And,       /**< Boolean And operation (e.g., "&"). */
            Not,       /**< Boolean Not operation (e.g., "!&"). */
            NotSuffix, /** <Boolean Not operation as a expresion suffix (e.g., "'" for liberty grammar). */
            Or,        /**< Boolean Or operation (e.g., "|"). */
            Xor,       /**< Boolean Xor operation (e.g., "^"). */

            Variable, /**< Boolean variable (e.g., "i0"). */
            Constant, /**< Boolean constant (e.g., "0" or "1"). */

            BracketOpen,  /**< An opening bracket (e.g., "("). */
            BracketClose, /**< A closing bracket (e.g., "("). */
        };

        /// Token refers to a token identifier and accompanied data.
        struct Token
        {
            ////////////////////////////////////////////////////////////////////////////
            // Member
            ////////////////////////////////////////////////////////////////////////////

            /// refers to the underlying token type identifier
            TokenType type;
            /// optional value and bit-size in case token is a variable
            std::tuple<std::string, u16> variable{};
            /// optional value in case token is a constant
            std::vector<BooleanFunction::Value> constant{};

            ////////////////////////////////////////////////////////////////////////////
            // Constructors, Destructors, Operators
            ////////////////////////////////////////////////////////////////////////////

            /**
             * @brief Construct a new initialized Token.
             * 
             * 
             * @param[in] _type - The type of the token.
             * @param[in] variable - A variable name and bit-size in case the token type is `Variable`.
             * @param[in] constant - A constant value in case the token type is `Constant`.
             */
            Token(TokenType _type, std::tuple<std::string, u16> variable, std::vector<BooleanFunction::Value> constant);

            /**
             * Creates an `AND` token.
             * 
             * @returns The `AND` token.
             */
            static Token And();

            /**
             * Creates an `NOT` token.
             * 
             * @returns The `NOT` token.
             */
            static Token Not();

            /**
             * Creates an `NOT_SUFFIX` token.
             * 
             * @returns The `NOT_SUFFIX` token.
             */
            static Token NotSuffix();

            /**
             * Creates an `OR` token.
             * 
             * @returns The `OR` token.
             */
            static Token Or();

            /**
             * Creates an `XOR` token.
             * 
             * @returns The `XOR` token.
             */
            static Token Xor();

            /**
             * Creates an `Variable` token.
             * 
             * @param[in] name - The name of the variable.
             * @param[in] size - The bit-size of the variable.  
             * @returns The `Variable` token.
             */
            static Token Variable(std::string name, u16 size);

            /**
             * Creates an `Constant` token.
             * 
             * @param[in] value - The (multi-bit) value of the constant.
             * @returns The `Constant` token.
             */
            static Token Constant(std::vector<BooleanFunction::Value> value);

            /**
             * Creates an `BracketOpen` token.
             * 
             * @returns The `BracketOpen` token.
             */
            static Token BracketOpen();

            /**
             * Creates an `BracketClose` token.
             * 
             * @returns The `BracketClose` token.
             */
            static Token BracketClose();

            ////////////////////////////////////////////////////////////////////////////
            // Interface
            ////////////////////////////////////////////////////////////////////////////

            /** 
             * Returns the precedence of a token.
             *
             * @param[in] type The parser type identifier.
             * @returns An unsigned predecene value between 2 and 4.
             */
            unsigned precedence(const ParserType& type) const;

            /**
             * Short-hand implementation to check for a token type.
             * 
             * @param[in] type Token type.
             * @returns `true` in case instance has the same token type, `false` otherwise.
             */
            bool is(TokenType type) const;

            /**
             * Outputs a token to the given output stream.
             * 
             * @param[in] os - The output stream.
             * @param[in] token - The token to be output.
             * @returns The output stream.
             */
            friend std::ostream& operator<<(std::ostream& os, const Token& token);

            /**
             * Convert the token to its string representation.
             * 
             * @returns The string representation of the token.
             */
            std::string to_string() const;
        };

        /**
         * Parses a Boolean function from a string representation into its tokens.
         * 
         * @param[in] expression - Boolean function string.
         * @returns Ok() and the list of tokens on success, Err() otherwise.
         */
        Result<std::vector<Token>> parse_with_standard_grammar(const std::string& expression);

        /**
         * Parses a Boolean function from a string representation into its tokens
         * based on the data format defined for Liberty, see Liberty user guide.
         * 
         * @param[in] expression - Boolean function string.
         * @returns Ok() and the list of tokens on success, Err() otherwise.
         */
        Result<std::vector<Token>> parse_with_liberty_grammar(const std::string& expression);

        /**
         * Transforms a list of tokens in infix notation (e.g., "A & B") into the
         * reverse polish notation notation (e.g., "A B &") that allows for an easy
         * translation into the AST data structure. 
         * 
         * In order to translate between the different notations, we leverage the
         * well-known Shunting Yard algorithm, see wikipedia for an example:
         *  https://en.wikipedia.org/wiki/Shunting-yard_algorithm
         * 
         * @param[in] tokens - List of tokens in infix notation.
         * @param[in] expression - Expression string.
         * @param[in] parser - Parser identifier
         * @returns Ok() and list of tokens in reverse-polish notation on success, Err() otherwise.
         */
        Result<std::vector<Token>> reverse_polish_notation(std::vector<Token>&& tokens, const std::string& expression, const ParserType& parser);

        /**
         * Translates a list of tokens (in reverse-polish notation form) into a list
         * of BooleanFunction nodes that are then assembled into a Boolean function.
         * 
         * @param[in] tokens - List of tokens.
         * @param[in] expression - Expression string.
         * @returns Ok() and the Boolean function on success, Err() otherwise.
         */
        Result<BooleanFunction> translate(std::vector<Token>&& tokens, const std::string& expression);

    }    // namespace BooleanFunctionParser
}    // namespace hal
