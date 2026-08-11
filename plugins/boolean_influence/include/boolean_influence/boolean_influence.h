
#pragma once

#include "hal_core/defines.h"
#include "hal_core/utilities/result.h"
#include "z3_utils/z3_utils.h"

#include <map>
#include <unordered_map>
#include <vector>

namespace hal
{
    class BooleanFunction;
    class Gate;
    class Net;
    class Netlist;

    /**
     * Computes the Boolean influence of the input variables of a Boolean function.
     *
     * The Boolean influence of a variable is the fraction of input assignments for which flipping that variable
     * also flips the output of the function. It therefore ranges from `0.0` for a variable that the function does
     * not depend on to `1.0` for a variable that the output follows directly.
     *
     * @ingroup plugins
     */
    namespace boolean_influence
    {
        /**
         * Compute the Boolean influence of each input variable of a Boolean function.
         * The influence is approximated by evaluating the function on randomly sampled input assignments.
         *
         * @param[in] bf - The Boolean function.
         * @param[in] num_evaluations - The number of evaluations that are performed for each input variable.
         * @returns A map from each variable of the function to its Boolean influence on success, an error otherwise.
         */
        Result<std::unordered_map<std::string, double>> get_boolean_influence(const BooleanFunction& bf, const u32 num_evaluations = 32000);

        /**
         * Compute the Boolean influence of each input variable of a Boolean function using only HAL-internal functionality.
         * The influence is approximated by evaluating the function on randomly sampled input assignments.
         * This variant is slower than `get_boolean_influence`, but it is better suited for use in a multi-threaded environment.
         *
         * @param[in] bf - The Boolean function.
         * @param[in] num_evaluations - The number of evaluations that are performed for each input variable.
         * @returns A map from each variable of the function to its Boolean influence on success, an error otherwise.
         */
        Result<std::unordered_map<std::string, double>> get_boolean_influence_with_hal_boolean_function_class(const BooleanFunction& bf, const u32 num_evaluations);

        /**
         * Compute the Boolean influence of each input variable of a Boolean function using only z3 substitution and simplification.
         * The influence is approximated by evaluating the function on randomly sampled input assignments.
         * This variant is slower than `get_boolean_influence`, but it is better suited for use in a multi-threaded environment.
         *
         * @param[in] bf - The Boolean function.
         * @param[in] num_evaluations - The number of evaluations that are performed for each input variable.
         * @returns A map from each variable of the function to its Boolean influence on success, an error otherwise.
         */
        Result<std::unordered_map<std::string, double>> get_boolean_influence_with_z3_expr(const BooleanFunction& bf, const u32 num_evaluations);

        /**
         * Compute the Boolean influence of each input variable of a Boolean function.
         * The influence is approximated by evaluating the function on randomly sampled input assignments.
         *
         * @param[in] e - The z3 expression representing the Boolean function.
         * @param[in] num_evaluations - The number of evaluations that are performed for each input variable.
         * @returns A map from each variable of the function to its Boolean influence on success, an error otherwise.
         */
        Result<std::unordered_map<std::string, double>> get_boolean_influence(const z3::expr& e, const u32 num_evaluations = 32000);

        /**
         * Compute the exact Boolean influence of each input variable of a Boolean function.
         * In contrast to `get_boolean_influence`, the function is evaluated on every possible input assignment instead of a random sample.
         * This is only feasible for functions of at most 16 variables.
         *
         * @param[in] bf - The Boolean function.
         * @returns A map from each variable of the function to its Boolean influence on success, an error otherwise.
         */
        Result<std::unordered_map<std::string, double>> get_boolean_influence_deterministic(const BooleanFunction& bf);

        /**
         * Compute the exact Boolean influence of each input variable of a Boolean function.
         * In contrast to `get_boolean_influence`, the function is evaluated on every possible input assignment instead of a random sample.
         * This is only feasible for functions of at most 16 variables.
         *
         * @param[in] e - The z3 expression representing the Boolean function.
         * @returns A map from each variable of the function to its Boolean influence on success, an error otherwise.
         */
        Result<std::unordered_map<std::string, double>> get_boolean_influence_deterministic(const z3::expr& e);

        /**
         * Compute the Boolean influence of each input net of a subcircuit on one of its output nets.
         * The Boolean function of the start net is built from the given gates, translated into C code, and then compiled and executed for speed.
         * The influence is approximated by evaluating that function on randomly sampled input assignments.
         *
         * @param[in] gates - The gates of the subcircuit.
         * @param[in] start_net - The output net of the subcircuit at which to start the analysis.
         * @param[in] num_evaluations - The number of evaluations that are performed for each input variable.
         * @returns A map from each input net of the subcircuit to its Boolean influence on the start net on success, an error otherwise.
         */
        Result<std::map<Net*, double>> get_boolean_influences_of_subcircuit(const std::vector<Gate*>& gates, const Net* start_net, const u32 num_evaluations = 32000);

        /**
         * Compute the Boolean influence of each net that drives the data input of the given flip-flop.
         * The Boolean function of the data input net is built, translated into C code, and then compiled and executed for speed.
         * The influence is approximated by evaluating that function on randomly sampled input assignments.
         *
         * @param[in] gate - The flip-flop whose data input net is used to build the Boolean function.
         * @param[in] num_evaluations - The number of evaluations that are performed for each input variable.
         * @returns A map from each net of the function to its Boolean influence on the data input net on success, an error otherwise.
         */
        Result<std::map<Net*, double>> get_boolean_influences_of_gate(const Gate* gate, const u32 num_evaluations = 32000);

        /**
         * Compute the exact Boolean influence of each input net of a subcircuit on one of its output nets.
         * In contrast to `get_boolean_influences_of_subcircuit`, the function is evaluated on every possible input assignment instead of a random sample.
         * This is only feasible for subcircuits with at most 16 input nets.
         *
         * @param[in] gates - The gates of the subcircuit.
         * @param[in] start_net - The output net of the subcircuit at which to start the analysis.
         * @returns A map from each input net of the subcircuit to its Boolean influence on the start net on success, an error otherwise.
         */
        Result<std::map<Net*, double>> get_boolean_influences_of_subcircuit_deterministic(const std::vector<Gate*>& gates, const Net* start_net);

        /**
         * Compute the exact Boolean influence of each net that drives the data input of the given flip-flop.
         * In contrast to `get_boolean_influences_of_gate`, the function is evaluated on every possible input assignment instead of a random sample.
         * This is only feasible for data input functions of at most 16 nets.
         *
         * @param[in] gate - The flip-flop whose data input net is used to build the Boolean function.
         * @returns A map from each net of the function to its Boolean influence on the data input net on success, an error otherwise.
         */
        Result<std::map<Net*, double>> get_boolean_influences_of_gate_deterministic(const Gate* gate);

        /**
         * Get the flip-flop dependency matrix of a netlist, i.e., a matrix that holds an entry for every pair of flip-flops that are connected through combinational logic.
         *
         * @param[in] netlist - The netlist to extract the dependency matrix from.
         * @param[in] with_boolean_influence - Set `true` to use the Boolean influence as the matrix entry, `false` to use `1.0` for every connection.
         * @returns A pair consisting of a map from the original gate IDs to the corresponding matrix indices and the flip-flop dependency matrix itself, an error otherwise.
         */
        Result<std::pair<std::map<u32, Gate*>, std::vector<std::vector<double>>>> get_ff_dependency_matrix(const Netlist* netlist, bool with_boolean_influence);
    }    // namespace boolean_influence
}    // namespace hal