//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/gate_library/enums/async_set_reset_behavior.h"
#include "hal_core/netlist/pins/gate_pin.h"
#include "hal_core/utilities/log.h"

namespace hal
{
    namespace simulation_utils
    {
        BooleanFunction::Value toggle(BooleanFunction::Value v);
        BooleanFunction::Value process_clear_preset_behavior(AsyncSetResetBehavior behavior, BooleanFunction::Value previous_output);
        std::vector<BooleanFunction::Value> int_to_values(u32 integer, u32 len);
        u32 values_to_int(const std::vector<BooleanFunction::Value>& values);
        u32 get_int_bus_value(const std::unordered_map<std::string, BooleanFunction::Value>& signal_values, const std::vector<GatePin*>& ordered_pins);
    }    // namespace simulation_utils
}    // namespace hal