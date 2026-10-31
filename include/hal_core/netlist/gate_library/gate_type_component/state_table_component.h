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

#include "hal_core/netlist/gate_library/gate_type_component/gate_type_component.h"
#include "hal_core/utilities/result.h"

#include <memory>
#include <string>
#include <vector>

namespace hal
{
    class GatePin;

    /**
     * A component describing the behavior of a sequential gate type via a Liberty statetable.
     * Acts as a subcomponent of FFComponent or LatchComponent.
     */
    class StateTableComponent : public GateTypeComponent
    {
    public:
        /**
         * Symbol for an input column or current-state column entry in a statetable row.
         * L/H and H/L are expanded into two rows at parse time and never appear in stored rows.
         */
        enum class TableInputSymbol
        {
            LOW,        /**< Logic low (L). */
            HIGH,       /**< Logic high (H). */
            DONT_CARE,  /**< Don't care (-). */
            RISING,     /**< Rising edge (R). */
            FALLING,    /**< Falling edge (F). */
            NOT_RISING, /**< Not rising edge (~R). */
            NOT_FALLING /**< Not falling edge (~F). */
        };

        /**
         * Symbol for a next-state column entry in a statetable row.
         * L/H and H/L are expanded into two rows at parse time and never appear in stored rows.
         */
        enum class TableOutputSymbol
        {
            LOW,         /**< Force low (L). */
            HIGH,        /**< Force high (H). */
            UNSPECIFIED, /**< Output not specified (-). */
            UNKNOWN,     /**< Unknown (X). */
            HOLD         /**< Hold current value (N). */
        };

        /**
         * One row of a statetable after L/H and H/L expansion.
         * input_values is indexed by the input_pins of the enclosing StateTable.
         * current_state_values is indexed by the current_state_pins of the enclosing StateTable.
         * next_state_value is the resolved next-state output for the pin this StateTable belongs to.
         */
        struct TableRow
        {
            std::vector<TableInputSymbol> input_values;
            std::vector<TableInputSymbol> current_state_values;
            TableOutputSymbol next_state_value;
        };

        /**
         * A fully resolved, per-pin statetable.
         * All column references are real HAL pin names; no abstract node names remain.
         * Rows are stored in priority order (first match wins).
         */
        struct StateTable
        {
            std::string pin_name;                        /**< The output or internal pin this table drives. */
            std::vector<std::string> input_pins;         /**< Real input port names in column order. */
            std::vector<std::string> current_state_pins; /**< Pin names whose current values index current_state_values. */
            std::vector<TableRow> rows;                  /**< Priority-ordered expanded rows. */
        };

        /**
         * Create a new StateTableComponent.
         *
         * @param[in] component - Child component to take ownership of (typically nullptr or an InitComponent).
         * @param[in] tables - Per-pin state tables, fully resolved to pin names.
         * @returns The StateTableComponent.
         */
        static std::unique_ptr<StateTableComponent> create(std::unique_ptr<GateTypeComponent> component, std::vector<StateTable> tables);

        /**
         * Get the type of the gate type component.
         *
         * @returns The type of the gate type component.
         */
        ComponentType get_type() const override;

        /**
         * Check whether a component is a StateTableComponent.
         *
         * @param[in] component - The component to check.
         * @returns True if component is a StateTableComponent, false otherwise.
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

        /**
         * Get all per-pin state tables.
         *
         * @returns The state tables.
         */
        const std::vector<StateTable>& get_state_tables() const;

        /**
         * Get the state table for a specific pin.
         *
         * @param[in] pin_name - The name of the pin.
         * @returns The StateTable for that pin on success, an error otherwise.
         */
        Result<const StateTable*> get_state_table_for_pin(const std::string& pin_name) const;

        /**
         * Get the state table for a specific pin.
         *
         * @param[in] pin - The pin.
         * @returns The StateTable for that pin on success, an error otherwise.
         */
        Result<const StateTable*> get_state_table_for_pin(const GatePin* pin) const;

    private:
        StateTableComponent(std::unique_ptr<GateTypeComponent> component, std::vector<StateTable> tables);

        static constexpr ComponentType m_type          = ComponentType::state_table;
        std::unique_ptr<GateTypeComponent> m_component = nullptr;
        std::vector<StateTable> m_tables;
    };
}    // namespace hal
