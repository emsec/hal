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

#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/enums/gate_type_property.h"
#include "hal_core/netlist/gate_library/gate_type.h"

namespace hal
{
    namespace cte
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

        bool is_control_pin( const PinType &pin_type )
        {
            return pin_type == PinType::clock || pin_type == PinType::enable || pin_type == PinType::select
                || pin_type == PinType::set || pin_type == PinType::reset;
        }

        bool is_connected_to_control_pin( const Endpoint *endpoint )
        {
            return is_control_pin( endpoint->get_pin()->get_type() );
        }
    }  // namespace cte
}  // namespace hal
