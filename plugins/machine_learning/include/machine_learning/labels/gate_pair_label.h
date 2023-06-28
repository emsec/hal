#pragma once

#include "hal_core/defines.h"
#include "machine_learning/types.h"

#include <map>
#include <optional>
#include <vector>

namespace hal
{
    /* Forward declaration */
    class Gate;
    class Netlist;

    namespace machine_learning
    {
        namespace gate_pair_label
        {
            struct MultiBitInformation
            {
                std::map<const std::pair<const std::string, const std::string>, std::vector<const Gate*>> word_to_gates;
                std::map<const Gate*, std::vector<std::pair<const std::string, const std::string>>> gate_to_words;
            };

            struct LabelContext
            {
                LabelContext() = delete;
                LabelContext(const Netlist* netlist) : nl(netlist){};

                const Netlist* nl;
                std::optional<MultiBitInformation> mbi;
            };

            class GatePairLabel
            {
            public:
                // virtual std::vector<std::pair<const Gate*, const Gate*>> calculate_gate_pairs(LabelContext& lc, const Netlist* nl) const = 0;
                // virtual std::vector<u32> calculate_label(LabelContext& lc, const Gate* g_a, const Gate* g_b) const                       = 0;
                virtual std::pair<std::vector<std::pair<const Gate*, const Gate*>>, std::vector<std::vector<u32>>> calculate_labels(LabelContext& lc) const = 0;
            };

            class SharedSignalGroup : public GatePairLabel
            {
            public:
                SharedSignalGroup(){};

                // std::vector<std::pair<const Gate*, const Gate*>> calculate_gate_pairs(LabelContext& lc, const Netlist* nl) const override;
                // std::vector<u32> calculate_label(LabelContext& lc, const Gate* g_a, const Gate* g_b) const override;

                std::pair<std::vector<std::pair<const Gate*, const Gate*>>, std::vector<std::vector<u32>>> calculate_labels(LabelContext& lc) const override;
            };

            class SharedConnection : public GatePairLabel
            {
            public:
                SharedConnection(){};

                // std::vector<std::pair<const Gate*, const Gate*>> calculate_gate_pairs(LabelContext& lc, const Netlist* nl) const override;
                // std::vector<u32> calculate_label(LabelContext& lc, const Gate* g_a, const Gate* g_b) const override;

                std::pair<std::vector<std::pair<const Gate*, const Gate*>>, std::vector<std::vector<u32>>> calculate_labels(LabelContext& lc) const override;
            };

            std::pair<std::vector<std::pair<const Gate*, const Gate*>>, std::vector<std::vector<u32>>> test_build_labels(const Netlist* nl);
        }    // namespace gate_pair_label
    }        // namespace machine_learning
}    // namespace hal