// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
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
 * @file cipher_candidate.h
 * @brief This file contains the class that holds all information on a candidate for a symmetric cryptographic implementation.
 */

#pragma once

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/utilities/result.h"
#include "hawkeye/candidate_search.h"
#include "hawkeye/sbox_database.h"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace hal
{
    class Gate;
    class Module;
    class Net;
    class Netlist;

    namespace graph_algorithm
    {
        class NetlistGraph;
    }

    namespace hawkeye
    {
        /**
         * @brief The outcome of trying to identify an S-box, see `CipherCandidate::identify_sboxes`.
         */
        enum class SBoxStatus
        {
            unidentified, /**< Identification ran but the S-box is not contained in the database. */
            identified,   /**< Identification ran and matched, see `SBox::identified_as`. */
            superseded,   /**< Identification did not run, another variant of the same S-box was identified before. */
        };

        /**
         * @brief An S-box located within the round function of a `CipherCandidate`.
         *
         * Owned by the candidate it was located in, which also owns every gate it refers to, so an S-box is only valid
         * for as long as its candidate is.
         *
         * The exact size and shape of an S-box is not known in advance, so the search deliberately produces more S-boxes
         * than the round function actually contains, among them smaller ones nested inside larger ones. Identification
         * resolves that, see `SBoxStatus`.
         */
        struct SBox
        {
            /**
             * @brief The gates of the connected component that the S-box was located in, including its input flip-flops.
             */
            std::vector<Gate*> component;

            /**
             * @brief The input flip-flops of the S-box, ordered by gate ID.
             *
             * These are the flip-flops of the state register that the S-box reads, and hence the only link between the
             * identified S-box and the state bits it operates on. They are **not** ordered by S-box input bit: the
             * database matches under affine equivalence, which absorbs any permutation of the input and output bits, so
             * no bit correspondence is established during identification.
             */
            std::vector<Gate*> input_gates;

            /**
             * @brief The output gates of the S-box, ordered by gate ID. Usually combinational gates feeding the linear layer.
             */
            std::vector<Gate*> output_gates;

            /**
             * @brief The name of the S-box in the database it was identified as, empty unless `status` is `identified`.
             */
            std::string identified_as;

            /**
             * @brief The outcome of trying to identify the S-box, `unidentified` until `identify_sboxes` ran.
             *
             * Distinguishes an S-box that was checked against the database without a match from one that was never
             * checked because a larger S-box containing it had already been identified.
             */
            SBoxStatus status = SBoxStatus::unidentified;

            /**
             * @brief Get the combinational gates computing the outputs of the S-box from its input flip-flops.
             *
             * Walks back from the output gates within the component and stops at the flip-flops, so the result is the
             * logic of this S-box alone rather than that of the whole component, which several S-boxes may share.
             *
             * @returns The combinational gates of the S-box, ordered by gate ID.
             */
            std::vector<Gate*> get_combinational_gates() const;
        };

        /**
         * @brief A candidate for a symmetric cryptographic implementation within a netlist.
         *
         * A candidate is discovered by `detect` in stages and is filled in as the analysis proceeds: detection only
         * establishes the state register, `build_round_function` adds the combinational logic computing the next state,
         * and `locate_sboxes` adds the S-boxes within that logic. Use `has_round_function` and `get_sboxes` to find out
         * how far a candidate has been analyzed.
         *
         * All gates and nets of a candidate belong to the netlist it was detected in, so they can be inspected and
         * grouped into modules directly.
         */
        class CipherCandidate
        {
        public:
            CipherCandidate() = default;
            ~CipherCandidate() = default;

            /**
             * @brief Construct a round-based candidate, i.e., one whose input and output register are the same.
             *
             * @param[in] round_reg - The state register of the candidate.
             */
            explicit CipherCandidate(const std::set<Gate*>& round_reg);

            /**
             * @brief Construct a candidate from an input and an output register. The candidate is round-based if both
             * registers are equal.
             *
             * @param[in] in_reg - The input register of the candidate.
             * @param[in] out_reg - The output register of the candidate.
             */
            CipherCandidate(const std::set<Gate*>& in_reg, const std::set<Gate*>& out_reg);

            CipherCandidate(CipherCandidate&&) = default;
            CipherCandidate& operator=(CipherCandidate&&) = default;

            /**
             * The S-boxes are owned through pointers that are handed out to the caller, so copying a candidate would
             * silently produce a second set of S-boxes at different addresses. Candidates are moved, never copied.
             */
            CipherCandidate(const CipherCandidate&) = delete;
            CipherCandidate& operator=(const CipherCandidate&) = delete;

            /**
             * @brief Attempt to locate candidates for symmetric cryptographic SPN, Feistel, and ARX implementations within a gate-level netlist.
             *
             * Operates on an abstraction of the netlist that holds only the flip-flops as vertices and their
             * connections through combinational logic as edges. Computes the k-neighborhood of every flip-flop for
             * `k = 1, ..., config.timeout` and stops once the size of the neighborhood saturates, at which point a
             * candidate is created if the neighborhood is larger than `config.min_register_size`. Depending on the
             * `config`, further criteria narrow down the search, see `DetectionConfiguration::Control` and
             * `DetectionConfiguration::Components`. The candidates found are then reduced by discarding those smaller
             * than `min_state_size` as well as those that fully contain a smaller candidate.
             *
             * The returned candidates only know their state register, call `build_round_function` on a candidate to
             * analyze it further.
             *
             * @param[in] nl - The netlist to operate on.
             * @param[in] configs - The configurations of the detection approaches to be executed one after another on each start flip-flop.
             * @param[in] min_state_size - The minimum size of a candidate to be considered a cryptographic state register. Defaults to `40`.
             * @param[in] start_ffs - The flip-flops to analyze. Defaults to an empty vector, i.e., all flip-flops of the netlist are analyzed.
             * @returns The candidates on success, an error otherwise.
             */
            static Result<std::vector<CipherCandidate>>
                detect(Netlist* nl, const std::vector<DetectionConfiguration>& configs, u32 min_state_size = 40, const std::vector<Gate*>& start_ffs = {});

            /**
             * @brief Determine the round function of the candidate, i.e., the combinational logic computing the next state.
             *
             * Determines the state logic between the input and the output register together with the state, control, and
             * other inputs of the candidate, and builds the graph that `locate_sboxes` operates on. Discards any S-boxes
             * located so far, as they are derived from the round function, which invalidates all pointers previously
             * returned by `locate_sboxes` and `get_sboxes`.
             *
             * Recomputes the round function on every call, which only makes a difference if the netlist changed in the
             * meantime.
             *
             * @returns Ok on success, an error otherwise.
             */
            Result<std::monostate> build_round_function();

            /**
             * @brief Try to locate S-boxes within the round function of the candidate.
             *
             * Computes an initial set of connected components within the round function. If these components are
             * reasonably small and their input and output sizes match, they are turned into S-boxes right away.
             * Otherwise, iteratively considers more combinational gates starting from the components' input gates and
             * searches for sub-components.
             *
             * Returns the S-boxes located by an earlier call unchanged instead of locating them again, so the returned
             * pointers stay valid. Call `clear_sboxes` to locate them anew.
             *
             * @returns The S-boxes of the candidate on success, an error otherwise.
             */
            Result<std::vector<SBox*>> locate_sboxes();

            /**
             * @brief Discard the S-boxes located so far.
             *
             * Invalidates all pointers previously returned by `locate_sboxes` and `get_sboxes`.
             */
            void clear_sboxes();

            /**
             * @brief Try to identify all S-boxes of the candidate by matching them against a database of known S-boxes.
             *
             * Annotates every S-box with the outcome, see `SBox::status` and `SBox::identified_as`. An S-box that is not
             * contained in the database is not an error.
             *
             * Since the exact outputs of an S-box are not known in advance, `locate_sboxes` produces many variants of
             * the same S-box that differ only in which of the surplus gates are taken as its outputs but all read the
             * same input flip-flops. Variants are therefore identified as a group, and the group is left as soon as one
             * of them matches, marking the remaining ones `superseded`. This is where most of the runtime would
             * otherwise go, as identifying an S-box costs far more than locating it.
             *
             * @param[in] db - The database of known S-boxes.
             * @returns The number of identified S-boxes on success, an error otherwise.
             */
            Result<u32> identify_sboxes(const SBoxDatabase& db);

            /**
             * @brief Try to identify a single S-box of this candidate by matching it against a database of known S-boxes under affine equivalence.
             *
             * Tries every assignment of the control inputs that the S-box reads, as the round function computes the
             * S-box for one of them and something else for the others, and the right one is not known in advance. The
             * remaining inputs are held at `0`.
             *
             * Does not annotate the S-box, use `identify_sboxes` for that.
             *
             * @param[in] sbox - The S-box to identify. Must be one of the S-boxes of this candidate.
             * @param[in] db - The database of known S-boxes.
             * @returns The name of the S-box on success and an empty string if it is not contained in the database, an error otherwise.
             */
            Result<std::string> identify_sbox(const SBox* sbox, const SBoxDatabase& db) const;

            /**
             * @brief Try to identify an S-box given as one Boolean function per output bit by matching it against a
             * database of known S-boxes under affine equivalence.
             *
             * Evaluates the Boolean functions into a truth table and looks that up in the database, so it works on
             * functions that do not come from a netlist at all. Every variable occurring in them is taken as an input
             * bit of the S-box, so substitute anything that is not one beforehand.
             *
             * @param[in] output_functions - The Boolean functions of the S-box, one per output bit. Their order does not matter, as affine equivalence absorbs a permutation of the output bits.
             * @param[in] db - The database of known S-boxes.
             * @returns The name of the S-box on success and an empty string if it is not contained in the database, an error otherwise.
             */
            static Result<std::string> identify_sbox(const std::vector<BooleanFunction>& output_functions, const SBoxDatabase& db);

            /**
             * @brief Write the candidate back into the netlist as a module hierarchy.
             *
             * Creates one module holding the entire candidate, a submodule holding its state register, and one submodule
             * per identified S-box holding its combinational gates. S-boxes that were not identified are skipped, as are
             * identified S-boxes that overlap an S-box module already created, since a gate belongs to exactly one
             * module. Every skipped S-box is reported to the log.
             *
             * @returns The module holding the candidate on success, an error otherwise.
             */
            Result<Module*> create_modules();

            /**
             * @brief Get the netlist that the candidate belongs to.
             *
             * @returns The netlist of the candidate.
             */
            Netlist* get_netlist() const;

            /**
             * @brief Get the size of the candidate, i.e., the width of its state register.
             *
             * @returns The size of the candidate.
             */
            u32 get_size() const;

            /**
             * @brief Check whether the candidate is round-based, i.e., whether its input and output register are the same.
             *
             * @returns `true` if the candidate is round-based, `false` if it is pipelined.
             */
            bool is_round_based() const;

            /**
             * @brief Check whether the round function of the candidate has been computed, see `build_round_function`.
             *
             * @returns `true` if the round function has been computed, `false` otherwise.
             */
            bool has_round_function() const;

            /**
             * @brief Get the input register of the candidate, ordered by gate ID.
             *
             * @returns The input register of the candidate.
             */
            const std::vector<Gate*>& get_input_reg() const;

            /**
             * @brief Get the output register of the candidate, ordered by gate ID. Equal to the input register for a
             * round-based candidate.
             *
             * @returns The output register of the candidate.
             */
            const std::vector<Gate*>& get_output_reg() const;

            /**
             * @brief Get the combinational logic computing the next state, ordered by gate ID.
             *
             * @returns The round function of the candidate, empty if it has not been computed yet.
             */
            const std::vector<Gate*>& get_round_logic() const;

            /**
             * @brief Get all gates of the candidate, i.e., its registers together with its round function, ordered by gate ID.
             *
             * @returns The gates of the candidate.
             */
            std::vector<Gate*> get_gates() const;

            /**
             * @brief Get the S-boxes located within the round function of the candidate.
             *
             * @returns The S-boxes of the candidate, empty if they have not been located yet.
             */
            std::vector<SBox*> get_sboxes() const;

            /**
             * @brief Get the graph of the round function, in which the gates of the state register are represented by a
             * primary and a shadow vertex so that the feedback of a round-based candidate does not close a cycle.
             *
             * @returns The graph of the round function, `nullptr` if the round function has not been computed yet.
             */
            graph_algorithm::NetlistGraph* get_graph() const;

            /**
             * @brief Get the state inputs of the round function.
             *
             * @returns The state inputs of the candidate.
             */
            const std::set<Net*>& get_state_inputs() const;

            /**
             * @brief Get the control inputs of the round function.
             *
             * @returns The control inputs of the candidate.
             */
            const std::set<Net*>& get_control_inputs() const;

            /**
             * @brief Get the remaining inputs of the round function.
             *
             * @returns The other inputs of the candidate.
             */
            const std::set<Net*>& get_other_inputs() const;

            /**
             * @brief Get the state outputs of the round function.
             *
             * @returns The state outputs of the candidate.
             */
            const std::set<Net*>& get_state_outputs() const;

            /**
             * @brief Get a map from each gate of the round function to the input flip-flops it depends on.
             *
             * @returns A map from gates to sets of input flip-flops.
             */
            const std::map<Gate*, std::set<Gate*>>& get_input_ffs_of_gate() const;

            /**
             * @brief Get a map from a distance to all gates reachable within at most that distance from any input flip-flop.
             *
             * @returns A map from longest distance to a set of gates.
             */
            const std::map<u32, std::set<Gate*>>& get_longest_distance_to_gate() const;

            /**
             * @brief Compare two candidates.
             *
             * Larger candidates come first, as reducing a set of candidates relies on a candidate being visited before
             * the smaller candidates that it may contain. Ties are broken by the gate IDs of the registers so that the
             * order does not depend on where the gates of the netlist happen to be allocated.
             *
             * @param[in] rhs - The candidate to compare against.
             * @returns `true` if this candidate is ordered before `rhs`, `false` otherwise.
             */
            bool operator<(const CipherCandidate& rhs) const;

            /**
             * @brief Check whether two candidates have the same size and the same registers.
             *
             * @param[in] rhs - The candidate to compare against.
             * @returns `true` if both candidates are equal, `false` otherwise.
             */
            bool operator==(const CipherCandidate& rhs) const;

        private:
            /** The netlist that the candidate belongs to. */
            Netlist* m_netlist = nullptr;

            /** The width of the state register. */
            u32 m_size = 0;

            /** Is `true` if input and output register are the same. */
            bool m_is_round_based = false;

            /** The input register, ordered by gate ID. */
            std::vector<Gate*> m_in_reg;

            /** The output register, ordered by gate ID. Equal to `m_in_reg` for a round-based candidate. */
            std::vector<Gate*> m_out_reg;

            /**
             * The IDs of the gates of `m_in_reg` and `m_out_reg` in ascending order.
             *
             * Candidates are compared by gate ID so that their order does not depend on where the gates of the netlist
             * happen to be allocated. Detection compares them often enough for that to matter, so the IDs are computed
             * once on construction rather than on every comparison.
             */
            std::vector<u32> m_in_reg_ids, m_out_reg_ids;

            /** The combinational logic computing the next state, ordered by gate ID. Empty until `build_round_function`. */
            std::vector<Gate*> m_round_logic;

            /** The graph of the round function, `nullptr` until `build_round_function`. */
            std::unique_ptr<graph_algorithm::NetlistGraph> m_graph;

            /** The inputs and outputs of the round function. */
            std::set<Net*> m_state_inputs, m_state_outputs, m_control_inputs, m_other_inputs;

            /**
             * A map from each gate of the round function to the input flip-flops it depends on, and a map from a
             * distance to all gates reachable within at most that distance from any input flip-flop.
             *
             * Filled in by `compute_gate_dependencies` on first use rather than by `build_round_function`, hence
             * `mutable`: computing them walks the round function once per flip-flop of the register, which is the
             * most expensive part of analyzing a candidate and is only ever read when splitting a component into
             * several S-boxes.
             */
            mutable std::map<Gate*, std::set<Gate*>> m_input_ffs_of_gate;
            mutable std::map<u32, std::set<Gate*>> m_longest_distance_to_gate;

            /** Is `true` once `compute_gate_dependencies` has filled in the two maps above. */
            mutable bool m_has_gate_dependencies = false;

            /**
             * Determine which input flip-flops each gate of the round function depends on and how far each gate is
             * from the register, unless that has already been done for the current round function.
             */
            void compute_gate_dependencies() const;

            /**
             * The S-boxes located within the round function.
             *
             * Held by pointer so that the addresses handed out by `locate_sboxes` survive the candidate being moved
             * around, for instance while the vector returned by `detect` grows.
             */
            std::vector<std::unique_ptr<SBox>> m_sboxes;

            /** Is `true` once `build_round_function` completed successfully. */
            bool m_has_round_function = false;
        };
    }    // namespace hawkeye
}    // namespace hal
