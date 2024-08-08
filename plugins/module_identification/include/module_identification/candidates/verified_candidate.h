#pragma once

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/utilities/result.h"

#include <map>
#include <string>
#include <vector>

namespace hal
{
    class Net;
    class Gate;

    namespace module_identification
    {
        enum class CandidateType;

        /**
         * @struct WordLevelOperation
         * @brief Represents a word-level operation with its operands, control signals, and the operation implemented as a HAL Boolean function.
         * 
         * This struct is used to store the information related to a word-level operation, which includes the operands, control signals, and the Boolean function representing the operation.
         */
        struct WordLevelOperation
        {
            /**
             * @brief A map of operand names to their corresponding Boolean functions.
             */
            std::map<std::string, BooleanFunction> operands;

            /**
             * @brief A vector of control signals as Boolean functions.
             */
            std::vector<BooleanFunction> ctrl_signals;

            /**
             * @brief The Boolean function representing the word-level operation.
             */
            BooleanFunction operation;
        };

        /**
         * @class VerifiedCandidate
         * @brief Represents a verified candidate for module identification.
         * 
         * This class is used to represent a verified candidate within the module identification process, providing methods for their creation, manipulation, and verification.
         */
        class VerifiedCandidate
        {
        public:
            /**
             * @brief Constructor for `VerifiedCandidate`.
             * 
             * @param[in] operands - A vector of vectors of operand nets.
             * @param[in] output_nets - A vector of output nets.
             * @param[in] control_signals - A vector of control signal nets.
             * @param[in] control_signal_mappings - A vector of mappings from nets to their Boolean values.
             * @param[in] word_level_operations - A map of control signal mappings to their respective word-level operations.
             * @param[in] gates - A vector of gates associated with the candidate.
             * @param[in] base_gates - A vector of base gates associated with the candidate.
             * @param[in] additional_data - A map of additional data.
             * @param[in] types - A set of candidate types.
             */
            VerifiedCandidate(const std::vector<std::vector<Net*>>& operands,
                              const std::vector<Net*>& output_nets,
                              const std::vector<Net*>& control_signals,
                              const std::vector<std::map<Net*, BooleanFunction::Value>>& control_signal_mappings,
                              const std::map<std::map<Net*, BooleanFunction::Value>, BooleanFunction>& word_level_operations,
                              const std::vector<Gate*>& gates,
                              const std::vector<Gate*>& base_gates,
                              const std::map<std::string, std::string>& additional_data,
                              const std::set<CandidateType>& types);

            /**
             * @brief Default constructor for an empty/unverified `VerifiedCandidate`.
             */
            VerifiedCandidate();

            /**
             * @brief Equality comparison operator for two verified candidates.
             * 
             * Compares two verified candidates for equality.
             * 
             * @param[in] other - The verified candidate to compare against.
             * @returns `true` if the candidates are equal, `false` otherwise.
             */
            bool operator==(const VerifiedCandidate& other) const
            {
                return (m_operands == other.m_operands) && (m_control_signals == other.m_control_signals) && (m_control_signal_mappings == other.m_control_signal_mappings)
                       && (m_output_nets == other.m_output_nets) && (m_gates == other.m_gates) && (m_types == other.m_types) && (m_verified == other.m_verified);
            }

            /**
             * @brief Merge multiple verified candidates into a single candidate.
             * 
             * This function merges a vector of verified candidates into a single candidate.
             * 
             * @param[in] candidates - A vector of verified candidates to merge.
             * @returns OK() and the merged verified candidate on success, an error otherwise.
             */
            static hal::Result<VerifiedCandidate> merge(const std::vector<VerifiedCandidate>& candidates);

            /**
             * @brief Get the candidate information as a string.
             * 
             * @returns A string containing the candidate information.
             */
            std::string get_candidate_info() const;

            /**
             * @brief Check if the candidate is verified.
             * 
             * @returns `true` if the candidate is verified, `false` otherwise.
             */
            bool is_verified() const;

            /**
             * @brief Get the name of the candidate that represents the functionality of the candidate.
             * 
             * @returns A string containing the name of the candidate.
             */
            std::string get_name() const;

            /**
             * @brief Get the merged word-level operation for the candidate.
             * 
             * The mergred word-level operation includes all word-level oprations that we were able to verify for different control mappings of the candidate.
             * 
             * @returns The merged word-level operation as a Boolean function.
             */
            BooleanFunction get_merged_word_level_operation() const;

            /**
             * A vector of operands.
             */
            std::vector<std::vector<Net*>> m_operands;

            /**
             * A vector of output nets.
             */
            std::vector<Net*> m_output_nets;

            /**
             * A vector of control signal nets.
             */
            std::vector<Net*> m_control_signals;

            /**
             * A vector of all control mappings covered by this candidate.
             */
            std::vector<std::map<Net*, BooleanFunction::Value>> m_control_signal_mappings;

            /**
             * A map of control signal mappings to their respective word-level operations.
             */
            std::map<std::map<Net*, BooleanFunction::Value>, BooleanFunction> m_word_level_operations;

            /**
             * A vector of gates associated with the candidate.
             */
            std::vector<Gate*> m_gates;

            /**
             * A vector of base gates associated with the candidate.
             */
            std::vector<Gate*> m_base_gates;

            /**
             * A vector of all input nets to the gate subgraph, regardless of whether they appear in a word-level operation or not..
             */
            std::vector<Net*> m_total_input_nets;

            /**
             * A vector of all output nets of the subgrapg, regardless of whether they appear in a word-level operation or not.
             */
            std::vector<Net*> m_total_output_nets;

            /**
             * The set of contained candidate types.
             */
            std::set<CandidateType> m_types;

            /**
             * Indicates whether the candidate is verified.
             */
            bool m_verified;

            // void add_additional_data(std::string key, std::string value);
            // std::map<std::string, std::string> get_all_additional_data() const;

        private:
            /**
             * A map of additional data.
             */
            std::map<std::string, std::string> m_additional_data;
        };
    }    // namespace module_identification
}    // namespace hal
