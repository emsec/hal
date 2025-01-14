
#include "hal_core/netlist/gate.h"
#include "hal_core/utilities/log.h"
#include "machine_learning/features/edge_feature.h"

// #define PROGRESS_BAR

namespace hal
{
    namespace machine_learning
    {
        namespace edge_feature
        {
            Result<std::vector<FEATURE_TYPE>> PinTypesOnehot::calculate_feature(Context& ctx, const Endpoint* src, const Endpoint* dst) const
            {
                const auto& all_pin_types = ctx.get_possible_pin_types();

                // TODO remove debug print
                // std::cout << "Got following pin types: " << std::endl;
                // for (const auto& pt : all_pin_types)
                // {
                //     std::cout << enum_to_string(pt) << std::endl;
                // }

                std::vector<FEATURE_TYPE> feature = std::vector<FEATURE_TYPE>(all_pin_types.size() * 2, FEATURE_TYPE(0));

                const u32 index_src = std::distance(all_pin_types.begin(), std::find(all_pin_types.begin(), all_pin_types.end(), src->get_pin()->get_type()));
                const u32 index_dst = std::distance(all_pin_types.begin(), std::find(all_pin_types.begin(), all_pin_types.end(), dst->get_pin()->get_type()));

                feature.at(index_src) += FEATURE_TYPE(1);
                feature.at(all_pin_types.size() + index_dst) += FEATURE_TYPE(1);

                return OK(feature);
            };

            std::string PinTypesOnehot::to_string() const
            {
                return "PinTypesOnehot";
            }

            Result<std::vector<FEATURE_TYPE>> PinDirectionOnehot::calculate_feature(Context& ctx, const Endpoint* src, const Endpoint* dst) const
            {
                const auto& all_pin_directions = ctx.get_possible_pin_directions();

                // TODO remove debug print
                // std::cout << "Got following pin types: " << std::endl;
                // for (const auto& pt : all_pin_directions)
                // {
                //     std::cout << enum_to_string(pt) << std::endl;
                // }

                std::vector<FEATURE_TYPE> feature = std::vector<FEATURE_TYPE>(all_pin_directions.size() * 2, FEATURE_TYPE(0));

                const u32 index_src = std::distance(all_pin_directions.begin(), std::find(all_pin_directions.begin(), all_pin_directions.end(), src->get_pin()->get_direction()));
                const u32 index_dst = std::distance(all_pin_directions.begin(), std::find(all_pin_directions.begin(), all_pin_directions.end(), dst->get_pin()->get_direction()));

                feature.at(index_src) += FEATURE_TYPE(1);
                feature.at(all_pin_directions.size() + index_dst) += FEATURE_TYPE(1);

                return OK(feature);
            };

            std::string PinDirectionOnehot::to_string() const
            {
                return "PinDirectionOnehot";
            }

            Result<std::vector<FEATURE_TYPE>> build_feature_vec(Context& ctx, const std::vector<const EdgeFeature*>& features, const Endpoint* source, const Endpoint* destination)
            {
                std::vector<FEATURE_TYPE> feature_vec;

                for (const auto& ef : features)
                {
                    const auto new_features = ef->calculate_feature(ctx, source, destination);
                    if (new_features.is_error())
                    {
                        return ERR_APPEND(new_features.get_error(), "cannot build feature vector");
                    }

                    feature_vec.insert(feature_vec.end(), new_features.get().begin(), new_features.get().end());
                }

                return OK(feature_vec);
            }

        }    // namespace edge_feature
    }        // namespace machine_learning
}    // namespace hal