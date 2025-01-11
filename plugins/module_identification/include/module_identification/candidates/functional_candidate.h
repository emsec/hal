// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/**
 * @file functional_candidate.h 
 * @brief This file contains the class and functions for handling functional candidates within the module identification process.
 */

#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/gate.h"
#include "module_identification/candidates/candidate_context.h"
#include "module_identification/candidates/structural_candidate.h"
#include "module_identification/candidates/verified_candidate.h"

namespace hal
{
    namespace module_identification
    {
        /**
         * @class FunctionalCandidate
         * @brief Represents a functional candidate derived from structural candidates.
         * 
         * This class is used to handle functional candidates within the module identification process, providing methods for their creation, manipulation, and verification.
         */
        class FunctionalCandidate
        {
        public:
            /**
             * @brief Constructor for `FunctionalCandidate`.
             * 
             * @param[in] sc - A pointer to the parent structural candidate.
             * @param[in] max_control_signal - The maximum number of control signals.
             * @param[in] candidate_type - The type of the candidate.
             */
            FunctionalCandidate(StructuralCandidate* sc, u32 max_control_signal, module_identification::CandidateType candidate_type);

            /**
             * @brief Create functional candidates from a structural candidate.
             * 
             * @param[in] sc - A pointer to the structural candidate.
             * @param[in] max_control_signal - The maximum number of control signals.
             * @param[in] ctx - The candidate context.
             * @param[in] candidate_type - The type of the candidates to create.
             * @param[in] registers - The cache of gate vectors.
             * @returns OK() and a vector of functional candidates on success, an error otherwise.
             */
            static hal::Result<std::vector<FunctionalCandidate>> create_candidates(StructuralCandidate* sc,
                                                                                   u32 max_control_signal,
                                                                                   CandidateContext& ctx,
                                                                                   module_identification::CandidateType candidate_type,
                                                                                   const std::vector<std::vector<Gate*>>& registers);

            // Comparisons

            /**
             * @brief Find control signals for a functional candidate.
             * 
             * @param[in] ctx - The candidate context.
             * @param[in] candidate - The functional candidate.
             * @returns OK() and a vector of functional candidates on success, an error otherwise.
             */
            static hal::Result<std::vector<FunctionalCandidate>> find_control_signals(CandidateContext& ctx, const FunctionalCandidate& candidate);

            /**
             * @brief Order input operands for a functional candidate.
             * 
             * @param[in] ctx - The candidate context.
             * @param[in] candidate - The functional candidate.
             * @returns OK() and a vector of functional candidates on success, an error otherwise.
             */
            static hal::Result<std::vector<FunctionalCandidate>> order_input_operands(CandidateContext& ctx, const FunctionalCandidate& candidate);

            /**
             * @brief Discard equal candidates based on their number of input signals. 
             * 
             * @param[in] ctx - The candidate context.
             * @param[in] candidate - The functional candidate.
             * @returns OK() and a vector of functional candidates on success, an error otherwise.
             */
            static hal::Result<std::vector<FunctionalCandidate>> discard_equal_candidate(CandidateContext& ctx, const FunctionalCandidate& candidate);

            /**
             * @brief Trim a functional candidate to a single output net.
             * 
             * @param[in] ctx - The candidate context.
             * @param[in] candidate - The functional candidate.
             * @returns OK() and a vector of functional candidates on success, an error otherwise.
             */
            static hal::Result<std::vector<FunctionalCandidate>> trim_to_single_output_net(CandidateContext& ctx, const FunctionalCandidate& candidate);

            /**
             * @brief Create sign extension variants of a functional candidate.
             * 
             * @param[in] ctx - The candidate context.
             * @param[in] candidate - The functional candidate.
             * @returns OK() and a vector of functional candidates on success, an error otherwise.
             */
            static hal::Result<std::vector<FunctionalCandidate>> create_sign_extension_variants(CandidateContext& ctx, const FunctionalCandidate& candidate);

            // Adder, subtractor, counter

            /**
             * @brief Identify control signals for an adder, subtractor, or counter.
             * 
             * @param[in] ctx - The candidate context.
             * @param[in] candidate - The functional candidate.
             * @returns OK() and a vector of functional candidates on success, an error otherwise.
             */
            static hal::Result<std::vector<FunctionalCandidate>> identify_control_signals(CandidateContext& ctx, const FunctionalCandidate& candidate);

            /**
             * @brief Realize control signals for an adder, subtractor, or counter by setting the control signals to concrete values.
             * 
             * @param[in] ctx - The candidate context.
             * @param[in] candidate - The functional candidate.
             * @returns OK() and a vector of functional candidates on success, an error otherwise.
             */
            static hal::Result<std::vector<FunctionalCandidate>> realize_control_signals(CandidateContext& ctx, const FunctionalCandidate& candidate);

