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

#pragma once

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/plugin_system/plugin_interface_base.h"
#include "hal_core/utilities/result.h"

namespace hal
{
    class Net;
    class Netlist;
    class Gate;

    class PLUGIN_API BooleanInfluencePlugin : public BasePluginInterface
    {
    public:
        std::string get_name() const override;
        std::string get_version() const override;

        void initialize() override;

        /**
         * Generates the Boolean influence of each input variable of a Boolean function.
         *
         * @param[in] bf - The Boolean function.
         * @param[in] num_evaluations - The amount of evaluations that are performed for each input variable.
         * @param[in] unique_identifier - A unique identifier that is applied to file names to prevent collisions during multi-threading.
         * @returns A map from the variables that appear in the function to their Boolean influence on said function on success, an error otherwise.
         */
        static Result<std::unordered_map<std::string, double>> get_boolean_influence(const BooleanFunction& bf, const u32 num_evaluations = 32000, const std::string& unique_identifier = "");

        /**
         * Generates the Boolean influence of each input variable of a Boolean function.
         *
         * @param[in] e - The z3 expression representing a Boolean function.
         * @param[in] num_evaluations - The amount of evaluations that are performed for each input variable.
         * @param[in] unique_identifier - A unique identifier that is applied to file names to prevent collisions during multi threading.
         * @returns A map from the variables that appear in the function to their Boolean influence on said function on success, an error otherwise.
         */
        static Result<std::unordered_map<std::string, double>> get_boolean_influence(const z3::expr& e, const u32 num_evaluations = 32000, const std::string& unique_identifier = "");

        /**
         * Generates the Boolean influence of each input variable of a Boolean function using the internal HAL functions only
         * The function is slower, but can be better used in multithreading enviroment.
         *
         * @param[in] bf - The Boolean function.
         * @param[in] num_evaluations - The amount of evaluations that are performed for each input variable.
         * @param[in] unique_identifier - A unique identifier that is applied to file names to prevent collisions during multi-threading.
         * @returns A map from the variables that appear in the function to their Boolean influence on said function on success, an error otherwise.
         */
        static Result<std::unordered_map<std::string, double>> get_boolean_influence_with_hal_boolean_function_class(const BooleanFunction& bf, const u32 num_evaluations);

        /**
         * Generates the Boolean influence of each input variable of a Boolean function.
         *
         * @param[in] bf - The Boolean function.
         * @param[in] unique_identifier - A unique identifier that is applied to file names to prevent collisions during multi-threading.
         * @returns A map from the variables that appear in the function to their Boolean influence on said function on success, an error otherwise.
         */
        static Result<std::unordered_map<std::string, double>> get_boolean_influence_deterministic(const BooleanFunction& bf, const std::string& unique_identifier = "");

        /**
         * Generates the Boolean influence of each input variable of a Boolean function.
         *
         * @param[in] e - The z3 expression representing a Boolean function.
         * @param[in] unique_identifier - A unique identifier that is applied to file names to prevent collisions during multi threading.
         * @returns A map from the variables that appear in the function to their Boolean influence on said function on success, an error otherwise.
         */
        static Result<std::unordered_map<std::string, double>> get_boolean_influence_deterministic(const z3::expr& e, const std::string& unique_identifier = "");

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
        static Result<std::map<Net*, double>>
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
        static Result<std::map<Net*, double>> get_boolean_influences_of_gate(const Gate* gate, const u32 num_evaluations = 32000, const std::string& unique_identifier = "");

        /**
         * Generates the function of the net using only the given gates.
         * Afterwards the generated function gets translated from a z3::expr to efficent c code, compiled, executed and evaluated.
         *
         * @param[in] gates - The gates of the subcircuit.
         * @param[in] start_net - The output net of the subcircuit at which to start the analysis.
         * @param[in] unique_identifier - A unique identifier that is applied to file names to prevent collisions during multi threading.
         * @returns A map from the nets that appear in the function of the start net to their Boolean influence on said function on success, an error otherwise.
         */
        static Result<std::map<Net*, double>> get_boolean_influences_of_subcircuit_deterministic(const std::vector<Gate*>& gates, const Net* start_net, const std::string& unique_identifier = "");

        /**
         * Generates the function of the dataport net of the given flip-flop.
         * Afterwards the generated function gets translated from a z3::expr to efficient c code, compiled, executed and evaluated.
         *
         * @param[in] gate - Pointer to the flip-flop which data input net is used to build the Boolean function.
         * @param[in] unique_identifier - A unique identifier that is applied to file names to prevent collisions during multi threading.
         * @returns A map from the nets that appear in the function of the data net to their Boolean influence on said function on success, an error otherwise.
         */
        static Result<std::map<Net*, double>> get_boolean_influences_of_gate_deterministic(const Gate* gate, const std::string& unique_identifier = "");

        /**
         * Get the FF dependency matrix of a netlist.
         *
         * @param[in] netlist - The netlist to extract the dependency matrix from.
         * @param[in] with_boolean_influence - True -- set Boolean influence, False -- sets 1.0 if connection between FFs
         * @returns A pair consisting of std::map<u32, Gate*>, which includes the mapping from the original gate
         *          IDs to the ones in the matrix, and a std::vector<std::vector<double>, which is the ff dependency matrix
         */
        static Result<std::pair<std::map<u32, Gate*>, std::vector<std::vector<double>>>> get_ff_dependency_matrix(const Netlist* netlist, bool with_boolean_influence);

    private:
        static const std::string probabilistic_function;
        static const std::string deterministic_function;

        static Result<std::unordered_map<std::string, double>>
            get_boolean_influence_internal(const z3::expr& e, const u32 num_evaluations, const bool deterministic, const std::string& unique_identifier);

        static Result<std::map<Net*, double>> get_boolean_influences_of_subcircuit_internal(const std::vector<Gate*>& gates,
                                                                                            const Net* start_net,
                                                                                            const u32 num_evaluations,
                                                                                            const bool deterministic,
                                                                                            const std::string& unique_identifier);

        static Result<std::map<Net*, double>> get_boolean_influences_of_gate_internal(const Gate* gate, const u32 num_evaluations, const bool deterministic, const std::string& unique_identifier);
    };
}    // namespace hal
