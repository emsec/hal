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
 * @file vhdl_ast.h
 * @brief The syntax tree of a structural VHDL file, as written, before any name is resolved or any range expanded.
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
    namespace vhdl
    {
        namespace ast
        {
            using Location = netlist_ir::Location;

            /**
             * A name as written: basic or extended. Two names refer to the same thing when their `key()` is equal.
             */
            struct Name
            {
                std::string text;
                bool extended = false;

                std::string key() const;
                bool operator==(const Name& other) const
                {
                    return key() == other.key();
                }
            };

            /**
             * An expression: a wiring expression in an association or an assignment, or a constant expression in a
             * range, a generic, or an attribute. The tree keeps what the file says; elaboration decides what it means.
             */
            struct Expr
            {
                enum class Kind
                {
                    Empty,        /**< Nothing. */
                    Open,         /**< The keyword `open`. */
                    Identifier,   /**< A name; `name` holds it, `prefix` any selected prefix such as `work.pkg`. */
                    Number,       /**< An integer or real; `text` as written. */
                    Character,    /**< A character literal; `text` is the character. */
                    String,       /**< A string literal; `text` without quotes. */
                    BitString,    /**< A bit string literal; `text` as written. */
                    Index,        /**< `base(i, j)`: children are the base and the indices. */
                    Slice,        /**< `base(l downto r)` / `base(l to r)`: children are the base, left and right; `text` is `downto` or `to`. */
                    Attribute,    /**< `base'attr`: children are the base; `text` is the attribute name. */
                    Concat,       /**< `a & b & ...`: children are the items, left to right. */
                    Aggregate,    /**< `(a, b, others => c)`: children are the elements; an element with `choices` set is `choice => value`. */
                    Unary,        /**< An operator with one operand; `text` holds the operator. */
                    Binary,       /**< An operator with two operands; `text` holds the operator. */
                    Others        /**< The keyword `others` as an aggregate choice. */
                };

                Kind kind = Kind::Empty;
                std::string text;
                Name name;
                std::vector<Name> prefix;    /**< For a selected name `a.b.c`, the leading parts `a`, `b`. */
                std::vector<Expr> children;
                std::vector<Expr> choices;    /**< For an aggregate element: the choices before `=>`. */
                Location location;

                bool is_empty() const
                {
                    return kind == Kind::Empty;
                }
            };

            /**
             * A discrete range `left to right` or `left downto right`.
             */
            struct Range
            {
                Expr left;
                Expr right;
                bool descending = false;
            };

            /**
             * A subtype indication such as `std_logic`, `std_logic_vector(7 downto 0)`, `work.t_bus`, or the HAL
             * pseudo types `std_logic_vector2(0 to 1, 2 to 3)`.
             */
            struct TypeMark
            {
                Name name;
                std::vector<Name> prefix;
                std::vector<Range> constraints;    /**< The index constraint, one range per dimension; empty for a scalar. */
                Location location;
            };

            /**
             * A generic or port declaration.
             */
            struct InterfaceDecl
            {
                Name name;
                PinDirection mode = PinDirection::none;    /**< `none` for a generic, `input`/`output`/`inout` for a port (`buffer` and `linkage` map to output and inout). */
                TypeMark type;
                std::optional<Expr> default_value;
                Location location;
            };

            /**
             * A signal or constant declaration in an architecture or package.
             */
            struct ObjectDecl
            {
                Name name;
                bool is_constant = false;
                TypeMark type;
                std::optional<Expr> initializer;
                Location location;
            };

            /**
             * `attribute name : type;`
             */
            struct AttributeDecl
            {
                Name name;
                Name type;
                Location location;
            };

            /**
             * `attribute name of target : class is value;`
             */
            struct AttributeSpec
            {
                Name attribute;
                std::vector<Name> targets;    /**< `others` and `all` are not supported and rejected by the parser. */
                std::string entity_class;    /**< `signal`, `label`, `entity`, `component`, `architecture`, ... in lower case. */
                Expr value;
                Location location;
            };

            /**
             * A component declaration.
             */
            struct ComponentDecl
            {
                Name name;
                std::vector<InterfaceDecl> generics;
                std::vector<InterfaceDecl> ports;
                Location location;
            };

            /**
             * A type or subtype declaration that the netlist subset understands: an array type `type t is array (0 to
             * 3) of std_logic;` becomes a vector type, a subtype `subtype t is std_logic_vector(3 downto 0);` stands
             * for its base type; anything else is recorded by name only and rejected when used.
             */
            struct TypeDecl
            {
                Name name;
                bool is_array = false;
                std::vector<Range> ranges;         /**< The index ranges of an array type; empty for an unconstrained array. */
                TypeMark element;                  /**< The element type of an array type. */
                std::optional<TypeMark> subtype;    /**< The type a subtype stands for. */
                Location location;
            };

            /**
             * One association in a generic or port map: `formal => actual` or positional.
             */
            struct Association
            {
                std::optional<Expr> formal;    /**< A name, possibly with an index or slice; absent for positional. */
                Expr actual;
                Location location;
            };

            /**
             * A component instantiation statement.
             */
            struct Instantiation
            {
                enum class Kind
                {
                    Component,    /**< `label : comp` or `label : component comp`. */
                    Entity,       /**< `label : entity work.e(arch)`. */
                    Configuration /**< `label : configuration work.cfg`. */
                };

                Name label;
                Kind kind = Kind::Component;
                std::vector<Name> prefix;    /**< The library and package parts before the unit name. */
                Name unit;                   /**< The component, entity, or configuration name. */
                std::optional<Name> architecture;
                std::vector<Association> generic_map;
                std::vector<Association> port_map;
                Location location;
            };

            /**
             * A concurrent signal assignment `target <= value;`.
             */
            struct Assignment
            {
                std::optional<Name> label;
                Expr target;
                Expr value;
                Location location;
            };

            /**
             * A library or use clause, kept as written.
             */
            struct ContextClause
            {
                bool is_use = false;
                std::vector<Name> parts;
                Location location;
            };

            /**
             * The declarations an architecture or a package may hold.
             */
            struct Declarations
            {
                std::vector<ObjectDecl> objects;
                std::vector<ComponentDecl> components;
                std::vector<TypeDecl> types;
                std::vector<AttributeDecl> attribute_decls;
                std::vector<AttributeSpec> attribute_specs;
            };

            struct Entity
            {
                Name name;
                std::vector<InterfaceDecl> generics;
                std::vector<InterfaceDecl> ports;
                Declarations declarations;    /**< The entity declarative part, in practice attribute specifications. */
                Location location;
            };

            struct Architecture
            {
                Name name;
                Name entity;
                Declarations declarations;
                std::vector<Instantiation> instantiations;
                std::vector<Assignment> assignments;
                Location location;
            };

            struct Package
            {
                Name name;
                Declarations declarations;
                Location location;
            };

            /**
             * A configuration declaration; only `for all : comp use entity work.e(arch);` bindings are kept.
             */
            struct Configuration
            {
                struct Binding
                {
                    Name component;
                    std::vector<Name> prefix;
                    Name entity;
                    std::optional<Name> architecture;
                };

                Name name;
                Name entity;
                std::vector<Binding> bindings;
                Location location;
            };

            /**
             * A parsed file.
             */
            struct SourceFile
            {
                std::string file;
                std::vector<ContextClause> context;
                std::vector<Entity> entities;
                std::vector<Architecture> architectures;
                std::vector<Package> packages;
                std::vector<Configuration> configurations;
            };
        }    // namespace ast
    }    // namespace vhdl
}    // namespace hal
