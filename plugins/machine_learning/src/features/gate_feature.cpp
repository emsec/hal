#include "hal_core/defines.h"
#include "hal_core/netlist/decorators/netlist_abstraction_decorator.h"
#include "hal_core/netlist/decorators/netlist_traversal_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/progress_printer.h"
#include "machine_learning/features/gate_feature.h"

#include <vector>

namespace hal
{
    namespace machine_learning
    {
        namespace gate_feature
        {
            Result<std::vector<std::vector<FEATURE_TYPE>>> build_feature_vecs(const std::vector<const GateFeatureBulk*>& features, const std::vector<Gate*>& gates)
            {
                if (gates.empty())
                {
                    return OK({});
                }

                Context ctx(gates.front()->get_netlist());
                return build_feature_vecs(features, gates);
            }

            Result<std::vector<std::vector<FEATURE_TYPE>>> build_feature_vecs(Context& ctx, const std::vector<const GateFeatureBulk*>& features, const std::vector<Gate*>& gates)
            {
                std::vector<std::vector<FEATURE_TYPE>> feature_vecs(gates.size(), std::vector<FEATURE_TYPE>());

                for (const auto* gf : features)
                {
                    const auto new_feature_vecs = gf->calculate_feature(ctx, gates);
                    if (new_feature_vecs.is_error())
                    {
                        return ERR_APPEND(new_feature_vecs.get_error(), "cannot build feature vectors: failed to build feature " + gf->to_string());
                    }

                    for (u32 idx = 0; idx < gates.size(); idx++)
                    {
                        auto& feature_vec           = feature_vecs.at(idx);
                        const auto& new_feature_vec = new_feature_vecs.get().at(idx);

                        feature_vec.insert(feature_vec.end(), new_feature_vec.begin(), new_feature_vec.end());
                    }
                }

                return OK(feature_vecs);
            }
        }    // namespace gate_feature
    }        // namespace machine_learning
}    // namespace hal