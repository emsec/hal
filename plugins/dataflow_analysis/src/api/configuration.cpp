#include "dataflow_analysis/api/configuration.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/netlist.h"

namespace hal
{
    namespace dataflow
    {
        Configuration::Configuration(Netlist* nl) : netlist(nl)
        {
        }

        Configuration& Configuration::with_min_group_size(u32 size)
        {
            this->min_group_size = size;
            return *this;
        }

        Configuration& Configuration::with_expected_sizes(const std::vector<u32>& sizes)
        {
            this->expected_sizes = sizes;
            return *this;
        }

        Configuration& Configuration::with_known_groups(const std::vector<const Module*>& groups, bool overwrite)
        {
            if (overwrite)
            {
                this->known_gate_groups.clear();
                this->known_module_groups.clear();
            }

            for (const auto* mod : groups)
            {
                this->known_module_groups.push_back(std::make_pair(mod, std::vector<const PinGroup<ModulePin>*>({})));
            }

            return *this;
        }

        Configuration& Configuration::with_known_groups(const std::vector<std::pair<const Module*, std::vector<const PinGroup<ModulePin>*>>>& groups, bool overwrite)
        {
            if (overwrite)
            {
                this->known_gate_groups.clear();
                this->known_module_groups = groups;
            }
            else
            {
                this->known_module_groups.insert(this->known_module_groups.end(), groups.begin(), groups.end());
            }

            return *this;
        }

        Configuration& Configuration::with_known_groups(const std::vector<std::vector<const Gate*>>& groups, bool overwrite)
        {
            if (overwrite)
            {
                this->known_gate_groups = groups;
                this->known_module_groups.clear();
            }
            else
            {
                this->known_gate_groups.insert(this->known_gate_groups.end(), groups.begin(), groups.end());
            }

            return *this;
        }

        Configuration& Configuration::with_known_groups(const std::vector<std::vector<u32>>& groups, bool overwrite)
        {
            if (overwrite)
            {
                this->known_gate_groups.clear();
                this->known_module_groups.clear();
            }

            for (const auto& gate_ids : groups)
            {
                std::vector<const Gate*> gates;
                std::transform(gate_ids.cbegin(), gate_ids.cend(), std::back_inserter(gates), [this](u32 gid) { return this->netlist->get_gate_by_id(gid); });
                this->known_gate_groups.push_back(gates);
            }

            return *this;
        }

        Configuration& Configuration::with_known_groups(const std::unordered_map<u32, std::unordered_set<Gate*>>& groups, bool overwrite)
        {
            if (overwrite)
            {
                this->known_gate_groups.clear();
                this->known_module_groups.clear();
            }

            for (const auto& [_, gates] : groups)
            {
                std::vector<const Gate*> group(gates.cbegin(), gates.cend());
                this->known_gate_groups.push_back(group);
            }

            return *this;
        }

        Configuration& Configuration::with_gate_types(const std::set<const GateType*>& types, bool overwrite)
        {
            if (overwrite)
            {
                this->gate_types = types;
            }
            else
            {
                this->gate_types.insert(types.begin(), types.end());
            }

            return *this;
        }

        Configuration& Configuration::with_gate_types(const std::set<GateTypeProperty>& gate_type_properties, bool overwrite)
        {
            auto gate_types_map = this->netlist->get_gate_library()->get_gate_types([gate_type_properties](const GateType* gt) {
                for (GateTypeProperty p : gate_type_properties)
                {
                    if (gt->has_property(p))
                    {
                        return true;
                    }
                }
                return false;
            });
            std::set<const GateType*> types;
            std::transform(gate_types_map.begin(), gate_types_map.end(), std::inserter(types, types.begin()), [](const auto& gt) { return gt.second; });
            this->with_gate_types(types, overwrite);

            return *this;
        }

        Configuration& Configuration::with_control_pin_types(const std::set<PinType>& types, bool overwrite)
        {
            if (overwrite)
            {
                this->control_pin_types = types;
            }
            else
            {
                this->control_pin_types.insert(types.begin(), types.end());
            }

            return *this;
        }

        Configuration& Configuration::with_stage_identification(bool enable)
        {
            this->enable_stages = enable;
            return *this;
        }

        Configuration& Configuration::with_type_consistency(bool enable)
        {
            this->enforce_type_consistency = enable;
            return *this;
        }
    }    // namespace dataflow
}    // namespace hal