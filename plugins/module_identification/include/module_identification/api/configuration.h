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
 * @file configuration.h
 * @brief This file contains the struct Configuration for module identification analysis.
 */

#pragma once

#include "hal_core/defines.h"
#include "module_identification/types/candidate_types.h"
#include "module_identification/types/multithreading_types.h"

#include <set>
#include <thread>
#include <vector>

namespace hal
{
    class Netlist;
    class Gate;

    namespace module_identification
    {
        /**
         * @struct Configuration
         * @brief Configuration for the module identification analysis.
         *
         * This struct holds important parameters that configure the module identification analysis, including netlist to analyze, known registers, candidate types to check, threading options, etc.
         */
        struct Configuration
        {
            /**
             * @brief Constructs a new ModuleIdentification analysis configuration for the given netlist.
             *
             * @param[in] nl - The netlist to be analyzed.
             */
            Configuration(Netlist* nl);

            /**
             * @brief Constructs an empty configuration.
             */
            Configuration();

            /**
             * @brief The netlist to be analyzed.
             */
            Netlist* m_netlist;

            /**
             * @brief A vector handling possibly known registers. 
             */
            std::vector<std::vector<Gate*>> m_known_registers = {};

            /**
             * @brief CandidateTypes that shall be checked. Defaults to all checkable candidate types.
             */
            std::vector<module_identification::CandidateType> m_types_to_check = all_checkable_candidate_types;

            /**
             * @brief Maximum number of concurrent threads created during execution. Defaults to 1.
             */
            u32 m_max_thread_count = 1;

            /**
             * @brief Maximum number of control signals to be tested. Defaults to 3.
             */
            u32 m_max_control_signals = 3;

            /**
             * @brief Gates to ignore during processing.
             */
            std::vector<std::vector<Gate*>> m_already_classified_candidates = {};

            /**
             * @brief Base candidates to block during analysis.
             */
            std::vector<std::set<Gate*>> m_blocked_base_candidates = {};

            /**
             * @brief Choose which `MultithreadingPriority` to use for the analysis. Defaults to memory priority
             */
            MultithreadingPriority m_multithreading_priority = MultithreadingPriority::memory_priority;

            /**
             * @brief Set the known registers for prioritization.
             *
             * @param[in] registers - The groups provided by a dana run.
             * @returns The updated module identification configuration.
             */
            Configuration& with_known_registers(const std::vector<std::vector<Gate*>>& registers);

            /**
             * @brief Set the maximum number of threads.
             *
             * @param[in] max_thread_count - The number of threads to be started at max.
             * @returns The updated module identification configuration.
             */
            Configuration& with_max_thread_count(const u32& max_thread_count);

            /**
             * @brief Set the maximum number of control signals to be tested.
             *
             * @param[in] max_control_signals - The number of control signals checked.
             * @returns The updated module identification configuration.
             */
            Configuration& with_max_control_signals(const u32& max_control_signals);

            /**
             * @brief Set the multithreading priority type.
             *
             * @param[in] priority - The type of multithreading used during execution.
             * @returns The updated module identification configuration.
             */
            Configuration& with_multithreading_priority(const MultithreadingPriority& priority);

            /**
             * @brief Set the candidate types to be checked.
             *
             * @param[in] types_to_check - A list of candidate types to be checked for.
             * @returns The updated module identification configuration.
             */
            Configuration& with_types_to_check(const std::vector<module_identification::CandidateType>& types_to_check);

            /**
             * @brief Add gates to be ignored during processing.
             * 
             * All candidates that are build during the module identification run that contain any gates that overlap with any already classified candidate are discarded to avoid conflicts.
             *
             * @param[in] already_classified_candidates - Candidates to be ignored.
             * @returns The updated module identification configuration.
             */
            Configuration& with_already_classified_candidates(const std::vector<std::vector<Gate*>>& already_classified_candidates);

            /**
             * @brief Add base candidates to be blocked during analysis.
             *
             * @param[in] blocked_base_candidates - Base candidates to be ignored.
             * @returns The updated module identification configuration.
             */
            Configuration& with_blocked_base_candidates(const std::vector<std::set<Gate*>>& blocked_base_candidates);
        };
    }    // namespace module_identification
}    // namespace hal
