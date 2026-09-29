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
 * @file netlist_ir.h
 * @brief The language-neutral intermediate representation that netlist parsers produce and the shared instantiation consumes.
 */

#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/gate_library/enums/pin_direction.h"
#include "hal_core/netlist/parameter.h"
#include "hal_core/utilities/result.h"

#include <deque>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace hal
{
    /**
     * The netlist intermediate representation (IR).
     *
     * A front end for a netlist format (Verilog, VHDL, EDIF, or a Python script) builds a `Design` from a file. The
     * design is a set of modules; every module owns its ports, signals, instances, and aliases. Ports and signals are
     * expanded to individual bits, and every bit has a module-local handle (`BitId`) that connections and aliases refer
     * to. Nothing in the IR depends on the format the design came from, and nothing depends on a gate library: the
     * gate library only enters when the design is instantiated into a `Netlist`.
     *
     * @ingroup netlist
     */
    namespace netlist_ir
    {
        /** Module-local handle for one net bit. */
        using BitId = u32;

        /** The bit that is constant `0`; every module has it. */
        constexpr BitId ZERO = 0;

        /** The bit that is constant `1`; every module has it. */
        constexpr BitId ONE = 1;

        /** The first handle a module hands out for its own bits. */
        constexpr BitId FIRST_USER_BIT = 2;

        /** Not a bit: a position in a connection or an assignment that is left open, e.g. a `z` or `x` in a literal. */
        constexpr BitId OPEN = 0xFFFFFFFFu;

        /**
         * One index range of a port or signal, as declared: Verilog `[7:0]` is `{7, 0}`, VHDL `(0 to 3)` is `{0, 3}`.
         */
        struct NETLIST_API Range
        {
            /** The index written first. */
            i32 left = 0;

            /** The index written last. */
            i32 right = 0;

            /**
             * Get the number of indices in the range.
             *
             * @returns The number of indices.
             */
            u32 size() const;

            /**
             * Check whether the range runs from a high index down to a low one.
             *
             * @returns `true` for a descending range such as `[7:0]`, `false` otherwise.
             */
            bool is_descending() const;

            /**
             * Get the index at a position, counted from the left end of the range.
             *
             * @param[in] offset - The position, `0` for the left end.
             * @returns The index at that position.
             */
            i32 index_at(u32 offset) const;

            /**
             * Get the position of an index, counted from the left end of the range.
             *
             * @param[in] index - The index.
             * @returns The position, or `std::nullopt` if the index is not in the range.
             */
            std::optional<u32> offset_of(i32 index) const;

            bool operator==(const Range& other) const;
            bool operator!=(const Range& other) const;
        };

        /**
         * Where something was read from: a line and a column of the design's source file, both `0` when unknown.
         */
        struct NETLIST_API Location
        {
            u32 line   = 0;
            u32 column = 0;

            /**
             * Format the location for a message.
             *
             * @returns `line 12, column 4`, or an empty string when unknown.
             */
            std::string to_string() const;
        };

        /**
         * A named, typed value: a parameter or generic, or an attribute such as a Verilog `(* keep = "true" *)`; the
         * declaration's source tells which.
         *
         * The declaration carries the name, the type and the source. A front end infers the type from the literal
         * form in the file: an integer, a string, a bit vector of the literal's width, a boolean for a flag without a
         * value. When the gate type of an instance declares a parameter of the same name, the gate type's declaration
         * wins at instantiation; attributes are never declared by gate types. A generic and an attribute may share a
         * name.
         */
        struct NETLIST_API TypedValue
        {
            Parameter declaration;
            std::string value;
        };

        /**
         * A signal of a module, expanded to bits.
         *
         * `bits` lists one handle per bit in declaration order: the index written first in the outermost dimension
         * comes first, and the innermost dimension runs fastest. A scalar has no dimensions and one bit.
         */
        struct NETLIST_API Signal
        {
            std::string name;

            /** The index ranges as declared, outermost first; empty for a scalar. */
            std::vector<Range> dims;

            /** One handle per bit, in declaration order. */
            std::vector<BitId> bits;

            /** The typed values, in practice attributes. */
            std::vector<TypedValue> parameters;
            Location location;

            /**
             * Get the number of bits.
             *
             * @returns The number of bits.
             */
            u32 width() const;

            /**
             * Get the bit at the given indices, one per dimension.
             *
             * @param[in] indices - One declared index per dimension, outermost first.
             * @returns The bit on success, an error if the number of indices or an index is out of range.
             */
            Result<BitId> bit_at(const std::vector<i32>& indices) const;

            /**
             * Get the bits of a part of a one-dimensional signal, in the order the given range lists them.
             *
             * @param[in] range - The indices to select, `{1, 3}` selects the bits at indices 1, 2, 3 in that order.
             * @returns The bits on success, an error if the signal is not one-dimensional or the range is not contained.
             */
            Result<std::vector<BitId>> slice(const Range& range) const;

            /**
             * Get the name of one bit as the netlist will show it: `name` for a scalar, `name(3)` for a vector bit,
             * `name(1)(0)` for a bit of a two-dimensional signal.
             *
             * @param[in] position - The position of the bit in `bits`.
             * @returns The name.
             */
            std::string bit_name(u32 position) const;
        };

        /**
         * A port of a module. A port is a signal of the module with a direction, so everything said about `Signal`
         * applies; a port is not repeated in the module's signal list.
         */
        struct NETLIST_API Port : public Signal
        {
            PinDirection direction = PinDirection::none;
        };

        /**
         * The bits connected to one port of an instance.
         *
         * `bits` are in expression order, left to right as written, so the last bit is bit 0 of the port. A connection
         * to a constant refers to `ZERO` or `ONE`; a pin that is left open has no connection, and a single position
         * that is open, e.g. an `x` or `z` bit of a literal, is `OPEN`.
         */
        struct NETLIST_API Connection
        {
            /** The pin group of a gate or the port of a module; empty for a positional connection. */
            std::string port;

            /** The part of the port that is connected, for VHDL `DATA_OUT(3 downto 1) => ...`; the whole port if empty. */
            std::optional<Range> port_slice;

            /** The connected bits in expression order; `OPEN` leaves that position unconnected. */
            std::vector<BitId> bits;

            /**
             * The single bit in `bits` connects to every bit of the port, for VHDL `(others => '0')` on a port whose
             * width the front end does not know. Requires exactly one bit.
             */
            bool replicate = false;
        };

        /**
         * Whether an instance refers to a gate type of the library or to a module of the design.
         */
        enum class InstanceKind
        {
            Gate,
            Module,
        };

        /**
         * An instance of a gate type or of a module inside a module.
         */
        struct NETLIST_API Instance
        {
            std::string name;

            /** The gate type name or the module name. */
            std::string type;

            InstanceKind kind = InstanceKind::Gate;

            /** Either every connection is named or every connection is positional. */
            std::deque<Connection> connections;

            /** The generics set on the instance and its attributes. */
            std::vector<TypedValue> parameters;
            Location location;

            /**
             * Add a connection.
             *
             * @param[in] port - The port name, empty for a positional connection.
             * @param[in] bits - The connected bits in expression order.
             * @param[in] port_slice - The part of the port that is connected, if not the whole port.
             * @returns The new connection; the reference stays valid while the instance exists.
             */
            Connection& add_connection(const std::string& port, const std::vector<BitId>& bits, const std::optional<Range>& port_slice = std::nullopt);

            /**
             * Find a connection by port name.
             *
             * @param[in] port - The port name.
             * @returns The connection, or `nullptr` if there is none.
             */
            const Connection* find_connection(const std::string& port) const;
        };

        /**
         * A module: the unit a file declares, i.e., a Verilog module, a VHDL entity with its architecture, an EDIF cell.
         */
        struct NETLIST_API Module
        {
            std::string name;

            std::deque<Port> ports;
            std::deque<Signal> signals;
            std::deque<Instance> instances;

            /**
             * Pairs of bits that are the same net, from assignments, initializers, and port expressions. The first bit
             * of a pair is the receiving side (`assign first = second;`), which decides the name of the merged net.
             */
            std::vector<std::pair<BitId, BitId>> aliases;

            /** The declared parameters or generics with their default values, and the module's attributes. */
            std::vector<TypedValue> parameters;
            Location location;

            /**
             * Allocate a new bit handle.
             *
             * @returns The handle.
             */
            BitId new_bit();

            /**
             * Allocate consecutive bit handles.
             *
             * @param[in] count - How many.
             * @returns The handles in allocation order.
             */
            std::vector<BitId> new_bits(u32 count);

            /**
             * Get the next handle that `new_bit` would hand out; every handle of the module is below it.
             *
             * @returns The next handle.
             */
            BitId next_bit() const;

            /**
             * Add a port with freshly allocated bits.
             *
             * @param[in] name - The port name.
             * @param[in] direction - The direction.
             * @param[in] dims - The index ranges, outermost first; empty for a scalar.
             * @returns The new port; the reference stays valid while the module exists.
             */
            Port& add_port(const std::string& name, PinDirection direction, const std::vector<Range>& dims = {});

            /**
             * Add a signal with freshly allocated bits.
             *
             * @param[in] name - The signal name.
             * @param[in] dims - The index ranges, outermost first; empty for a scalar.
             * @returns The new signal; the reference stays valid while the module exists.
             */
            Signal& add_signal(const std::string& name, const std::vector<Range>& dims = {});

            /**
             * Add an instance.
             *
             * @param[in] name - The instance name.
             * @param[in] type - The gate type name or module name.
             * @param[in] kind - Whether the type is a gate type or a module.
             * @returns The new instance; the reference stays valid while the module exists.
             */
            Instance& add_instance(const std::string& name, const std::string& type, InstanceKind kind);

            /**
             * Record that two bits are the same net.
             *
             * @param[in] a - The receiving bit, the left side of an assignment.
             * @param[in] b - The other bit.
             */
            void add_alias(BitId a, BitId b);

            /**
             * Find a port by name.
             *
             * @param[in] name - The port name.
             * @returns The port, or `nullptr` if there is none.
             */
            const Port* find_port(const std::string& name) const;

            /**
             * Find a signal or a port by name.
             *
             * @param[in] name - The name.
             * @returns The signal, or `nullptr` if there is none.
             */
            const Signal* find_signal(const std::string& name) const;

            /**
             * Find an instance by name.
             *
             * @param[in] name - The instance name.
             * @returns The instance, or `nullptr` if there is none.
             */
            const Instance* find_instance(const std::string& name) const;

            /**
             * Find an instance by name.
             *
             * @param[in] name - The instance name.
             * @returns The instance, or `nullptr` if there is none.
             */
            Instance* find_instance(const std::string& name);

        private:
            BitId m_next_bit = FIRST_USER_BIT;
        };

        /**
         * A design: every module a file declares, and which one is the top.
         */
        struct NETLIST_API Design
        {
            std::deque<Module> modules;

            /** The top module if the file names one, for example through `(* top = 1 *)`; otherwise `find_top` derives it. */
            std::optional<std::string> top;

            /** The file the design was read from, for messages. */
            std::string source;

            /**
             * Add a module.
             *
             * @param[in] name - The module name.
             * @returns The new module; the reference stays valid while the design exists.
             */
            Module& add_module(const std::string& name);

            /**
             * Find a module by name.
             *
             * @param[in] name - The module name.
             * @returns The module, or `nullptr` if there is none.
             */
            const Module* find_module(const std::string& name) const;

            /**
             * Find a module by name.
             *
             * @param[in] name - The module name.
             * @returns The module, or `nullptr` if there is none.
             */
            Module* find_module(const std::string& name);

            /**
             * Determine the top module: the one named by `top` if set, otherwise the single module that no other module
             * instantiates.
             *
             * @returns The name of the top module, or an error naming every candidate if there is not exactly one.
             */
            Result<std::string> find_top() const;

            /**
             * Check everything the instantiation relies on: unique names, valid bit handles, consistent widths, existing
             * module types, named or positional connections but not both, and connection widths against module ports.
             * Anything that needs the gate library is checked at instantiation instead.
             *
             * @returns Ok on success, an error describing the first problem otherwise.
             */
            Result<std::monostate> validate() const;
        };
    }    // namespace netlist_ir
}    // namespace hal