            /**
             * @brief Update input and output statistics for an adder, subtractor, or counter.
             * 
             * @param[in] ctx - The candidate context.
             * @param[in] candidate - The functional candidate.
             * @returns OK() and a vector of functional candidates on success, an error otherwise.
             */
            static hal::Result<std::vector<FunctionalCandidate>> update_input_output_stats(CandidateContext& ctx, const FunctionalCandidate& candidate);

            /**
             * @brief Permute single input signals for an adder, subtractor, or counter.
             * 
             * @param[in] ctx - The candidate context.
             * @param[in] candidate - The functional candidate.
             * @returns OK() and a vector of functional candidates on success, an error otherwise.
             */
            static hal::Result<std::vector<FunctionalCandidate>> permute_single_input_signals(CandidateContext& ctx, const FunctionalCandidate& candidate);

            /**
             * @brief Build input operands for an adder, subtractor, or counter.
             * 
             * @param[in] ctx - The candidate context.
             * @param[in] candidate - The functional candidate.
             * @returns OK() and a vector of functional candidates on success, an error otherwise.
             */
            static hal::Result<std::vector<FunctionalCandidate>> build_input_operands(CandidateContext& ctx, const FunctionalCandidate& candidate);

            /**
             * @brief Order output signals for an adder, subtractor, or counter.
             * 
             * @param[in] ctx - The candidate context.
             * @param[in] candidate - The functional candidate.
             * @returns OK() and a vector of functional candidates on success, an error otherwise.
             */
            static hal::Result<std::vector<FunctionalCandidate>> order_output_signals(CandidateContext& ctx, const FunctionalCandidate& candidate);

            /**
             * @brief Create sign bit variants for an adder, subtractor, or counter.
             * 
             * @param[in] ctx - The candidate context.
             * @param[in] candidate - The functional candidate.
             * @returns OK() and a vector of functional candidates on success, an error otherwise.
             */
            static hal::Result<std::vector<FunctionalCandidate>> create_sign_bit_variants(CandidateContext& ctx, const FunctionalCandidate& candidate);

            /**
             * @brief Create input extension variants for an adder, subtractor, or counter.
             * 
             * @param[in] ctx - The candidate context.
             * @param[in] candidate - The functional candidate.
             * @returns OK() and a vector of functional candidates on success, an error otherwise.
             */
            static hal::Result<std::vector<FunctionalCandidate>> create_input_extension_variants(CandidateContext& ctx, const FunctionalCandidate& candidate);

            /**
             * @brief Create output net variants for an adder, subtractor, or counter.
             * 
             * @param[in] ctx - The candidate context.
             * @param[in] candidate - The functional candidate.
             * @returns OK() and a vector of functional candidates on success, an error otherwise.
             */
            static hal::Result<std::vector<FunctionalCandidate>> create_output_net_variant(CandidateContext& ctx, const FunctionalCandidate& candidate);

            /**
             * @brief Add single input signals for an adder, subtractor, or counter.
             * 
             * @param[in] ctx - The candidate context.
             * @param[in] candidate - The functional candidate.
             * @returns OK() and a vector of functional candidates on success, an error otherwise.
             */
            static hal::Result<std::vector<FunctionalCandidate>> add_single_input_signals(CandidateContext& ctx, const FunctionalCandidate& candidate);

            /**
             * @brief Early abort process for a functional candidate.
             * 
             * @param[in] ctx - The candidate context.
             * @param[in] candidate - The functional candidate.
             * @returns OK() and a vector of functional candidates on success, an error otherwise.
             */
            static hal::Result<std::vector<FunctionalCandidate>> early_abort(CandidateContext& ctx, const FunctionalCandidate& candidate);

            // Absolute

            /**
             * @brief Create operand control variations for absolute functional candidates.
             * 
             * @param[in] ctx - The candidate context.
             * @param[in] candidate - The functional candidate.
             * @returns OK() and a vector of functional candidates on success, an error otherwise.
             */
            static hal::Result<std::vector<FunctionalCandidate>> create_operand_control_variations(CandidateContext& ctx, const FunctionalCandidate& candidate);

            // Constant Multiplication

            /**
             * @brief Add n shifted operands for constant multiplication.
             * 
             * @param[in] candidate - The functional candidate.
             * @param[in] shift_vals - The shift values of the operands to add.
             * @returns A new functional candidate with the shifted operands added.
             */
            static FunctionalCandidate add_n_shifted_operands(const FunctionalCandidate& candidate, const std::vector<i32>& shift_vals);

            /**
             * @brief Add shifted operands to the functional candidate based on its input output stats.
             * 
             * @param[in] ctx - The candidate context.
             * @param[in] candidate - The functional candidate.
             * @returns OK() and a vector of functional candidates on success, an error otherwise.
             */
            static hal::Result<std::vector<FunctionalCandidate>> add_selected_shifted_operand(CandidateContext& ctx, const FunctionalCandidate& candidate);

