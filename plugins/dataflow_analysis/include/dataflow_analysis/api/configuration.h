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
 * @file configuration.h
 * @brief This file contains the struct that holds all information on a dataflow analysis configuration.
 */

#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/gate_library/enums/gate_type_property.h"
#include "hal_core/netlist/gate_library/enums/pin_type.h"
#include "hal_core/netlist/pins/module_pin.h"
#include "hal_core/netlist/pins/pin_group.h"

#include <set>
#include <unordered_set>
#include <vector>

namespace hal
{
    class Gate;
    class Module;
    class Grouping;
    class Netlist;
    class Net;
    class GateType;

    namespace dataflow
    {
        /**
         * @struct Configuration
         * @brief Configuration of a dataflow analysis run.
         * 
         * This struct holds all information relevant for the configuration of a dataflow analysis run, including the netlist to analyze. 
         */
        struct Configuration
        {
            /**
             * @brief Construct a new dataflow analysis configuration for the given netlist.
             * 
             * @param[in] nl - The netlist.
             */
            Configuration(Netlist* nl);

            /**
             * @brief The netlist to be analyzed.
             */
            Netlist* netlist;

            /**
             * @brief Minimum size of a group. Smaller groups will be penalized during analysis. Defaults to 8.
             */
            u32 min_group_size = 8;

            /**
             * @brief Expected group sizes. Groups of these sizes will be prioritized. Defaults to an empty vector.
             */
            std::vector<u32> expected_sizes = {};

            /**
             * @brief Groups of gates that have already been identified as word-level groups beforehand. All gates of a group must be of one of the target gate types. Defaults to an empty vector.
             */
            std::vector<std::vector<Gate*>> known_gate_groups = {};

            /**
             * @brief Groups of nets that have been identified as word-level datapathes beforehand. Defaults to an empty vector.
             */
            std::vector<std::vector<Net*>> known_net_groups = {};

            /**
             * @brief The gate types to be grouped by dataflow analysis. Defaults to an empty set.
             */
            std::set<const GateType*> gate_types = {};

            /**
             * @brief The pin types of the pins to be considered control pins. Defaults to an empty set.
             */
            std::set<PinType> control_pin_types = {};

            /**
             * @brief Enable stage identification as part of dataflow analysis. Defaults to `false`.
             */
            bool enable_stages = false;

            /**
             * @brief Enforce gate type consistency inside of a group. Defaults to `false`.
             */
            bool enforce_type_consistency = false;

            /**
             * @brief Set the minimum size of a group. Smaller groups will be penalized during analysis.
             * 
             * @param[in] size - The minimum group size.
             * @returns The updated dataflow analysis configuration.
             */
            Configuration& with_min_group_size(u32 size);

            /**
             * @brief Set the expected group sizes. Groups of these sizes will be prioritized.
             * 
             * @param[in] sizes - The expected group sizes.
             * @returns The updated dataflow analysis configuration.
             */
            Configuration& with_expected_sizes(const std::vector<u32>& sizes);

            /**
             * @brief Add modules to the set of previously identified word-level structures.
             * 
             * The gates contained in the modules do not have to be of the target gate types.
             * The input and output pin groups of these modules will be used to guide datapath analysis.
             * Only pin groups larger than `min_group_size´ will be considered.
             * 
             * @param[in] structures - A vector of modules.
             * @param[in] overwrite - Set `true` to overwrite the existing known word-level structures, `false` otherwise. Defaults to `false`.
             * @returns The updated dataflow analysis configuration.
             */
            Configuration& with_known_structures(const std::vector<Module*>& structures, bool overwrite = false);

            /**
             * @brief Add modules to the set of previously identified word-level structures.
             * 
             * The gates contained in the modules do not have to be of the target gate types.
             * The input and output pin groups of these modules will be used to guide datapath analysis.
             * For each module, the input and output pin groups to be considered for analysis must be specified. 
             * An empty pin group vector results in all pin groups of the module being considered.
             * Only pin groups larger than `min_group_size´ will be considered.
             * 
             * @param[in] structures - A vector of modules, each of them with a vector of module pin groups.
             * @param[in] overwrite - Set `true` to overwrite the existing known word-level structures, `false` otherwise. Defaults to `false`.
             * @returns The updated dataflow analysis configuration.
             */
            Configuration& with_known_structures(const std::vector<std::pair<Module*, std::vector<PinGroup<ModulePin>*>>>& structures, bool overwrite = false);

