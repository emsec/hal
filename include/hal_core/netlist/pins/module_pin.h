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
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/pins/base_pin.h"
#include "hal_core/netlist/pins/pin_group.h"

#include <string>

namespace hal
{
    class Module;

    /**
     * The pin of a module. Each pin has a name, a direction, and a type and is associated with a net. 
     * 
     * @ingroup pins
     */
    class ModulePin : public BasePin<ModulePin>
    {
    public:
        /**
         * Construct a new module pin from its name, net, direction and type.
         * 
         * @param[in] id - The pin ID.
         * @param[in] name - The pin name.
         * @param[in] net - The net passing through the pin.
         * @param[in] direction - The direction of the pin.
         * @param[in] type - The type of the pin.
         */
        ModulePin(const u32 id, const std::string& name, Net* net, PinDirection direction, PinType type = PinType::none);

        /**
         * Check whether two module pins are equal.
         *
         * @param[in] other - The module pin to compare against.
         * @returns True if both module pins are equal, false otherwise.
         */
        bool operator==(const ModulePin& other) const;

        /**
         * Check whether two module pins are unequal.
         *
         * @param[in] other - The module pin to compare against.
         * @returns True if both module pins are unequal, false otherwise.
         */
        bool operator!=(const ModulePin& other) const;

        /**
         * Get the net passing through the pin.
         * 
         * @returns The net of the pin.
         */
        Net* get_net() const;

    private:
        Net* m_net;

        ModulePin(const ModulePin&)            = delete;
        ModulePin(ModulePin&&)                 = delete;
        ModulePin& operator=(const ModulePin&) = delete;
        ModulePin& operator=(ModulePin&&)      = delete;
    };
}    // namespace hal