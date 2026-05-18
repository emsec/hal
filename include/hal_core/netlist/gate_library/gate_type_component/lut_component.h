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
#include "hal_core/netlist/pins/gate_pin.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace hal
{
    class LUTComponent : public GateTypeComponent
    {
    public:
        /**
         * Maps an output pin to a specific slice of an INIT string.
         */
        struct LUTOutputConfig
        {
            /**
             * @param[in] init_identifier - The data identifier within the INIT category.
             * @param[in] bit_offset      - First bit (LSB = 0) of the slice within the parsed INIT value.
             * @param[in] bit_count       - Number of bits; must be a non-zero power of two.
             * @param[in] is_ascending    - True if LSB is stored first (ascending), false otherwise.
             * @param[in] input_pins      - Ordered input pin names; must be non-empty (size must equal log2(bit_count)).
             */
            LUTOutputConfig(std::string init_identifier,
                            u32 bit_offset,
                            u32 bit_count,
                            bool is_ascending,
                            std::vector<std::string> input_pins)
                : init_identifier(std::move(init_identifier)),
                  bit_offset(bit_offset),
                  bit_count(bit_count),
                  is_ascending(is_ascending),
                  input_pins(std::move(input_pins))
            {
            }

            std::string init_identifier;
            u32 bit_offset               = 0;
            u32 bit_count                = 0;
            bool is_ascending            = true;
            std::vector<std::string> input_pins;
        };

        /**
         * Create a new LUTComponent with the given per-output-pin configurations.
         * Configurations with a bit_count of zero, a non-power-of-two bit_count, or empty input_pins are silently skipped.
         *
         * @param[in] configs - Map from output pin name to LUTOutputConfig.
         * @returns The new LUTComponent.
         */
        static std::unique_ptr<LUTComponent> create(std::unordered_map<std::string, LUTOutputConfig> configs = {});

        /**
         * Get the type of the gate type component.
         *
         * @returns The type of the gate type component.
         */
        ComponentType get_type() const override;

        /**
         * Check whether a component is a LUTComponent.
         *
         * @param[in] component - The component to check.
         * @returns True if component is a LUTComponent, false otherwise.
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
         * Get the output configuration for a specific pin, or nullptr if none is set.
         *
         * @param[in] pin_name - Name of the LUT output pin.
         * @returns Pointer to the LUTOutputConfig, or nullptr.
         */
        const LUTOutputConfig* get_output_pin_config(const std::string& pin_name) const;

        /**
         * Get all per-output-pin configurations.
         *
         * @returns Map from pin name to LUTOutputConfig.
         */
        const std::unordered_map<std::string, LUTOutputConfig>& get_output_pin_configs() const;

        /**
         * Extract a bit slice from a full INIT hex string.
         * Returns the slice as an uppercase hex string padded to `(bit_count+3)/4` characters.
         *
         * @param[in] full_hex   - Full INIT value as a hex string.
         * @param[in] bit_offset - First bit (LSB = 0) of the slice.
         * @param[in] bit_count  - Number of bits in the slice; must be non-zero.
         * @returns OK with the extracted hex string on success, an error otherwise.
         */
        static Result<std::string> extract_init_slice(const std::string& full_hex, u32 bit_offset, u32 bit_count);

        /**
         * Splice a new slice value into a full INIT hex string at `[bit_offset, bit_offset+bit_count)`.
         * The output preserves the digit width of `full_hex`; if `full_hex` is empty the minimum width
         * needed to hold the splice is used.
         *
         * @param[in] full_hex   - Current full INIT value as a hex string (may be empty).
         * @param[in] slice_hex  - New value for the bit slice.
         * @param[in] bit_offset - First bit (LSB = 0) of the slice.
         * @param[in] bit_count  - Number of bits in the slice; must be non-zero.
         * @returns OK with the updated full hex string on success, an error on parse failure.
         */
        static Result<std::string> splice_init_slice(const std::string& full_hex, const std::string& slice_hex, u32 bit_offset, u32 bit_count);

    private:
        explicit LUTComponent(std::unordered_map<std::string, LUTOutputConfig> configs);

        static constexpr ComponentType m_type = ComponentType::lut;
        std::unordered_map<std::string, LUTOutputConfig> m_output_pin_configs;
    };
}    // namespace hal