            /**
             * @brief Add (typically large) gates to the set of previously identified word-level structures.
             * 
             * The gates do not have to be of the target gate types.
             * The input and output pin groups of these gates will be used to guide datapath analysis.
             * Only pin groups larger than `min_group_size´ will be considered.
             * 
             * @param[in] structures - A vector of gates.
             * @param[in] overwrite - Set `true` to overwrite the existing known word-level structures, `false` otherwise. Defaults to `false`.
             * @returns The updated dataflow analysis configuration.
             */
            Configuration& with_known_structures(const std::vector<Gate*>& structures, bool overwrite = false);

            /**
             * @brief Add (typically large) gates to the set of previously identified word-level structures.
             * 
             * The gates do not have to be of the target gate types.
             * The input and output pin groups of these gates will be used to guide datapath analysis.
             * For each gate, the input and output pin groups to be considered for analysis must be specified. 
             * An empty pin group vector results in all pin groups of the gate being considered.
             * Only pin groups larger than `min_group_size´ will be considered.
             *
             * @param[in] structures - A vector of gates, each of them with a vector of gate pin groups.
             * @param[in] overwrite - Set `true` to overwrite the existing known word-level structures, `false` otherwise. Defaults to `false`.
             * @returns The updated dataflow analysis configuration.
             */
            Configuration& with_known_structures(const std::vector<std::pair<Gate*, std::vector<PinGroup<GatePin>*>>>& structures, bool overwrite = false);

            /**
             * @brief Add all gates of a (typically large) gate type to the set of previously identified word-level structures.
             * 
             * The gate types do not have to be part of the target gate types.
             * The input and output pin groups of the gates of these types will be used to guide datapath analysis.
             * Only pin groups larger than `min_group_size´ will be considered.
             * 
             * @param[in] structures - A set of gate types.
             * @param[in] overwrite - Set `true` to overwrite the existing known word-level structures, `false` otherwise. Defaults to `false`.
             * @returns The updated dataflow analysis configuration.
             */
            Configuration& with_known_structures(const std::unordered_set<const GateType*>& structures, bool overwrite = false);

            /**
             * @brief Add all gates of a (typically large) gate type to the set of previously identified word-level structures.
             * 
             * The gate types do not have to be part of the target gate types.
             * The input and output pin groups of the gates of these types will be used to guide datapath analysis.
             * For each gate type, the input and output pin groups to be considered for analysis must be specified. 
             * An empty pin group vector results in all pin groups of the gate type being considered.
             * Only pin groups larger than `min_group_size´ will be considered.
             *
             * @param[in] structures - A map from gates to a vector of a subset of their pin groups.
             * @param[in] overwrite - Set `true` to overwrite the existing known word-level structures, `false` otherwise. Defaults to `false`.
             * @returns The updated dataflow analysis configuration.
             */
            Configuration& with_known_structures(const std::unordered_map<const GateType*, std::vector<PinGroup<GatePin>*>>& structures, bool overwrite = false);

            /**
             * @brief Add modules to the set of previously identified word-level groups.
             * 
             * These groups must only contain gates of the target gate types specified for analysis and will otherwise be ignored.
             * The groups will be used to guide dataflow analysis, but will remain unaltered in the process.
             * 
             * @param[in] groups - A vector of modules.
             * @param[in] overwrite - Set `true` to overwrite the existing previously identified word-level groups, `false` otherwise. Defaults to `false`.
             * @returns The updated dataflow analysis configuration.
             */
            Configuration& with_known_groups(const std::vector<Module*>& groups, bool overwrite = false);

