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
            Result<std::vector<FEATURE_TYPE>> GateFeature::calculate_feature(Context& ctx, const Gate* gate) const
            {
                UNUSED(ctx);
                UNUSED(gate);

                return ERR("The calculation for a single gate has to be implemented by each feature class that does not implement the bulk method.");
            }

            Result<std::vector<std::vector<FEATURE_TYPE>>> GateFeature::calculate_feature(Context& ctx, const std::vector<Gate*>& gates) const
            {
                // Preallocate the feature vectors
                std::vector<std::vector<FEATURE_TYPE>> feature_vecs(gates.size());

                const u32 used_threads = std::min(u32(gates.size()), ctx.num_threads);

                std::vector<Result<std::monostate>> thread_results(used_threads, ERR("uninitialized"));

                // Worker function for each thread
                auto thread_func = [&](u32 start, u32 end, u32 thread_index) {
                    for (u32 i = start; i < end; ++i)
                    {
                        const auto feature_vec = (calculate_feature(ctx, gates.at(i)));
                        if (feature_vec.is_error())
                        {
                            thread_results.at(thread_index) = ERR(feature_vec.get_error().get());
                            return;
                        }

                        feature_vecs[i] = feature_vec.get();
                    }

                    thread_results.at(thread_index) = OK({});
                    return;
                };

                // Launch threads to process gates in parallel
                std::vector<std::thread> threads;
                u32 chunk_size = (gates.size() + used_threads - 1) / used_threads;
                for (u32 t = 0; t < used_threads; ++t)
                {
                    u32 start = t * chunk_size;
                    u32 end   = std::min(start + chunk_size, u32(gates.size()));
                    if (start < end)
                    {
                        threads.emplace_back(thread_func, start, end, t);
                    }
                }

                for (auto& thread : threads)
                {
                    thread.join();
                }

                // Check whether a thread encountered an error
                for (u32 idx = 0; idx < threads.size(); idx++)
                {
                    const auto& res = thread_results.at(idx);
                    if (res.is_error())
                    {
                        return ERR_APPEND(res.get_error(), "Encountered error when building feature vectors");
                    }
                }

                return OK(feature_vecs);
            }

            Result<std::vector<std::vector<FEATURE_TYPE>>> build_feature_vecs(const std::vector<const GateFeature*>& features, const std::vector<Gate*>& gates)
            {
                if (gates.empty())
                {
                    return OK({});
                }

                Context ctx(gates.front()->get_netlist());
                return build_feature_vecs(features, gates);
            }

            Result<std::vector<std::vector<FEATURE_TYPE>>> build_feature_vecs(Context& ctx, const std::vector<const GateFeature*>& features, const std::vector<Gate*>& gates)
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