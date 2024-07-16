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
 * @file candidate_context.h
 * @brief This file contains the CandidateContext struct and that is used for optimization purposes during the module identification and generation of functional candidates.
 */

#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/decorators/boolean_function_decorator.h"
#include "hal_core/netlist/decorators/boolean_function_net_decorator.h"
#include "hal_core/netlist/decorators/subgraph_netlist_decorator.h"
#include "hal_core/utilities/result.h"
#include "hal_core/utilities/log.h"

#include <vector>
#include <map>

// #define Z3_CANDIDATE_CONTEXT
#define HAL_CANDIDATE_CONTEXT

#ifdef Z3_CANDIDATE_CONTEXT
#include "z3_utils.h"
#endif

namespace hal
{
    class Gate;

    namespace module_identification
    {
        /**
         * @struct CandidateContext
         * @brief This struct manages the context of a candidate during module identification, including caches for all Boolean function related and expensive oprations that are populated during the functional candidate generation.
         */
        struct CandidateContext
        {
            /**
             * @brief Constructs a new CandidateContext object saving results for one structural candidate.
             * 
             * @param[in] nl - The netlist associated with the candidate context.
             * @param[in] gates - The gates of the structural canidate.
             */
            CandidateContext(const Netlist* nl, const std::vector<Gate*>& gates);

#ifdef HAL_CANDIDATE_CONTEXT
            /**
             * @brief Populates the boolean function cache for a set of nets.
             * 
             * @param[in] nets - The nets to populate the boolean function cache for.
             * @returns OK() incase of success, an error otherwise.
             */
            hal::Result<std::monostate> populate_boolean_function_cache(const std::vector<Net*> nets);
            
            /**
             * @brief Retrieves a constant boolean function for a given net and control mapping.
             * 
             * @param[in] n - The net to retrieve the boolean function for.
             * @param[in] ctrl_mapping - The control mapping to apply to the function.
             * @returns OK() and the boolean function or indicating failure.
             */
            hal::Result<const BooleanFunction> get_boolean_function_const(const Net* n, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping) const;

            /**
             * @brief Retrieves a set of constant boolean functions for a given set of nets and control mapping.
             * 
             * @param[in] nets - The nets to retrieve the boolean functions for.
             * @param[in] ctrl_mapping - The control mapping to apply to the function.
             * @returns OK() and a vector of boolean functions or indicating failure.
             */
            hal::Result<std::vector<BooleanFunction>> get_boolean_functions_const(const std::vector<Net*> nets, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping) const;

            /**
             * @brief Retrieves a boolean function for a given net and control mapping.
             * 
             * @param[in] n - The net to retrieve the boolean function for.
             * @param[in] ctrl_mapping - The control mapping to apply to the function.
             * @returns OK() and the boolean function incase of success, an error otherwise.
             */
            hal::Result<const BooleanFunction> get_boolean_function(const Net* n, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping);

            /**
             * @brief Retrieves a set of boolean functions for a given set of nets and control mapping.
             * 
             * @param[in] nets - The nets to retrieve the boolean functions for.
             * @param[in] ctrl_mapping - The control mapping to apply to the function.
             * @returns OK() and a vector of boolean functions incase of success, an error otherwise.
             */
            hal::Result<std::vector<BooleanFunction>> get_boolean_functions(const std::vector<Net*> nets, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping);

            /**
             * @brief Retrieves the variable names for a given net and control mapping.
             * 
             * @param[in] n - The net to retrieve the variable names for.
             * @param[in] ctrl_mapping - The control mapping to apply to the function.
             * @returns OK() and a set of variable names incase of success, an error otherwise.
             */
            hal::Result<const std::set<std::string>> get_variable_names(const Net* n, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping);

            /**
             * @brief Retrieves the variable nets for a given net and control mapping.
             * 
             * @param[in] n - The net to retrieve the variable nets for.
             * @param[in] ctrl_mapping - The control mapping to apply to the function.
             * @returns OK() and a set of variable nets incase of success, an error otherwise.
             */
            hal::Result<const std::set<Net*>> get_variable_nets(const Net* n, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping);

