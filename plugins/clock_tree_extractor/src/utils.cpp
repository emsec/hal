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

#include "clock_tree_extractor/utils.h"

#include "hal_core/defines.h"
#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/enums/gate_type_property.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/utilities/result.h"

#include <charconv>
#include <cmath>
#include <locale>
#include <sstream>

namespace hal::cte
{
    bool is_ff( const Gate *gate )
    {
        return gate->get_type()->has_property( GateTypeProperty::ff );
    }

    bool is_latch( const Gate *gate )
    {
        return gate->get_type()->has_property( GateTypeProperty::latch );
    }

    bool is_buffer( const Gate *gate )
    {
        return gate->get_type()->has_property( GateTypeProperty::c_buffer );
    }

    bool is_inverter( const Gate *gate )
    {
        return gate->get_type()->has_property( GateTypeProperty::c_inverter );
    }

    bool is_delay( const Gate *gate )
    {
        return gate->get_type()->has_property( GateTypeProperty::delay );
    }

    bool is_mux( const Gate *gate )
    {
        return gate->get_type()->has_property( GateTypeProperty::c_mux );
    }

    bool is_oscillator( const Gate *gate )
    {
        return gate->get_type()->has_property( GateTypeProperty::oscillator );
    }

    bool is_pll( const Gate *gate )
    {
        return gate->get_type()->has_property( GateTypeProperty::pll );
    }

    bool is_control_pin( PinType pin_type )
    {
        return pin_type == PinType::clock || pin_type == PinType::enable || pin_type == PinType::select
            || pin_type == PinType::set || pin_type == PinType::reset;
    }

    bool is_connected_to_control_pin( const Endpoint *endpoint )
    {
        return is_control_pin( endpoint->get_pin()->get_type() );
    }

    std::optional<double> read_parameter_coordinate( const Gate *gate, const std::string &name )
    {
        const Result<std::string> parameter = gate->get_parameter_value( name );
        if( parameter.is_error() )
        {
            return std::nullopt;
        }

        const std::string &value = parameter.get();
        const std::size_t first = value.find_first_not_of( " \t\r\n" );
        if( first == std::string::npos )
        {
            return std::nullopt;
        }
        const std::string text = value.substr( first, value.find_last_not_of( " \t\r\n" ) - first + 1 );

        // bit vectors are stored in hexadecimal, which not every standard library reads as a double
        if( text.rfind( "0x", 0 ) == 0 )
        {
            u64 coordinate = 0;
            if( const auto [end, error] = std::from_chars( text.data() + 2, text.data() + text.size(), coordinate, 16 );
                error != std::errc() || end != text.data() + text.size() )
            {
                return std::nullopt;
            }
            return static_cast<double>( coordinate );
        }

        // read with the classic locale, so a coordinate has a decimal point no matter where the program runs
        std::istringstream stream( text );
        stream.imbue( std::locale::classic() );

        double coordinate = 0.0;
        stream >> coordinate;

        // the whole value must be the number, "12 um" is not a coordinate
        if( stream.fail() || !stream.eof() || !std::isfinite( coordinate ) )
        {
            return std::nullopt;
        }
        return coordinate;
    }
}  // namespace hal::cte
