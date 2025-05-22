// #pragma once

// #include "machine_learning/features/gate_feature.h"

// namespace hal
// {
//     namespace machine_learning
//     {
//         namespace gate_feature
//         {
//             class ConnectedGlobalIOs : public GateFeatureSingle
//             {
//             public:
//                 ConnectedGlobalIOs(){};

//                 Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g) const override;
//                 std::string to_string() const override;
//             };

//             class DistanceGlobalIO : public GateFeatureSingle
//             {
//             public:
//                 DistanceGlobalIO(const PinDirection& direction, const bool directed = true, const std::vector<PinType>& forbidden_pin_types = {})
//                     : m_direction(direction), m_directed(directed), m_forbidden_pin_types(forbidden_pin_types){};

//                 Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g) const override;
//                 std::string to_string() const override;

//             private:
//                 const PinDirection m_direction;
//                 const bool m_directed;
//                 const std::vector<PinType> m_forbidden_pin_types;
//             };

//             class SequentialDistanceGlobalIO : public GateFeatureSingle
//             {
//             public:
//                 SequentialDistanceGlobalIO(const PinDirection& direction, const bool directed = true, const std::vector<PinType>& forbidden_pin_types = {})
//                     : m_direction(direction), m_directed(directed), m_forbidden_pin_types(forbidden_pin_types){};

//                 Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g) const override;
//                 std::string to_string() const override;

//             private:
//                 const PinDirection m_direction;
//                 const bool m_directed;
//                 const std::vector<PinType> m_forbidden_pin_types;
//             };

//             class IODegrees : public GateFeatureSingle
//             {
//             public:
//                 IODegrees(){};

//                 Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g) const override;
//                 std::string to_string() const override;
//             };

//             class GateTypeOneHot : public GateFeatureSingle
//             {
//             public:
//                 GateTypeOneHot(){};

//                 Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g) const override;
//                 std::string to_string() const override;
//             };

//             class NeighboringGateTypes : public GateFeatureSingle
//             {
//             public:
//                 NeighboringGateTypes(const u32 depth, const PinDirection& direction, const bool directed = true) : m_depth(depth), m_direction(direction), m_directed(directed){};

//                 Result<std::vector<FEATURE_TYPE>> calculate_feature(Context& ctx, const Gate* g) const override;
//                 std::string to_string() const override;

//             private:
//                 const u32 m_depth;
//                 const PinDirection m_direction;
//                 const bool m_directed;
//             };
//         }    // namespace gate_feature
//     }        // namespace machine_learning
// }    // namespace hal