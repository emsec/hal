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

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/gate_library/gate_type_component/gate_type_component.h"

namespace hal
{
    class RAMPortComponent : public GateTypeComponent
    {
    public:
        /**
         * Construct a new RAMPortComponent with given child component, the write/read data/address pin groups, and the write/read clock/enable functions.
         * 
         * @param[in] component - Another component to be added as a child component.
         * @param[in] data_group - The name of the read or write data pin group.
         * @param[in] addr_group - The name of the read or write address pin group.
         * @param[in] clock_bf - The read or write clock's Boolean function.
         * @param[in] enable_bf - The read or write enable's Boolean function.
         * @param[in] is_write - True if the port is a write port, false otherwise.
         */
        RAMPortComponent(std::unique_ptr<GateTypeComponent> component,
                         const std::string& data_group,
                         const std::string& addr_group,
                         const BooleanFunction& clock_bf,
                         const BooleanFunction& enable_bf,
                         bool is_write);

        /**
         * Get the type of the gate type component.
         * 
         * @returns The type of the gate type component.
         */
        ComponentType get_type() const override;

        /**
         * Check whether a component is a RAMPortComponent.
         * 
         * @param[in] component - The component to check.
         * @returns True if component is a RAMPortComponent, false otherwise.
         */
        static bool is_class_of(const GateTypeComponent* component);

        /**
         * Get the sub-components of the gate type component.
         * A user-defined filter may be applied to the result vector, but is disabled by default.
         * 
         * @param[in] filter - The user-defined filter function applied to all candidate components.
         * @returns The sub-components of the gate type component.
         */
        std::vector<GateTypeComponent*> get_components(const std::function<bool(const GateTypeComponent*)>& filter = nullptr) const override;

        // TODO add tests for functions below

        /**
         * Get the name of the read or write data pin group.
         * 
         * @returns The name of the pin group.
         */
        const std::string& get_data_group() const;

        /**
         * Set the name of the read or write data pin group.
         * 
         * @param[in] data_group - The name of the pin group.
         */
        void set_data_group(const std::string& data_group);

        /**
         * Get the name of the read or write address pin group.
         * 
         * @returns The name of the pin group.
         */
        const std::string& get_address_group() const;

        /**
         * Set the name of the read or write address pin group.
         * 
         * @param[in] addr_group - The name of the pin group.
         */
        void set_address_group(const std::string& addr_group);

        /**
         * Get the Boolean function determining the read or write clock.
         * 
         * @returns The Boolean function.
         */
        const BooleanFunction& get_clock_function() const;

        /**
         * Set the Boolean function determining the read or write clock.
         * 
         * @param[in] clock_bf - The Boolean function.
         */
        void set_clock_function(const BooleanFunction& clock_bf);

        /**
         * Get the Boolean function determining the read or write enable.
         * 
         * @returns The Boolean function.
         */
        const BooleanFunction& get_enable_function() const;

        /**
         * Set the Boolean function determining the read or write enable.
         * 
         * @param[in] enable_bf - The Boolean function.
         */
        void set_enable_function(const BooleanFunction& enable_bf);

        /**
         * Check whether the port is a write or a read port.
         * 
         * @returns True if the port is a write port, false if it is a read port.
         */
        bool is_write_port() const;

        /**
         * Set the port to be a write or a read port.
         * 
         * @param[in] is_write - True to set the port to be a write port, false to set it to be a read port.
         */
        void set_write_port(bool is_write);

    private:
        static constexpr ComponentType m_type          = ComponentType::ram_port;
        std::unique_ptr<GateTypeComponent> m_component = nullptr;

        std::string m_data_group    = "";
        std::string m_addr_group    = "";
        BooleanFunction m_clock_bf  = BooleanFunction();
        BooleanFunction m_enable_bf = BooleanFunction();
        bool m_is_write;
    };
}    // namespace hal