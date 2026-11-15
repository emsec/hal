// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
// Copyright (c) 2025-2026 Sascha Tommasone. All rights reserved.
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

#include "hal_core/netlist/gate_library/enums/pin_type.h"

#include <optional>
#include <string>

namespace hal
{
    class Endpoint;
    class Gate;

    namespace cte
    {
        bool is_ff( const Gate *gate );
        bool is_latch( const Gate *gate );
        bool is_buffer( const Gate *gate );
        bool is_inverter( const Gate *gate );
        bool is_delay( const Gate *gate );
        bool is_mux( const Gate *gate );
        bool is_oscillator( const Gate *gate );
        bool is_pll( const Gate *gate );
        bool is_control_pin( PinType pin_type );
        bool is_connected_to_control_pin( const Endpoint *endpoint );

        /**
         * Read a coordinate from a gate parameter, e.g., a Verilog instance parameter.
         *
         * The parameter must hold a finite number and nothing else apart from surrounding white space, e.g., "-600",
         * "12.5" or "0x10", which is how HAL stores bit vectors.
         *
         * @param[in] gate - The gate.
         * @param[in] name - The name of the parameter.
         * @returns The coordinate, or std::nullopt if the parameter is missing or holds no number.
         */
        std::optional<double> read_parameter_coordinate( const Gate *gate, const std::string &name );
    }  // namespace cte
}  // namespace hal