            /**
             * @brief Add vectors of gates to the set of previously identified word-level groups.
             * 
             * These groups must only contain gates of the target gate types specified for analysis and will otherwise be ignored.
             * The groups will be used to guide dataflow analysis, but will remain unaltered in the process.
             *
             * @param[in] groups - A vector of groups, each of them as a vector of gates.
             * @param[in] overwrite - Set `true` to overwrite existing set of identified word-level structures, `false` otherwise. Defaults to `false`.
             * @returns The updated dataflow analysis configuration.
             */
            Configuration& with_known_groups(const std::vector<std::vector<Gate*>>& groups, bool overwrite = false);

            /**
             * @brief Add vectors of gate IDs to the set of previously identified word-level groups.
             * 
             * These groups must only contain gates of the target gate types specified for analysis and will otherwise be ignored.
             * The groups will be used to guide dataflow analysis, but will remain unaltered in the process.
             *
             * @param[in] groups - A vector of groups, each of them given as a vector of gate IDs.
             * @param[in] overwrite - Set `true` to overwrite existing set of identified word-level structures, `false` otherwise. Defaults to `false`.
             * @returns The updated dataflow analysis configuration.
             */
            Configuration& with_known_groups(const std::vector<std::vector<u32>>& groups, bool overwrite = false);

            /**
             * @brief Add groups from a previous dataflow analysis run to the set of previously identified word-level groups.
             * 
             * These groups must only contain gates of the target gate types specified for analysis and will otherwise be ignored.
             * The groups will be used to guide dataflow analysis, but will remain unaltered in the process.
             * The group IDs will be ignored during analysis and the same group may be assigned a new ID.
             *
             * @param[in] groups - A map from group IDs to groups, each of them given as a set of gates.
             * @param[in] overwrite - Set `true` to overwrite existing set of identified word-level structures, `false` otherwise. Defaults to `false`.
             * @returns The updated dataflow analysis configuration.
             */
            Configuration& with_known_groups(const std::unordered_map<u32, std::unordered_set<Gate*>>& groups, bool overwrite = false);

            /**
             * @brief Add the gate types to the set of gate types to be grouped by dataflow analysis.
             * 
             * Overwrite the existing set of gate types by setting the optional `overwrite` flag to `true`.
             * 
             * @param[in] types - A set of gate types.
             * @param[in] overwrite - Set `true` to overwrite existing set of gate types, `false` otherwise. Defaults to `false`.
             * @returns The updated dataflow analysis configuration.
             */
            Configuration& with_gate_types(const std::set<const GateType*>& types, bool overwrite = false);

            /**
             * @brief Add the gate types featuring the specified properties to the set of gate types to be grouped by dataflow analysis.
             * 
             * Overwrite the existing set of gate types by setting the optional `overwrite` flag to `true`.
             * 
             * @param[in] type_properties - A set of gate type properties.
             * @param[in] overwrite - Set `true` to overwrite existing set of gate types, `false` otherwise. Defaults to `false`.
             * @returns The updated dataflow analysis configuration.
             */
            Configuration& with_gate_types(const std::set<GateTypeProperty>& type_properties, bool overwrite = false);

            /**
             * @brief Set the pin types of the pins to be considered control pins by dataflow analysis.
             * 
             * Overwrite the existing set of pin types by setting the optional `overwrite` flag to `true`.
             * 
             * @param[in] types - A set of pin types.
             * @param[in] overwrite - Set `true` to overwrite existing set of pin types, `false` otherwise. Defaults to `false`.
             * @returns The updated dataflow analysis configuration.
             */
            Configuration& with_control_pin_types(const std::set<PinType>& types, bool overwrite = false);

            /**
             * @brief Enable stage identification as part of dataflow analysis.
             * 
             * @param[in] enable - Set `true` to enable stage identification, `false` otherwise. Defaults to `true`.
             * @returns The updated dataflow analysis configuration.
             */
            Configuration& with_stage_identification(bool enable = true);

            /**
             * @brief Enable type consistency as part of dataflow analysis when deciding whether two gates are allowed to merge into the same group.
             * 
             * @param[in] enable - Set `true` to enable type consistency inside of a group, `false` otherwise. Defaults to `true`.
             * @returns The updated dataflow analysis configuration.
             */
            Configuration& with_type_consistency(bool enable = true);
        };
    }    // namespace dataflow
}    // namespace hal