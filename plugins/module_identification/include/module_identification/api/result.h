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
 * @file result.h
 * @brief This file contains the structures and functions related to module identification results.
 */

#pragma once

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"
#include "module_identification/candidates/base_candidate.h"
#include "module_identification/candidates/verified_candidate.h"

#include <set>

namespace hal
{
    namespace module_identification
    {
        /**
         * @struct Result
         * @brief The result of a module identification run containing the candidates.
         */
        struct Result
        {
            /**
             * @brief Constructor for `Result`.
             * 
             * @param[in] nl - The netlist on which module identification has been performed.
             * @param[in] result - A vector of pairs containing base candidates and their verified candidates.
             * @param[in] timing_stats_json - A JSON string containing timing statistics. Defaults to an empty string.
             */
            Result(Netlist* nl, const std::vector<std::pair<BaseCandidate, VerifiedCandidate>>& result, const std::string& timing_stats_json = "");

            /**
             * @brief Get the netlist on which module identification has been performed.
             *
             * @returns The netlist.
             */
            Netlist* get_netlist() const;

            /**
             * @brief Get a map of the candidate IDs to the gates contained inside the verified candidates.
             * 
             * This map only contains verified candidates that are fully verified.
             * The ID is only unique for this result.
             *
             * @returns A map of candidate IDs to a vector of gates.
             */
            std::map<u32, std::vector<Gate*>> get_verified_candidate_gates() const;

            /**
             * @brief Get a map of the candidate IDs to the verified candidates.
             * 
             * This map only contains verified candidates that are fully verified.
             * The ID is only unique for this result.
             *
             * @returns A map of candidate IDs to verified candidates.
             */
            std::map<u32, VerifiedCandidate> get_verified_candidates() const;

            /**
             * @brief Get a map of the candidate IDs to the gates contained inside the candidate.
             * 
             * This map contains all checked candidates, even the ones not verified.
             * The ID is only unique for this result.
             *
             * @returns A map of candidate IDs to a vector of gates.
             */
            std::map<u32, std::vector<Gate*>> get_candidate_gates() const;

            /**
             * @brief Get a map of the candidate IDs to the candidates.
             * 
             * This map contains all checked candidates, even the ones not verified.
             * The ID is only unique for this result.
             *
             * @returns A map of candidate IDs to candidates.
             */
            std::map<u32, VerifiedCandidate> get_candidates() const;

            /**
             * @brief Get the gates of the candidate with the corresponding ID.
             *
             * @param[in] id - The ID of the requested candidate.
             * @returns OK() and a vector of gates on success, an error otherwise.
             */
            hal::Result<std::vector<Gate*>> get_candidate_gates_by_id(const u32 id) const;

            /**
             * @brief Returns the candidate with the corresponding ID.
             *
             * @param[in] id - The ID of the requested candidate.
             * @returns OK() and the verified candidate on success, an error otherwise.
             */
            hal::Result<VerifiedCandidate> get_candidate_by_id(const u32 id) const;

            /**
             * @brief Get all gates contained in any of the candidates.
             *
             * @returns A set of gates.
             */
            std::set<Gate*> get_all_gates() const;

            /**
             * @brief Get all gates contained in any of the verified candidates.
             *
             * @returns A set of gates.
             */
            std::set<Gate*> get_all_verified_gates() const;

            /**
             * @brief Creates a HAL module for each candidate of the result.
             *
             * @returns OK() on success, an error otherwise.
             */
            hal::Result<std::monostate> create_modules_in_netlist();

            /**
             * @brief Get the collected timing information formatted as a JSON string.
             *
             * @returns A JSON formatted string.
             */
            std::string get_timing_stats() const;

            /**
             * @brief Merges two results by combining the found verified candidates.
             * 
             * When both results contain a verified candidate for the same base candidate, the better one is chosen via the same post-processing used in the orignal module identificaion process.
             * This requires that the base candidates are identical and that all gates of all candidates still exist in the netlist!
             *
             * @param[in] other - Another module identification result that is merged with this one.
             * @param[in] dana_cache - A list of previously identified register groupings that is used in the post-processing.
             * @returns OK() and a new merged module identification result on success, an error otherwise.
             */
            hal::Result<Result> merge(const Result& other, const std::vector<std::vector<Gate*>>& dana_cache) const;

            /**
             * @brief For different runs of the plugin figure out in which iteration the plugin found the highest quality result for each candidate.
             * 
             * This is used to compare the results of different runs of the plugin and afterwards get a list of base candidates for each execution for which this execution gave the best results.
             *
             * @param[in] iteration_results - A vector of all the execution results.
             * @param[in] create_block_lists - A parameter to determine whether to create allow or block lists.
             * @returns A vector of allow or block lists for each plugin execution iteration.
             */
            static std::vector<std::vector<std::set<Gate*>>> assign_base_candidates_to_iterations(const std::vector<Result>& iteration_results, const bool create_block_lists = false);

        private:
            /**
             * @brief The netlist on which module identification has been performed.
             */
            Netlist* m_netlist;

            /**
             * @brief A vector of pairs containing base candidates and their verified candidates.
             */
            std::vector<std::pair<BaseCandidate, VerifiedCandidate>> m_candidates;

            /**
             * @brief A JSON string containing timing statistics. Defaults to an empty string.
             */
            std::string m_timing_stats_json = "";
        };

    }    // namespace module_identification
}    // namespace hal