            /**
             * @brief Retrieves the boolean influence for a given net and control mapping.
             * 
             * @param[in] n - The net to retrieve the boolean influence for.
             * @param[in] ctrl_mapping - The control mapping to apply to the function.
             * @returns OK() and a map of variable names to influence values incase of success, an error otherwise.
             */
            hal::Result<std::unordered_map<std::string, double>> get_boolean_influence(const Net* n, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping);

            /**
             * @brief Evaluates the boolean function for a given net, control mapping, and evaluation mapping.
             * 
             * @param[in] n - The net to evaluate the boolean function for.
             * @param[in] ctrl_mapping - The control mapping to apply to the function.
             * @param[in] eval_mapping - The evaluation mapping for the boolean function.
             * @returns OK() and a vector of boolean function values incase of success, an error otherwise.
             */
            hal::Result<std::vector<BooleanFunction::Value>> evaluate(const Net* n, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping, const std::map<std::string, BooleanFunction::Value>& eval_mapping);

            /**
             * @brief The gates of the corresponding structural candidate.
             */
            std::vector<Gate*> m_gates;

        private:
            /**
             * @brief The netlist associated with the candidate context.
             */
            const Netlist* m_netlist;

            /**
             * @brief Cache for boolean functions.
             */
            std::map<std::pair<const Net*, std::map<Net*, BooleanFunction::Value>>, BooleanFunction> m_boolean_function_cache;

            /**
             * @brief Cache for boolean variable names.
             */
            std::map<std::pair<const Net*, std::map<Net*, BooleanFunction::Value>>, std::set<std::string>> m_boolean_vars_cache;

            /**
             * @brief Cache for boolean influence values.
             */
            std::map<std::pair<const Net*, std::map<Net*, BooleanFunction::Value>>, std::unordered_map<std::string, double>> m_boolean_influence_cache;
#endif

#ifdef Z3_CANDIDATE_CONTEXT
            /**
             * @brief Populates the z3 expression cache for a set of nets.
             * 
             * @param[in] nets - The nets to populate the z3 expression cache for.
             * @returns OK() incase of success, an error otherwise.
             */
            hal::Result<std::monostate> populate_boolean_function_cache(const std::vector<Net*> nets);

            /**
             * @brief Retrieves a z3 expression for a given net and control mapping.
             * 
             * @param[in] n - The net to retrieve the z3 expression for.
             * @param[in] ctrl_mapping - The control mapping to apply to the function.
             * @returns OK() and the z3 expression, an error otherwise.
             */
            hal::Result<z3::expr> get_boolean_function(const Net* n, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping);

            /**
             * @brief Retrieves a constant z3 expression for a given net and control mapping.
             * 
             * @param[in] n - The net to retrieve the z3 expression for.
             * @param[in] ctrl_mapping - The control mapping to apply to the function.
             * @returns OK() and the z3 expression, an error otherwise.
             */
            hal::Result<z3::expr> get_boolean_function_const(const Net* n, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping) const;

            /**
             * @brief Retrieves a HAL boolean function for a given net and control mapping.
             * 
             * @param[in] n - The net to retrieve the HAL boolean function for.
             * @param[in] ctrl_mapping - The control mapping to apply to the function.
             * @returns OK() and the boolean function, an error otherwise.
             */
            hal::Result<BooleanFunction> get_hal_boolean_function(const Net* n, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping);

            /**
             * @brief Retrieves a constant HAL boolean function for a given net and control mapping.
             * 
             * @param[in] n - The net to retrieve the HAL boolean function for.
             * @param[in] ctrl_mapping - The control mapping to apply to the function.
             * @returns OK() and the boolean function, an error otherwise.
             */
            hal::Result<BooleanFunction> get_hal_boolean_function_const(const Net* n, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping);