            /**
             * @brief Add all possible variations of possible shifted operands to the functional candidate. 
             * 
             * @param[in] ctx - The candidate context.
             * @param[in] candidate - The functional candidate.
             * @returns OK() and a vector of functional candidates on success, an error otherwise.
             */
            static hal::Result<std::vector<FunctionalCandidate>> add_all_shifted_operand(CandidateContext& ctx, const FunctionalCandidate& candidate);

            // Value check

            /**
             * @brief Check the output size of a functional candidate.
             * 
             * @param[in] ctx - The candidate context.
             * @param[in] candidate - The functional candidate.
             * @returns OK() and a vector of functional candidates on success, an error otherwise.
             */
            static hal::Result<std::vector<FunctionalCandidate>> check_output_size(CandidateContext& ctx, const FunctionalCandidate& candidate);

            /**
             * @brief Build an input operand for a functional candidate by including all external input signals of the candidate gates.
             * 
             * @param[in] ctx - The candidate context.
             * @param[in] candidate - The functional candidate.
             * @returns OK() and a vector of functional candidates on success, an error otherwise.
             */
            static hal::Result<std::vector<FunctionalCandidate>> build_input_operand(CandidateContext& ctx, const FunctionalCandidate& candidate);

            /**
             * @brief Check whether a functional candidate with the given input operands and output functions is actually implementing the function specified by its type.
             * 
             * @param[in] output_functions - The output functions.
             * @param[in] registers - The cache of gate vectors. This is used to reoder operands for commutative opreations like addition.
             * @returns OK() and a verified candidate on success, an error otherwise.
             */
            hal::Result<VerifiedCandidate> check(const std::vector<BooleanFunction>& output_functions, const std::vector<std::vector<Gate*>>& registers);

            /**
             * @brief Add additional data to the functional candidate.
             * 
             * @param[in] key - The key for the data.
             * @param[in] value - The value for the data.
             */
            void add_additional_data(std::string key, std::string value);

            /**
             * @brief Get all additional data of the functional candidate.
             * 
             * @returns A map containing all additional data.
             */
            std::map<std::string, std::string> get_all_additional_data() const;

            /**
             * @brief Get candidate information as a string.
             * 
             * @returns A string containing the candidate information.
             */
            std::string get_candidate_info() const;

            /**
             * The type of the candidate.
             */
            module_identification::CandidateType m_candidate_type;

            /**
             * The gates associated with the candidate.
             */
            std::vector<Gate*> m_gates;

            /**
             * Base gates of the candidate.
             */
            std::vector<Gate*> m_base_gates;

            /**
             * Pointer to the parent structural candidate.
             */
            StructuralCandidate* m_structural_candidate;

            /**
             * Operands of the candidate.
             */
            std::vector<std::vector<Net*>> m_operands;

            /**
             * Control signals of the candidate.
             */
            std::vector<Net*> m_control_signals;

            /**
             * Control mapping for the candidate.
             */
            std::map<Net*, BooleanFunction::Value> m_control_mapping;

            /**
             * Mapping the count of influenced output nets to the list of input nets that influence this many outputs.
             */
            std::map<u32, std::vector<Net*>> m_influence_count_to_input_nets;

            /**
             * Mapping of input count to a list of output nets that are influenced by that many inputs.
             */
            std::map<u32, std::vector<Net*>> m_input_count_to_output_nets;

            /**
             * Mapping input nets that influence a output, that is not influenced by any other input
             */
            std::map<Net*, Net*> m_single_input_to_output;

            /**
             * Permuted single input-output pairs.
             */
            std::vector<std::pair<Net*, Net*>> m_permuted_single_pairs;

            /**
             * Input nets of the candidate.
             */
            std::vector<Net*> m_input_nets;

            /**
             * Output nets of the candidate.
             */
            std::vector<Net*> m_output_nets;

            /**
             * Maximum number of control signals.
             */
            u32 m_max_control_signals;

            /**
             * Maximum number of operands.
             */
            u32 m_max_operands;

            /**
             * Additional data for the candidate.
             */
            std::map<std::string, std::string> m_additional_data = {};

            /**
             * Special case for absolute candidates where one of the control values also applies to the operand.
             */
            Net* m_ctrl_to_operand_net;

            /**
             * Sign nets for the candidate.
             */
            std::vector<Net*> m_sign_nets;

            /**
             * Timing information and statistics.
             */
            std::map<std::string, std::map<std::string, std::map<std::string, std::map<std::string, u64>>>> m_timings;
        };
    }    // namespace module_identification
}    // namespace hal
