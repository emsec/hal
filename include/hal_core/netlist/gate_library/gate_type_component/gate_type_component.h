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

#include "hal_core/defines.h"
#include "hal_core/netlist/gate_library/gate_type_component/buffer_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/carry_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/init_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/io_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/latch_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/mac_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/mux_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/ram_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/ram_port_component.h"

#include <functional>
#include <set>

namespace hal
{
    class GateTypeComponent
    {
    public:
        enum class ComponentType
        {
            // power,
            // ground,
            buffer,
            mux,
            carry,
            lut,
            ff,
            latch,
            ram,
            mac,
            io,
            init,
            ram_port,
        };

        // factory methods
        static std::unique_ptr<GateTypeComponent> create_buffer_component()
        {
            return std::make_unique<BufferComponent>();
        }

        static std::unique_ptr<GateTypeComponent> create_mux_component()
        {
            return std::make_unique<MUXComponent>();
        }

        static std::unique_ptr<GateTypeComponent> create_carry_component()
        {
            return std::make_unique<CarryComponent>();
        }

        static std::unique_ptr<GateTypeComponent> create_lut_component(std::unique_ptr<GateTypeComponent> component, bool init_ascending)
        {
            if (component == nullptr)
            {
                return nullptr;
            }

            return std::make_unique<LUTComponent>(std::move(component), init_ascending);
        }

        static std::unique_ptr<GateTypeComponent> create_ff_component(std::unique_ptr<GateTypeComponent> component, const BooleanFunction& next_state_bf, const BooleanFunction& clock_bf)
        {
            if (component == nullptr)
            {
                return nullptr;
            }

            return std::make_unique<FFComponent>(std::move(component), next_state_bf, clock_bf);
        }

        static std::unique_ptr<GateTypeComponent> create_latch_component(std::unique_ptr<GateTypeComponent> component, const BooleanFunction& data_in_bf, const BooleanFunction& enable_bf)
        {
            if (component == nullptr)
            {
                return nullptr;
            }

            return std::make_unique<LatchComponent>(std::move(component), data_in_bf, enable_bf);
        }

        static std::unique_ptr<GateTypeComponent> create_ram_component(std::unique_ptr<GateTypeComponent> component)
        {
            // TODO do fancy RAM stuff
            if (component == nullptr)
            {
                return nullptr;
            }

            return std::make_unique<RAMComponent>(std::move(component));
        }

        static std::unique_ptr<GateTypeComponent> create_mac_component()
        {
            // TODO do fancy MAC stuff
            return std::make_unique<MACComponent>();
        }

        static std::unique_ptr<GateTypeComponent> create_io_component()
        {
            // TODO do fancy IO stuff
            return std::make_unique<IOComponent>();
        }

        static std::unique_ptr<GateTypeComponent> create_init_component(const std::string& init_category, const std::string& init_identifier)
        {
            return std::make_unique<InitComponent>(init_category, init_identifier);
        }

        static std::unique_ptr<GateTypeComponent> create_ram_port_component()
        {
            // TODO do fancy RAM stuff
            return std::make_unique<RAMPortComponent>();
        }

        virtual ComponentType get_type() const = 0;

        template<typename T>
        const T* get_as() const
        {
            if (T::is_class_of(this))
            {
                return static_cast<const T*>(this);
            }

            return nullptr;
        }

        virtual std::set<GateTypeComponent*> get_components(const std::function<bool(const GateTypeComponent*)>& filter = nullptr) const = 0;
    };

    // class LUTComponent : public GateTypeComponent;

}    // namespace hal