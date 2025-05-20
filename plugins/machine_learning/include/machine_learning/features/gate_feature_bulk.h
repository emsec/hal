// #pragma once

// #include "machine_learning/features/gate_feature.h"

// namespace hal
// {
//     namespace machine_learning
//     {
//         namespace gate_feature
//         {
//             class BetweennessCentrality : public GateFeatureBulk
//             {
//             public:
//                 BetweennessCentrality(const bool directed = true, const i32 cutoff = -1, const bool normalize = true) : m_directed(directed), m_cutoff(cutoff), m_normalize(normalize){};

//                 Result<std::vector<std::vector<FEATURE_TYPE>>> calculate_feature(Context& ctx, const std::vector<Gate*>& gates) const override;
//                 std::string to_string() const override;

//             private:
//                 const bool m_directed;
//                 const i32 m_cutoff;
//                 const bool m_normalize;
//             };

//             class HarmonicCentrality : public GateFeatureBulk
//             {
//             public:
//                 HarmonicCentrality(const PinDirection& direction, const i32 cutoff = -1, const bool normalize = true) : m_direction(direction), m_cutoff(cutoff), m_normalize(normalize){};

//                 Result<std::vector<std::vector<FEATURE_TYPE>>> calculate_feature(Context& ctx, const std::vector<Gate*>& gates) const override;
//                 std::string to_string() const override;

//             private:
//                 const PinDirection m_direction;
//                 const i32 m_cutoff;
//                 const bool m_normalize;
//             };

//             class SequentialBetweennessCentrality : public GateFeatureBulk
//             {
//             public:
//                 SequentialBetweennessCentrality(const bool directed = true, const i32 cutoff = -1, const bool normalize = true) : m_directed(directed), m_cutoff(cutoff), m_normalize(normalize){};

//                 Result<std::vector<std::vector<FEATURE_TYPE>>> calculate_feature(Context& ctx, const std::vector<Gate*>& gates) const override;
//                 std::string to_string() const override;

//             private:
//                 const bool m_directed;
//                 const i32 m_cutoff;
//                 const bool m_normalize;
//             };

//             class SequentialHarmonicCentrality : public GateFeatureBulk
//             {
//             public:
//                 SequentialHarmonicCentrality(const PinDirection& direction, const i32 cutoff = -1, const bool normalize = true) : m_direction(direction), m_cutoff(cutoff), m_normalize(normalize){};

//                 Result<std::vector<std::vector<FEATURE_TYPE>>> calculate_feature(Context& ctx, const std::vector<Gate*>& gates) const override;
//                 std::string to_string() const override;

//             private:
//                 const PinDirection m_direction;
//                 const i32 m_cutoff;
//                 const bool m_normalize;
//             };
//         }    // namespace gate_feature
//     }    // namespace machine_learning
// }    // namespace hal