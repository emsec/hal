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

        Configuration& Configuration::with_known_groups(const std::vector<Module*>& groups)
        {
            for (const auto* mod : groups)
            {
                std::vector<u32> group;
                const auto& gates = mod->get_gates();
                std::transform(gates.cbegin(), gates.cend(), std::back_inserter(group), [](const Gate* g) { return g->get_id(); });
                this->known_groups.push_back(std::move(group));
            }
            return *this;
        }

        Configuration& Configuration::with_known_groups(const std::vector<hal::Grouping*>& groups)
        {
            for (const auto* grp : groups)
            {
                std::vector<u32> group;
                const auto& gates = grp->get_gates();
                std::transform(gates.cbegin(), gates.cend(), std::back_inserter(group), [](const Gate* g) { return g->get_id(); });
                this->known_groups.push_back(std::move(group));
            }
            return *this;
        }

        Configuration& Configuration::with_known_groups(const std::vector<std::vector<Gate*>>& groups)
        {
            for (const auto& gates : groups)
            {
                std::vector<u32> group;
                std::transform(gates.cbegin(), gates.cend(), std::back_inserter(group), [](const Gate* g) { return g->get_id(); });
                this->known_groups.push_back(std::move(group));
            }
            return *this;
        }

        Configuration& Configuration::with_known_groups(const std::vector<std::vector<u32>>& groups)
        {
            this->known_groups = groups;
            return *this;
        }

        Configuration& Configuration::with_known_groups(const std::unordered_map<u32, std::unordered_set<Gate*>>& groups)
        {
            for (const auto& [_, gates] : groups)
            {
                std::vector<u32> group;
                std::transform(gates.cbegin(), gates.cend(), std::back_inserter(group), [](const Gate* g) { return g->get_id(); });
                this->known_groups.push_back(group);
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

        Configuration& Configuration::with_register_stage_identification(bool enable)
        {
            this->enable_register_stages = enable;
            return *this;
        }

        Configuration& Configuration::with_type_consistency(bool enable)
        {
            this->enforce_type_consistency = enable;
            return *this;
        }
    }    // namespace dataflow
}    // namespace hal