            /**
             * @brief Retrieves a set of z3 expressions for a given set of nets and control mapping.
             * 
             * @param[in] nets - The nets to retrieve the z3 expressions for.
             * @param[in] ctrl_mapping - The control mapping to apply to the function.
             * @returns OK() and a vector of z3 expressions, an error otherwise.
             */
            hal::Result<std::vector<BooleanFunction>> get_boolean_functions(const std::vector<Net*>& nets, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping);

            /**
             * @brief Retrieves a set of constant z3 expressions for a given set of nets and control mapping.
             * 
             * @param[in] nets - The nets to retrieve the z3 expressions for.
             * @param[in] ctrl_mapping - The control mapping to apply to the function.
             * @returns OK() and a vector of z3 expressions, an error otherwise.
             */
            hal::Result<std::vector<BooleanFunction>> get_boolean_functions_const(const std::vector<Net*>& nets, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping);

            /**
             * @brief Retrieves the variable names for a given net and control mapping.
             * 
             * @param[in] n - The net to retrieve the variable names for.
             * @param[in] ctrl_mapping - The control mapping to apply to the function.
             * @returns OK() and a set of variable names, an error otherwise.
             */
            hal::Result<std::set<std::string>> get_variable_names(const Net* n, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping);

            /**
             * @brief Retrieves the variable nets for a given net and control mapping.
             * 
             * @param[in] n - The net to retrieve the variable nets for.
             * @param[in] ctrl_mapping - The control mapping to apply to the function.
             * @returns OK() and a set of variable nets, an error otherwise.
             */
            hal::Result<std::set<Net*>> get_variable_nets(const Net* n, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping);

            /**
             * @brief Retrieves the boolean influence for a given net and control mapping.
             * 
             * @param[in] n - The net to retrieve the boolean influence for.
             * @param[in] ctrl_mapping - The control mapping to apply to the function.
             * @returns OK() and a map of variable names to influence values, an error otherwise.
             */
            hal::Result<std::unordered_map<std::string, double>> get_boolean_influence(const Net* n, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping);

            /**
             * @brief Evaluates the boolean function for a given net, control mapping, and evaluation mapping.
             * 
             * @param[in] n - The net to evaluate the boolean function for.
             * @param[in] ctrl_mapping - The control mapping to apply to the function.
             * @param[in] eval_mapping - The evaluation mapping for the boolean function.
             * @returns OK() and a vector of boolean function values, an error otherwise.
             */
            hal::Result<std::vector<BooleanFunction::Value>> evaluate(const Net* n, const std::map<Net*, BooleanFunction::Value>& ctrl_mapping, const std::map<std::string, BooleanFunction::Value>& eval_mapping);

            /**
             * @brief The gates involved in the candidate context.
             */
            std::vector<Gate*> m_gates;

            /**
             * @brief Z3 context for boolean function evaluation.
             */
            z3::context m_ctx;

        private:
            /**
             * @brief The netlist associated with the candidate context.
             */
            const Netlist* m_netlist;

            /**
             * @brief Cache for HAL boolean functions.
             */
            std::map<std::pair<const Net*, std::map<Net*, BooleanFunction::Value>>, BooleanFunction> m_boolean_function_hal_cache;

            /**
             * @brief Cache for Z3 boolean functions.
             */
            std::map<std::pair<const Net*, std::map<Net*, BooleanFunction::Value>>, z3::expr> m_boolean_function_cache;

            /**
             * @brief Cache for boolean variable names.
             */
            std::map<std::pair<const Net*, std::map<Net*, BooleanFunction::Value>>, std::set<std::string>> m_boolean_vars_cache;

            /**
             * @brief Cache for boolean influence values.
             */
            std::map<std::pair<const Net*, std::map<Net*, BooleanFunction::Value>>, std::unordered_map<std::string, double>> m_boolean_influence_cache;
#endif
        };
    }    // namespace module_identification
}    // namespace hal
