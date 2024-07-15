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
 * @file bitorder_propagation.h 
 * @brief This file contains functions for bit-order propagation from pin groups of known bit order to pin groups of unknown bit order.
 */

#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/pins/pin_group.h"
#include "hal_core/utilities/result.h"

#include <map>

namespace hal
{
    class Net;
    class Netlist;
    class Module;
    class ModulePin;

    namespace bitorder_propagation
    {
        /** 
         * @brief Propagate known bit-order information from the given module pin groups to module pin groups of unknown bit order.
         * 
         * The known bit-order information is taken from the map from net to index given for each pair of module and pin group in `src`.
         * After propagation, the algorithm tries to reconstruct valid bit orders from the propagated information.
         *
         * @param[in] src - The known indices for the nets belonging to the given module pin groups. 
         * @param[in] dst - The pairs of module ID and pin group name with unknown bit order.
         * @param[in] enforce_continuous_bitorders - Set `true` to only allow for continuous bit orders, `false` to also allow bit orders that are not continuous. Defaults to `true`.
         * @returns OK and a map containing all known bit orders (including new and already known ones) on success, an error otherwise.
         */
        Result<std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>>>
            propagate_module_pingroup_bitorder(const std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>>& src,
                                               const std::set<std::pair<Module*, PinGroup<ModulePin>*>>& dst,
                                               const bool enforce_continuous_bitorders = true);

        /**
         * @brief Reorder and rename the pins of the pin groups according to the provided bit-order information. 
         *
         * @param[in] ordered_module_pin_groups - A mapping from pairs of modules and their pin groups to known bit-order information given as a mapping from nets to their index.
         * @returns OK on success, an error otherwise.
         */
        Result<std::monostate> reorder_module_pin_groups(const std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>>& ordered_module_pin_groups);

        /**
         * @brief Propagate known bit-order information from one module pin group to another module pin group of unknown bit order.
         * 
         * The known bit-order information is taken from the order of pins in the pin group of `src`.
         * After propagation, the algorithm tries to reconstruct a valid bit order from the propagated information.
         * The valid bit order is then annotated to the module pin group, i.e., the pins of the respective pin group are renamed and reordered.
         *
         * @param[in] nl  - The netlist containing the module.
         * @param[in] src - The pair of module ID and pin group name with known bit order. 
         * @param[in] dst - The pair of module ID and pin group name with unknown bit order.
         * @returns OK and a map containing all known bit orders (including new and already known ones) on success, an error otherwise.
         */
        Result<std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>>> propagate_bitorder(Netlist* nl, const std::pair<u32, std::string>& src, const std::pair<u32, std::string>& dst);

        /**
         * @brief Propagate known bit-order information from one module pin group to another module pin group of unknown bit order.
         * 
         * The known bit-order information is taken from the order of pins in the pin group of `src`.
         * After propagation, the algorithm tries to reconstruct a valid bit order from the propagated information.
         * The valid bit order is then annotated to the module pin group, i.e., the pins of the respective pin group are renamed and reordered.
         *
         * @param[in] src - The pair of module and pin group with known bit order. 
         * @param[in] dst - The pair of module and pin group with unknown bit order.
         * @returns OK and a map containing all known bit orders (including new and already known ones) on success, an error otherwise.
         */
        Result<std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>>> propagate_bitorder(const std::pair<Module*, PinGroup<ModulePin>*>& src,
                                                                                                           const std::pair<Module*, PinGroup<ModulePin>*>& dst);

        /**
         * @brief Propagate known bit-order information from the given module pin groups to module pin groups of unknown bit order.
         * 
         * The known bit-order information is taken from the order of pins in the pin groups of `src`.
         * After propagation, the algorithm tries to reconstruct valid bit orders from the propagated information.
         * The valid bit orders are then annotated to the module pin groups, i.e., the pins of the respective pin groups are renamed and reordered.
         *
         * @param[in] nl  - The netlist containing the modules.
         * @param[in] src - The pairs of module ID and pin group name with known bit order.
         * @param[in] dst - The pairs of module ID and pin group name with unknown bit order.
         * @returns OK and a map containing all known bit orders (including new and already known ones) on success, an error otherwise.
         */
        Result<std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>>>
            propagate_bitorder(Netlist* nl, const std::vector<std::pair<u32, std::string>>& src, const std::vector<std::pair<u32, std::string>>& dst);

        /**
         * @brief Propagate known bit-order information from the given module pin groups to module pin groups of unknown bit order.
         * 
         * The known bit-order information is taken from the order of pins in the pin groups of `src`.
         * After propagation, the algorithm tries to reconstruct valid bit orders from the propagated information.
         * The valid bit orders are then annotated to the module pin groups, i.e., the pins of the respective pin groups are renamed and reordered.
         *
         * @param[in] src - The pairs of module and pin group with known bit order.
         * @param[in] dst - The pairs of module and pin group with unknown bit order.
         * @returns OK and a map containing all known bit orders (including new and already known ones) on success, an error otherwise.
         */
        Result<std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>>> propagate_bitorder(const std::vector<std::pair<Module*, PinGroup<ModulePin>*>>& src,
                                                                                                           const std::vector<std::pair<Module*, PinGroup<ModulePin>*>>& dst);
    }    // namespace bitorder_propagation
}    // namespace hal