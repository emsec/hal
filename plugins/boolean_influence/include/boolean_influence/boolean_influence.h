
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

    namespace boolean_influence
    {
        /**
         * Generates the Boolean influence of each input variable of a Boolean function.
         *
         * @param[in] bf - The Boolean function.
         * @param[in] num_evaluations - The amount of evaluations that are performed for each input variable.
         * @param[in] unique_identifier - A unique identifier that is applied to file names to prevent collisions during multi-threading.
         * @returns A map from the variables that appear in the function to their Boolean influence on said function on success, an error otherwise.
         */
        Result<std::unordered_map<std::string, double>> get_boolean_influence(const BooleanFunction& bf, const u32 num_evaluations = 32000, const std::string& unique_identifier = "");

        /**
         * Generates the Boolean influence of each input variable of a Boolean function.
         *
         * @param[in] e - The z3 expression representing a Boolean function.
         * @param[in] num_evaluations - The amount of evaluations that are performed for each input variable.
         * @param[in] unique_identifier - A unique identifier that is applied to file names to prevent collisions during multi threading.
         * @returns A map from the variables that appear in the function to their Boolean influence on said function on success, an error otherwise.
         */
        Result<std::unordered_map<std::string, double>> get_boolean_influence(const z3::expr& e, const u32 num_evaluations = 32000, const std::string& unique_identifier = "");

        /**
         * Generates the Boolean influence of each input variable of a Boolean function.
         *
         * @param[in] bf - The Boolean function.
         * @param[in] unique_identifier - A unique identifier that is applied to file names to prevent collisions during multi-threading.
         * @returns A map from the variables that appear in the function to their Boolean influence on said function on success, an error otherwise.
         */
        Result<std::unordered_map<std::string, double>> get_boolean_influence_deterministic(const BooleanFunction& bf, const std::string& unique_identifier = "");

        /**
         * Generates the Boolean influence of each input variable of a Boolean function.
         *
         * @param[in] e - The z3 expression representing a Boolean function.
         * @param[in] unique_identifier - A unique identifier that is applied to file names to prevent collisions during multi threading.
         * @returns A map from the variables that appear in the function to their Boolean influence on said function on success, an error otherwise.
         */
        Result<std::unordered_map<std::string, double>> get_boolean_influence_deterministic(const z3::expr& e, const std::string& unique_identifier = "");

        /**
         * Generates the function of the net using only the given gates.
         * Afterwards the generated function gets translated from a z3::expr to efficent c code, compiled, executed and evaluated.
         *
         * @param[in] gates - The gates of the subcircuit.
         * @param[in] start_net - The output net of the subcircuit at which to start the analysis.
         * @param[in] num_evaluations - The amount of evaluations that are performed for each input variable.
         * @param[in] unique_identifier - A unique identifier that is applied to file names to prevent collisions during multi threading.
         * @returns A map from the nets that appear in the function of the start net to their Boolean influence on said function on success, an error otherwise.
         */
        Result<std::map<Net*, double>>
            get_boolean_influences_of_subcircuit(const std::vector<Gate*>& gates, const Net* start_net, const u32 num_evaluations = 32000, const std::string& unique_identifier = "");

        /**
         * Generates the function of the dataport net of the given flip-flop.
         * Afterwards the generated function gets translated from a z3::expr to efficient c code, compiled, executed and evaluated.
         *
         * @param[in] gate - Pointer to the flip-flop which data input net is used to build the Boolean function.
         * @param[in] num_evaluations - The amount of evaluations that are performed for each input variable.
         * @param[in] unique_identifier - A unique identifier that is applied to file names to prevent collisions during multi threading.
         * @returns A map from the nets that appear in the function of the data net to their Boolean influence on said function on success, an error otherwise.
         */
        Result<std::map<Net*, double>> get_boolean_influences_of_gate(const Gate* gate, const u32 num_evaluations = 32000, const std::string& unique_identifier = "");

        /**
         * Generates the function of the net using only the given gates.
         * Afterwards the generated function gets translated from a z3::expr to efficent c code, compiled, executed and evaluated.
         *
         * @param[in] gates - The gates of the subcircuit.
         * @param[in] start_net - The output net of the subcircuit at which to start the analysis.
         * @param[in] unique_identifier - A unique identifier that is applied to file names to prevent collisions during multi threading.
         * @returns A map from the nets that appear in the function of the start net to their Boolean influence on said function on success, an error otherwise.
         */
        Result<std::map<Net*, double>> get_boolean_influences_of_subcircuit_deterministic(const std::vector<Gate*>& gates, const Net* start_net, const std::string& unique_identifier = "");

        /**
         * Generates the function of the dataport net of the given flip-flop.
         * Afterwards the generated function gets translated from a z3::expr to efficient c code, compiled, executed and evaluated.
         *
         * @param[in] gate - Pointer to the flip-flop which data input net is used to build the Boolean function.
         * @param[in] unique_identifier - A unique identifier that is applied to file names to prevent collisions during multi threading.
         * @returns A map from the nets that appear in the function of the data net to their Boolean influence on said function on success, an error otherwise.
         */
        Result<std::map<Net*, double>> get_boolean_influences_of_gate_deterministic(const Gate* gate, const std::string& unique_identifier = "");

        /**
         * Generates the Boolean influence of each input variable of a Boolean function using the internal HAL functions only
         * The function is slower, but can be better used in multithreading enviroment.
         *
         * @param[in] bf - The Boolean function.
         * @param[in] num_evaluations - The amount of evaluations that are performed for each input variable.
         * @returns A map from the variables that appear in the function to their Boolean influence on said function on success, an error otherwise.
         */
        static Result<std::unordered_map<std::string, double>> get_boolean_influence_with_hal_boolean_function_class(const BooleanFunction& bf, const u32 num_evaluations);

        /**
         * Generates the Boolean influence of each input variable of a Boolean function using z3 expressions and substitutions/simplifications only.
         * The function is slower, but can be better used in multithreading enviroment.
         *
         * @param[in] bf - The Boolean function.
         * @param[in] num_evaluations - The amount of evaluations that are performed for each input variable.
         * @returns A map from the variables that appear in the function to their Boolean influence on said function on success, an error otherwise.
         */
        static Result<std::unordered_map<std::string, double>> get_boolean_influence_with_z3_expr(const BooleanFunction& bf, const u32 num_evaluations);

        /**
         * Get the FF dependency matrix of a netlist.
         *
         * @param[in] netlist - The netlist to extract the dependency matrix from.
         * @param[in] with_boolean_influence - True -- set Boolean influence, False -- sets 1.0 if connection between FFs
         * @returns A pair consisting of std::map<u32, Gate*>, which includes the mapping from the original gate
         *          IDs to the ones in the matrix, and a std::vector<std::vector<double>, which is the ff dependency matrix
         */
        Result<std::pair<std::map<u32, Gate*>, std::vector<std::vector<double>>>> get_ff_dependency_matrix(const Netlist* netlist, bool with_boolean_influence);
    }    // namespace boolean_influence
}    // namespace hal