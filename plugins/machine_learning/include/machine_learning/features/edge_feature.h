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
            /**
             * Abstract base class for per-edge feature extractors.
             *
             * An EdgeFeature maps an ordered (source, destination) endpoint pair — sharing a
             * Context — to a fixed-width vector of FEATURE_TYPE values. Edges here are signal
             * connections between pins, i.e. one source pin (output/inout) driving one
             * destination pin (input/inout) through a net.
             *
             * Subclasses must override:
             *   - calculate_feature(ctx, src, dst): the per-edge computation;
             *   - to_string(): a stable, parameter-encoded name;
             *   - get_legend(ctx): one label per element of the produced feature vector.
             */
            class EdgeFeature
            {
            public:
                /**
                 * Compute the feature vector for a single edge.
                 *
                 * @param[in] ctx - Shared ML context.
                 * @param[in] source - Source endpoint of the edge (driver).
                 * @param[in] destination - Destination endpoint of the edge (load).
                 * @returns The feature vector on success, an error otherwise. The returned vector
                 *          has length equal to get_legend(ctx).size().
                 */
                virtual Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Endpoint* source, const Endpoint* destination) const = 0;

                /**
                 * Stable, parameter-encoded name of the feature.
                 */
                virtual std::string to_string() const = 0;

                /**
                 * Concise per-index description of the feature vector produced by
                 * calculate_feature(ctx, ...).
                 *
                 * The returned vector's size equals the width of the feature vector. Labels are
                 * purely descriptive (no to_string() prefix); a caller that needs globally unique
                 * names across concatenated features should build that mapping itself.
                 *
                 * @param[in] ctx - Shared ML context.
                 * @returns One concise descriptor per index of the feature vector.
                 */
                virtual std::vector<std::string> get_legend(Context& ctx) const = 0;
            };

            /**
             * Pair of one-hot encodings of the source- and destination-pin types.
             *
             * Width = 2 * |PinType values present in the gate library|. The first half is the
             * one-hot encoding of the source pin's type; the second half is the one-hot
             * encoding of the destination pin's type. Index order within each half is
             * determined by Context::get_possible_pin_types() (sorted alphabetically).
             */
            class PinTypesOnehot : public EdgeFeature
            {
            public:
                PinTypesOnehot(){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Endpoint* source, const Endpoint* destination) const override;
                std::string to_string() const override;
                std::vector<std::string> get_legend(Context& ctx) const override;
            };

            /**
             * Pair of one-hot encodings of the source- and destination-pin directions.
             *
             * Width = 2 * |PinDirection values present in the gate library|. First half is the
             * source direction one-hot, second half is the destination direction one-hot.
             * Index order within each half is determined by
             * Context::get_possible_pin_directions() (sorted alphabetically).
             */
            class PinDirectionOnehot : public EdgeFeature
            {
            public:
                PinDirectionOnehot(){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Endpoint* source, const Endpoint* destination) const override;
                std::string to_string() const override;
                std::vector<std::string> get_legend(Context& ctx) const override;
            };

            /**
             * Integer indices of the source and destination gate types within the gate library
             * (see Context::get_gate_type_index). Suitable as embedding lookup keys.
             *
             * Legend (width 2): source_gate_type_index, destination_gate_type_index.
             */
            class GateTypeIndices : public EdgeFeature
            {
            public:
                GateTypeIndices(){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Endpoint* source, const Endpoint* destination) const override;
                std::string to_string() const override;
                std::vector<std::string> get_legend(Context& ctx) const override;
            };

            /**
             * Integer indices of the source and destination pins within their respective gate
             * types (see Context::get_gate_pin_index). Suitable as embedding lookup keys.
             *
             * Legend (width 2): source_pin_index, destination_pin_index.
             */
            class PinIndices : public EdgeFeature
            {
            public:
                PinIndices(){};

                Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Endpoint* source, const Endpoint* destination) const override;
                std::string to_string() const override;
                std::vector<std::string> get_legend(Context& ctx) const override;
            };

            Result<std::vector<FEATURE_TYPE>> build_feature_vec(Context& ctx, const std::vector<const EdgeFeature*>& features, const Endpoint* source, const Endpoint* destination);

        }    // namespace edge_feature
    }        // namespace machine_learning
}    // namespace hal
