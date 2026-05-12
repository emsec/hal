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

#include "hal_core/defines.h"
#include "hal_core/utilities/enums.h"
#include "hal_core/utilities/result.h"

#include <map>
#include <string>
#include <vector>

namespace hal
{
    /**
     * A typed, named parameter declaration.
     *
     * @ingroup netlist
     */
    struct NETLIST_API Parameter
    {
        /**
         * Type tag for typed parameters across the netlist.
         */
        enum class Type
        {
            /** A logical boolean (`true` or `false`). */
            Boolean,
            /** A multi-bit unsigned integer value (2-state). */
            BitVector,
            /** A multi-bit 9-state logic vector; each bit may be `0`, `1`, `X`, `Z`, `U`, `L`, `H`, `W`, or `-`. */
            LogicVector,
            /** A signed 64-bit integer in decimal. */
            Integer,
            /** An arbitrary string. */
            String,
            /** A double-precision floating-point number. */
            Float,
            /** A time value with a unit suffix (`fs`, `ps`, `ns`, `us`, `ms`, `s`, `min`, `h`). */
            Time,
            /** A finite, ordered set of named values; encoded as an integer index. */
            Enum,
        };

        /**
         * Constructs an empty / invalid Parameter.
         */
        explicit Parameter();

        /**
         * Get the name of the parameter.
         *
         * @returns The parameter name.
         */
        const std::string& get_name() const;

        /**
         * Get the data type of the parameter.
         *
         * @returns The parameter type.
         */
        Type get_type() const;

        /**
         * Get the bit-width of the parameter.
         * Fixed at 1 for `Boolean`, 64 for `Integer` and `Float`, 0 for `String` and `Time`,
         * `ceil(log2(N))` for `Enum` with N values, and the declared width for `BitVector` / `LogicVector`.
         *
         * @returns The bit-width.
         */
        u16 get_size() const;

        /**
         * Get the default value of the parameter as a string.
         * An empty string signals "no default"; all other values satisfy the type's grammar.
         *
         * @returns The default value string.
         */
        const std::string& get_default_value() const;

        /**
         * Get the ordered list of named values for an `Enum` parameter.
         * Always empty for every other type.
         *
         * @returns The enum value list.
         */
        const std::vector<std::string>& get_enum_values() const;

        /**
         * Construct a boolean parameter. Accepts only the value strings `"true"` and `"false"`.
         *
         * @param[in] name - Parameter name (must be non-empty).
         * @param[in] default_value - Default value, either `"true"` or `"false"`.
         */
        static Result<Parameter> Boolean(const std::string& name, const std::string& default_value);

        /**
         * Construct a bit-vector parameter. Values use the prefixes
         * `0b` / `0o` / `0x`.
         *
         * @param[in] name - Parameter name (must be non-empty).
         * @param[in] size - Bit-width (must be in [1, 64]).
         * @param[in] default_value - Default value as a string.
         */
        static Result<Parameter> BitVector(const std::string& name, u16 size, const std::string& default_value);

        /**
         * Construct a 9-state logic-vector parameter. Values use the prefixes
         * `0b` / `0o` / `0x`, with any of the VHDL `std_logic` state characters
         * (`X`, `Z`, `U`, `L`, `H`, `W`, `-`) appearing in place of digits
         * (case-insensitive; for octal/hex, a state-character digit expands to
         * 3/4 copies of that state).
         *
         * @param[in] name - Parameter name (must be non-empty).
         * @param[in] size - Bit-width (must be in [1, 64]).
         * @param[in] default_value - Default value as a 9-state literal string (empty for "no default").
         */
        static Result<Parameter> LogicVector(const std::string& name, u16 size, const std::string& default_value);

        /**
         * Construct a signed-integer parameter. Values are parsed in base 10 and must fit in `i64`.
         *
         * @param[in] name - Parameter name (must be non-empty).
         * @param[in] default_value - Default value as a decimal string (optional leading `+`/`-`).
         */
        static Result<Parameter> Integer(const std::string& name, const std::string& default_value);

        /**
         * Construct a string parameter. Any value (including the empty string) is accepted.
         *
         * @param[in] name - Parameter name (must be non-empty).
         * @param[in] default_value - Default value.
         */
        static Result<Parameter> String(const std::string& name, const std::string& default_value);

        /**
         * Construct a floating-point parameter. Values are parsed via `std::stod`.
         *
         * @param[in] name - Parameter name (must be non-empty).
         * @param[in] default_value - Default value as a decimal float (e.g. `3.14`, `1e-5`).
         */
        static Result<Parameter> Float(const std::string& name, const std::string& default_value);

        /**
         * Construct a time parameter. Values are of the form `<number><unit>` with the unit
         * in {`fs`, `ps`, `ns`, `us`, `ms`, `s`, `min`, `h`}; the number is a non-negative
         * decimal.
         *
         * @param[in] name - Parameter name (must be non-empty).
         * @param[in] default_value - Default value, e.g. `10ns`.
         */
        static Result<Parameter> Time(const std::string& name, const std::string& default_value);

        /**
         * Construct an enum parameter. `size` is auto-computed as `ceil(log2(values.size()))`.
         *
         * @param[in] name - Parameter name (must be non-empty).
         * @param[in] values - Ordered list of enum values (at least two, unique, non-empty).
         * @param[in] default_value - Default value, must equal one of ``values``.
         */
        static Result<Parameter> Enum(const std::string& name, const std::vector<std::string>& values, const std::string& default_value);

        /**
         * Check whether a candidate value is valid for this parameter declaration.
         *
         * Per-type rules:
         *  - Boolean: value must be `true` or `false`.
         *  - BitVector: parses as a non-negative integer (`0b`, `0o`, `0x`) that fits in `size`.
         *  - LogicVector: parses as a 4-state literal (`0b`, `0o`, `0x`) whose total bit-length fits in `size`.
         *  - Integer: parses as a signed decimal integer (fits in `i64`).
         *  - String: any value (including the empty string) is accepted.
         *  - Float: parses as a `double` via `std::stod`.
         *  - Time: matches `<number><unit>` with unit in {`fs`, `ps`, `ns`, `us`, `ms`, `s`, `min`, `h`}.
         *  - Enum: must equal one of the declared enum values.
         *
         * @param[in] value - The candidate value string.
         * @returns `true` if the value is valid, `false` otherwise.
         */
        bool validate(const std::string& value) const;

        /**
         * Convert a string value to its canonical integer encoding for this parameter.
         *
         * Per-type rules:
         *  - Boolean: returns 0 for `false`, 1 for `true`.
         *  - BitVector: parses the value (`0b`, `0o`, `0x`).
         *  - Integer: returns the bit-pattern of the signed value reinterpreted as `u64`.
         *  - LogicVector, String, Float, Time: no canonical integer encoding; returns an error.
         *  - Enum: returns the index of the value in the declared value list.
         *
         * @param[in] value - The string value to encode.
         * @returns OK() and the integer encoding on success, an error otherwise.
         */
        Result<u64> encode_as_int(const std::string& value) const;

        bool operator==(const Parameter& other) const;
        bool operator!=(const Parameter& other) const;

    private:
        std::string name;
        Type type;
        u16 size = 0;
        std::string default_value = "";
        std::vector<std::string> enum_values = {};
    };

    template<>
    std::map<Parameter::Type, std::string> EnumStrings<Parameter::Type>::data;
}    // namespace hal
