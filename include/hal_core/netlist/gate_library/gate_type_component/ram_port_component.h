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
         * @param[in] write_data_group - The name of the write data pin group.
         * @param[in] read_data_group - The name of the read data pin group.
         * @param[in] write_addr_group - The name of the write address pin group.
         * @param[in] write_addr_group - The name of the read address pin group.
         * @param[in] write_clock_bf - The write clock Boolean function.
         * @param[in] read_clock_bf - The read clock Boolean function.
         * @param[in] write_enable_bf - The write enable Boolean function.
         * @param[in] read_enable_bf - The read enable Boolean function.
         */
        RAMPortComponent(std::unique_ptr<GateTypeComponent> component,
                         const std::string& write_data_group,
                         const std::string& read_data_group,
                         const std::string& write_addr_group,
                         const std::string& read_addr_group,
                         const BooleanFunction& write_clock_bf,
                         const BooleanFunction& read_clock_bf,
                         const BooleanFunction& write_enable_bf,
                         const BooleanFunction& read_enable_bf);

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
         * A user-defined filter may be applied to the result set, but is disabled by default.
         * 
         * @param[in] filter - The user-defined filter function applied to all candidate components.
         * @returns The sub-components of the gate type component.
         */
        std::set<GateTypeComponent*> get_components(const std::function<bool(const GateTypeComponent*)>& filter = nullptr) const override;

        // TODO pybinds and tests

        /**
         * Get the name of the pin group forming the write data input.
         * 
         * @returns The name of the pin group.
         */
        const std::string& get_write_data_group() const;

        /**
         * Set the name of the pin group forming the write data input.
         * 
         * @param[in] write_data_group - The name of the pin group.
         */
        void set_write_data_group(const std::string& write_data_group);

        /**
         * Get the name of the pin group forming the read data output.
         * 
         * @returns The name of the pin group.
         */
        const std::string& get_read_data_group() const;

        /**
         * Set the name of the pin group forming the read data output.
         * 
         * @param[in] read_data_group - The name of the pin group.
         */
        void set_read_data_group(const std::string& read_data_group);

        /**
         * Get the name of the pin group forming the write address.
         * 
         * @returns The name of the pin group.
         */
        const std::string& get_write_address_group() const;

        /**
         * Set the name of the pin group forming the write address.
         * 
         * @param[in] write_addr_group - The name of the pin group.
         */
        void set_write_address_group(const std::string& write_addr_group);

        /**
         * Get the name of the pin group forming the read address.
         * 
         * @returns The name of the pin group.
         */
        const std::string& get_read_address_group() const;

        /**
         * Set the name of the pin group forming the read address.
         * 
         * @param[in] read_addr_group - The name of the pin group.
         */
        void set_read_address_group(const std::string& read_addr_group);

        /**
         * Get the Boolean function determining the write clock.
         * 
         * @returns The Boolean function.
         */
        const BooleanFunction& get_write_clock_function() const;

        /**
         * Set the Boolean function determining the write clock.
         * 
         * @param[in] write_clock_bf - The Boolean function.
         */
        void set_write_clock_function(const BooleanFunction& write_clock_bf);

        /**
         * Get the Boolean function determining the read clock.
         * 
         * @returns The Boolean function.
         */
        const BooleanFunction& get_read_clock_function() const;

        /**
         * Set the Boolean function determining the read clock.
         * 
         * @param[in] read_clock_bf - The Boolean function.
         */
        void set_read_clock_function(const BooleanFunction& read_clock_bf);

        /**
         * Get the Boolean function determining the write enable.
         * 
         * @returns The Boolean function.
         */
        const BooleanFunction& get_write_enable_function() const;

        /**
         * Set the Boolean function determining the write enable.
         * 
         * @param[in] write_enable_bf - The Boolean function.
         */
        void set_write_enable_function(const BooleanFunction& write_enable_bf);

        /**
         * Get the Boolean function determining the read enable.
         * 
         * @returns The Boolean function.
         */
        const BooleanFunction& get_read_enable_function() const;

        /**
         * Set the Boolean function determining the read enable.
         * 
         * @param[in] read_enable_bf - The Boolean function.
         */
        void set_read_enable_function(const BooleanFunction& read_enable_bf);

    private:
        static constexpr ComponentType m_type          = ComponentType::ram_port;
        std::unique_ptr<GateTypeComponent> m_component = nullptr;

        std::string m_write_data_group    = "";
        std::string m_read_data_group     = "";
        std::string m_write_addr_group    = "";
        std::string m_read_addr_group     = "";
        BooleanFunction m_write_clock_bf  = BooleanFunction();
        BooleanFunction m_read_clock_bf   = BooleanFunction();
        BooleanFunction m_write_enable_bf = BooleanFunction();
        BooleanFunction m_read_enable_bf  = BooleanFunction();
    };
}    // namespace hal