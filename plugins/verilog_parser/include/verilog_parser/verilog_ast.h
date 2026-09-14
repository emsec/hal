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
 * @file verilog_ast.h
 * @brief The syntax tree of a structural Verilog file, as written, before any name is resolved or any range expanded.
 */

#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/gate_library/enums/pin_direction.h"
#include "hal_core/netlist/netlist_ir/netlist_ir.h"

#include <optional>
#include <string>
#include <vector>

namespace hal
{
    namespace verilog
    {
        namespace ast
        {
            using Location = netlist_ir::Location;

            /**
             * An expression: a wiring expression in a connection or an assignment, or a constant expression in a range,
             * a parameter, or an attribute. The tree keeps what the file says; elaboration decides what it may mean.
             */
            struct Expr
            {
                enum class Kind
                {
                    Empty,      /**< Nothing, as in `.a()` or an empty positional slot. */
                    Identifier, /**< A name; `text` holds it without escape syntax. */
                    Number,     /**< A number; `text` holds it as written, e.g. `4'b1010`. */
                    String,     /**< A string; `text` holds it without quotes. */
                    Index,      /**< `base[index]`: children are the base and the index. */
                    Slice,      /**< `base[left:right]`: children are the base, the left and the right index. */
                    Concat,     /**< `{a, b, ...}`: children are the items, left to right. */
                    Replicate,  /**< `{count{a, b, ...}}`: the first child is the count, the rest are the items. */
                    Unary,      /**< An operator with one operand; `text` holds the operator. */
                    Binary,     /**< An operator with two operands; `text` holds the operator. */
                    Conditional /**< `cond ? a : b`. */
                };

                Kind kind = Kind::Empty;
                std::string text;
                std::vector<Expr> children;
                Location location;

                bool is_empty() const
                {
                    return kind == Kind::Empty;
                }
            };

            /**
             * A range `[left:right]` as written; the bounds are constant expressions.
             */
            struct Range
            {
                Expr left;
                Expr right;
            };

            /**
             * An attribute from `(* name = value, flag *)`.
             */
            struct Attribute
            {
                std::string name;
                std::optional<Expr> value;
                Location location;
            };

            /**
             * A `parameter` or `localparam` declaration of a module, with its default.
             */
            struct ParameterDecl
            {
                std::string name;
                Expr value;
                bool is_local = false;
                Location location;
            };

            /**
             * A port of a module: from the ANSI header or from `input`/`output`/`inout` declarations in the body.
             */
            struct PortDecl
            {
                std::string name;
                PinDirection direction = PinDirection::none;
                std::vector<Range> packed_dims;   /**< The ranges before the name, outermost first. */
                std::vector<Range> unpacked_dims; /**< The ranges after the name. */
                std::string net_type;             /**< `wire`, `reg`, ... or empty. */
                std::vector<Attribute> attributes;
                Location location;
            };

            /**
             * A `wire`, `tri`, `reg`, `supply0`, `supply1`, ... declaration in the body.
             */
            struct NetDecl
            {
                std::string name;
                std::string net_type;
                std::vector<Range> packed_dims;
                std::vector<Range> unpacked_dims;
                std::optional<Expr> initializer; /**< `wire d = a;` */
                std::vector<Attribute> attributes;
                Location location;
            };

            /**
             * An entry of the module header port list. Plain form `a`, or `.name(expression)` where the expression
             * says which signals the port stands for inside the module.
             */
            struct HeaderPort
            {
                std::string name;
                std::optional<Expr> expression;
                Location location;
            };

            /**
             * One connection of an instance: `.port(expr)`, or positional when `port` is empty.
             */
            struct Connection
            {
                std::string port;
                Expr expr;
                Location location;
            };

            /**
             * One parameter assignment of an instance: `.P(expr)` or positional when `name` is empty.
             */
            struct ParameterAssignment
            {
                std::string name;
                Expr value;
                Location location;
            };

            /**
             * An instantiation of a module or a gate type.
             */
            struct Instantiation
            {
                std::string type;
                std::string name;
                std::vector<ParameterAssignment> parameters;
                std::vector<Connection> connections;
                std::vector<Attribute> attributes;
                Location location;
            };

            /**
             * A continuous assignment `assign lhs = rhs;`.
             */
            struct Assignment
            {
                Expr lhs;
                Expr rhs;
                std::vector<Attribute> attributes;
                Location location;
            };

            /**
             * A `defparam inst.P = value;`, with the instance path as written.
             */
            struct Defparam
            {
                std::vector<std::string> path; /**< The instance path, last element is the parameter name. */
                Expr value;
                Location location;
            };

            /**
             * A module.
             */
            struct Module
            {
                std::string name;
                std::vector<Attribute> attributes;
                std::vector<HeaderPort> header_ports;
                bool ansi_ports = false; /**< The header declared the ports itself. */
                std::vector<ParameterDecl> parameters;
                std::vector<PortDecl> ports; /**< In declaration order; for ANSI headers this is the header order. */
                std::vector<NetDecl> nets;
                std::vector<Assignment> assignments;
                std::vector<Instantiation> instantiations;
                std::vector<Defparam> defparams;
                Location location;
            };

            /**
             * A parsed file.
             */
            struct SourceFile
            {
                std::string file;
                std::vector<Module> modules;
            };
        }    // namespace ast
    }    // namespace verilog
}    // namespace hal
