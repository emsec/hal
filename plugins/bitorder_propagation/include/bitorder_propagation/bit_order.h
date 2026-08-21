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

/**
 * @file bit_order.h
 * @brief This file contains the bit order of a module pin group and the collection of bit orders that a propagation produces.
 */

#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/pins/pin_group.h"

#include <optional>
#include <vector>

namespace hal
{
    class Net;
    class Module;
    class ModulePin;

    namespace bitorder_propagation
    {
        /**
         * The index of a module pin group among the words that `export_bitorder_propagation_information` writes.
         *
         * It identifies a word within the exported file and has nothing to do with the index of a bit.
         */
        using WordIndex = u32;

        /**
         * The bit order of a single module pin group, i.e., which net of the pin group carries which bit.
         *
         * The index of a net is stored with it rather than implied by its position, so that an order with gaps in
         * it can be expressed. Propagation produces a continuous order today, but it is not guaranteed to keep
         * doing so, and the pin group a bit order belongs to need not start at index 0 either.
         */
        class BitOrder
        {
        public:
            /**
             * Construct a bit order for a module pin group.
             *
             * The order is sorted by index, so that iterating over it walks the bits from the lowest index to the
             * highest no matter in which order they were discovered.
             *
             * @param[in] module - The module the pin group belongs to.
             * @param[in] pin_group - The pin group.
             * @param[in] order - The index of each net of the pin group.
             */
            BitOrder(Module* module, PinGroup<ModulePin>* pin_group, std::vector<std::pair<Net*, u32>> order);

            /**
             * Get the module that the pin group belongs to.
             *
             * @returns The module.
             */
            Module* get_module() const;

            /**
             * Get the pin group whose bit order this is.
             *
             * @returns The pin group.
             */
            PinGroup<ModulePin>* get_pin_group() const;

            /**
             * Get the index of every net, ordered by index.
             *
             * @returns The nets and their indices.
             */
            const std::vector<std::pair<Net*, u32>>& get_order() const;

            /**
             * Get the index of the given net.
             *
             * @param[in] net - The net.
             * @returns The index of the net, or `std::nullopt` if the net is not part of this bit order.
             */
            std::optional<u32> get_index(const Net* net) const;

            /**
             * Get the net at the given index.
             *
             * @param[in] index - The index.
             * @returns The net at the index, or a `nullptr` if no net carries that index.
             */
            Net* get_net_at(u32 index) const;

            /**
             * Get the number of nets that the bit order covers.
             *
             * @returns The number of nets.
             */
            u32 get_size() const;

            /**
             * Check whether the indices run from 0 without leaving a gap.
             *
             * @returns `true` if the order is continuous, `false` otherwise.
             */
            bool is_continuous() const;

            bool operator==(const BitOrder& other) const;
            bool operator!=(const BitOrder& other) const;

        private:
            Module* m_module;
            PinGroup<ModulePin>* m_pin_group;
            std::vector<std::pair<Net*, u32>> m_order;
        };

        /**
         * The bit orders that are known, which is what a propagation reports: the ones it was given as well as
         * the ones it worked out.
         *
         * The bit orders are held in a stable order, by module ID and then by pin group ID, so that iterating over
         * a result does not depend on where the modules and pin groups happen to be allocated.
         */
        class BitOrderResult
        {
        public:
            BitOrderResult() = default;

            /**
             * Construct a result from the given bit orders.
             *
             * @param[in] bit_orders - The bit orders.
             */
            explicit BitOrderResult(std::vector<BitOrder> bit_orders);

            /**
             * Add a bit order, replacing one that is already known for the same pin group.
             *
             * @param[in] bit_order - The bit order.
             */
            void add(BitOrder bit_order);

            /**
             * Get every bit order, ordered by module ID and pin group ID.
             *
             * @returns The bit orders.
             */
            const std::vector<BitOrder>& get_bit_orders() const;

            /**
             * Get the bit order of the given pin group.
             *
             * @param[in] module - The module the pin group belongs to.
             * @param[in] pin_group - The pin group.
             * @returns The bit order, or a `nullptr` if the pin group has no known bit order.
             */
            const BitOrder* get(const Module* module, const PinGroup<ModulePin>* pin_group) const;

            /**
             * Check whether the bit order of the given pin group is known.
             *
             * @param[in] module - The module the pin group belongs to.
             * @param[in] pin_group - The pin group.
             * @returns `true` if the bit order is known, `false` otherwise.
             */
            bool contains(const Module* module, const PinGroup<ModulePin>* pin_group) const;

            /**
             * Get the number of pin groups whose bit order is known.
             *
             * @returns The number of bit orders.
             */
            u32 get_size() const;

            /**
             * Check whether no bit order at all is known.
             *
             * @returns `true` if there is no bit order, `false` otherwise.
             */
            bool is_empty() const;

            std::vector<BitOrder>::const_iterator begin() const;
            std::vector<BitOrder>::const_iterator end() const;

        private:
            std::vector<BitOrder> m_bit_orders;
        };
    }    // namespace bitorder_propagation
}    // namespace hal
