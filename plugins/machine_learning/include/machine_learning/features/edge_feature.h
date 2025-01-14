#pragma once

#include "hal_core/defines.h"
#include "machine_learning/types.h"

#include <optional>
#include <vector>

namespace hal
{
    /* Forward declaration */
    class Endpoint;

    namespace machine_learning
    {
        namespace edge_feature
        {
            class EdgeFeature
            {
            public:
                virtual Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Endpoint* source, const Endpoint* destination) const = 0;
                virtual std::string to_string() const                                                                                                = 0;
            };

            class PinTypesOnehot : public EdgeFeature
            {
            public:
                PinTypesOnehot(){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Endpoint* source, const Endpoint* destination) const override;
                std::string to_string() const override;
            };

            class PinDirectionOnehot : public EdgeFeature
            {
            public:
                PinDirectionOnehot(){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Endpoint* source, const Endpoint* destination) const override;
                std::string to_string() const override;
            };

            Result<std::vector<FEATURE_TYPE>> build_feature_vec(Context& ctx, const std::vector<const EdgeFeature*>& features, const Endpoint* source, const Endpoint* destination);

        }    // namespace edge_feature
    }        // namespace machine_learning
}    // namespace hal