#include "dataflow_analysis/api/configuration.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/netlist/module.h"

namespace hal
{
    namespace dataflow
    {
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