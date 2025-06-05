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

#pragma once

#include "hal_core/utilities/result.h"
#include "z3++.h"

#include <map>
#include <vector>

namespace hal
{
    class BooleanFunction;
    class Gate;
    class GatePin;
    class Net;

    namespace z3_utils
    {
        /**
         * @brief Get the z3 expression representation of a combined Boolean function of a subgraph of combinational gates.
         *
         * This function considers all gates for which the provided filter returns true, starting from the source of the specified output net.
         * The variables of the resulting Boolean function are created from the subgraph input nets using
         * `BooleanFunctionNetDecorator::get_boolean_variable`.
         *
         * @param[in] subgraph_filter A callable that returns true for gates to include in the subgraph.
         * @param[in] subgraph_output The output net of the subgraph for which to generate the Boolean function.
         * @param[in] ctx The Z3 context in which to build expressions.
         * @return On success, the z3 expression representing the combined Boolean function of the selected subgraph; otherwise, an error.
         */
        Result<z3::expr> get_subgraph_z3_function(const std::function<bool(const Gate*)> subgraph_filter, const Net* subgraph_output, z3::context& ctx);

        /**
         * @brief Get the z3 expression representation of a combined Boolean function of a subgraph of combinational gates starting at the source of the provided subgraph output net.
         *
         * The variables of the resulting Boolean function are created from the subgraph input nets using `BooleanFunctionNetDecorator::get_boolean_variable`.
         *
         * @param[in] subgraph_gates The gates making up the subgraph to consider.
         * @param[in] subgraph_output The subgraph output net for which to generate the Boolean function.
         * @param[in] ctx The Z3 context in which to build expressions.
         * @return On success, the z3 expression representing the combined Boolean function of the given subgraph; otherwise, an error.
         */
        Result<z3::expr> get_subgraph_z3_function(const std::vector<Gate*>& subgraph_gates, const Net* subgraph_output, z3::context& ctx);

        /**
         * @brief Get the z3 expression representation of a combined Boolean function of a subgraph of combinational gates filtered by property.
         *
         * Considers all gates whose type property matches the given `subgraph_property`, starting from the source of the provided output net.
         * The variables of the resulting Boolean function are created from the subgraph input nets using
         * `BooleanFunctionNetDecorator::get_boolean_variable`.
         *
         * @param[in] subgraph_property The gate type property that defines which gates to include in the subgraph.
         * @param[in] subgraph_output The subgraph output net for which to generate the Boolean function.
         * @param[in] ctx The Z3 context in which to build expressions.
         * @return On success, the z3 expression representing the combined Boolean function of the filtered subgraph; otherwise, an error.
         */
        Result<z3::expr> get_subgraph_z3_function(const GateTypeProperty subgraph_property, const Net* subgraph_output, z3::context& ctx);

        /**
         * @brief Get the z3 expression representation of a combined Boolean function of a subgraph with caching.
         *
         * This overload allows providing caches for both nets and gate-level Boolean functions to avoid
         * redundant recomputation. The variables of the resulting Boolean function are created from the subgraph input nets
         * using `BooleanFunctionNetDecorator::get_boolean_variable`.
         *
         * @param[in] subgraph_filter A callable that returns true for gates to include in the subgraph.
         * @param[in] subgraph_output The subgraph output net for which to generate the Boolean function.
         * @param[in] ctx The Z3 context in which to build expressions.
         * @param[in,out] net_cache A map (keyed by net ID) caching previously computed z3 expressions for nets.
         * @param[in,out] gate_cache A map caching previously computed BooleanFunction objects for (net ID, gate pin) pairs.
         * @return On success, the z3 expression representing the combined Boolean function; otherwise, an error.
         */
        Result<z3::expr> get_subgraph_z3_function(const std::function<bool(const Gate*)> subgraph_filter,
                                                  const Net* subgraph_output,
                                                  z3::context& ctx,
                                                  std::map<u32, z3::expr>& net_cache,
                                                  std::map<std::pair<u32, const GatePin*>, BooleanFunction>& gate_cache);

        /**
         * @brief Get the z3 expression representation of a combined Boolean function of a given subgraph of gates with caching.
         *
         * This overload uses an explicit list of gates and provided caches to avoid redundant computation. The variables
         * of the resulting Boolean function are created from the subgraph input nets using
         * `BooleanFunctionNetDecorator::get_boolean_variable`.
         *
         * @param[in] subgraph_gates The gates making up the subgraph to consider.
         * @param[in] subgraph_output The subgraph output net for which to generate the Boolean function.
         * @param[in] ctx The Z3 context in which to build expressions.
         * @param[in,out] net_cache A map (keyed by net ID) caching previously computed z3 expressions for nets.
         * @param[in,out] gate_cache A map caching previously computed BooleanFunction objects for (net ID, gate pin) pairs.
         * @return On success, the z3 expression representing the combined Boolean function; otherwise, an error.
         */
        Result<z3::expr> get_subgraph_z3_function(const std::vector<Gate*>& subgraph_gates,
                                                  const Net* subgraph_output,
                                                  z3::context& ctx,
                                                  std::map<u32, z3::expr>& net_cache,
                                                  std::map<std::pair<u32, const GatePin*>, BooleanFunction>& gate_cache);

        /**
         * @brief Get the z3 expression representation of a combined Boolean function of a filtered subgraph with caching by property.
         *
         * This overload filters gates by the given type property and uses provided caches to avoid redundant computation.
         * The variables of the resulting Boolean function are created from the subgraph input nets using
         * `BooleanFunctionNetDecorator::get_boolean_variable`.
         *
         * @param[in] subgraph_property The gate type property that defines which gates to include.
         * @param[in] subgraph_output The subgraph output net for which to generate the Boolean function.
         * @param[in] ctx The Z3 context in which to build expressions.
         * @param[in,out] net_cache A map (keyed by net ID) caching previously computed z3 expressions for nets.
         * @param[in,out] gate_cache A map caching previously computed BooleanFunction objects for (net ID, gate pin) pairs.
         * @return On success, the z3 expression representing the combined Boolean function; otherwise, an error.
         */
        Result<z3::expr> get_subgraph_z3_function(const GateTypeProperty subgraph_property,
                                                  const Net* subgraph_output,
                                                  z3::context& ctx,
                                                  std::map<u32, z3::expr>& net_cache,
                                                  std::map<std::pair<u32, const GatePin*>, BooleanFunction>& gate_cache);

        /**
         * @brief Get the z3 expression representation of a combined Boolean function of a filtered subgraph using a pre-allocated net cache.
         *
         * This overload uses a vector-based cache for net expressions to avoid redundant computation. It considers all gates
         * for which the provided filter returns true, starting from the source of the specified output net. The variables of
         * the resulting Boolean function are created from the subgraph input nets using `BooleanFunctionNetDecorator::get_boolean_variable`.
         *
         * @warning The `net_cache` vector must be pre-allocated with size at least `max_net_id + 1` and filled with empty z3::expr, where `max_net_id` is the
         *   highest net ID in the design. Each index in the vector corresponds to a net ID, and the cache relies on direct
         *   indexing by net ID.
         *
         * @param[in] subgraph_filter A callable that returns true for gates to include in the subgraph.
         * @param[in] subgraph_output The subgraph output net for which to generate the Boolean function.
         * @param[in] ctx The Z3 context in which to build expressions.
         * @param[in,out] net_cache A vector caching previously computed z3 expressions for nets, indexed by net ID.
         * @return On success, the z3 expression representing the combined Boolean function; otherwise, an error.
         */
        Result<z3::expr> get_subgraph_z3_function(const std::function<bool(const Gate*)> subgraph_filter, const Net* subgraph_output, z3::context& ctx, std::vector<z3::expr>& net_cache);

        /**
         * @brief Get the z3 expression representation of a combined Boolean function of a given subgraph using a pre-allocated net cache.
         *
         * This overload uses a vector-based cache for net expressions to avoid redundant computation. It combines the Boolean
         * functions of the explicitly provided list of gates, starting at the source of the given output net. The variables
         * of the resulting Boolean function are created from the subgraph input nets using
         * `BooleanFunctionNetDecorator::get_boolean_variable`.
         *
         * @warning The `net_cache` vector must be pre-allocated with size at least `max_net_id + 1` and filled with empty z3::expr, where `max_net_id` is the
         *   highest net ID in the design. Each index in the vector corresponds to a net ID, and the cache relies on direct
         *   indexing by net ID.
         *
         * @param[in] subgraph_gates The gates making up the subgraph to consider.
         * @param[in] subgraph_output The subgraph output net for which to generate the Boolean function.
         * @param[in] ctx The Z3 context in which to build expressions.
         * @param[in,out] net_cache A vector caching previously computed z3 expressions for nets, indexed by net ID.
         * @return On success, the z3 expression representing the combined Boolean function; otherwise, an error.
         */
        Result<z3::expr> get_subgraph_z3_function(const std::vector<Gate*>& subgraph_gates, const Net* subgraph_output, z3::context& ctx, std::vector<z3::expr>& net_cache);

        /**
         * @brief Get the z3 expression representation of a combined Boolean function of a subgraph filtered by property using a pre-allocated net cache.
         *
         * This overload filters gates by the given type property and uses a vector-based cache for net expressions to avoid
         * redundant computation. The variables of the resulting Boolean function are created from the subgraph input nets
         * using `BooleanFunctionNetDecorator::get_boolean_variable`.
         *
         * @warning The `net_cache` vector must be pre-allocated with size at least `max_net_id + 1` and filled with empty z3::expr, where `max_net_id` is the
         *   highest net ID in the design. Each index in the vector corresponds to a net ID, and the cache relies on direct
         *   indexing by net ID.
         *
         * @param[in] subgraph_property The gate type property that defines which gates to include.
         * @param[in] subgraph_output The subgraph output net for which to generate the Boolean function.
         * @param[in] ctx The Z3 context in which to build expressions.
         * @param[in,out] net_cache A vector caching previously computed z3 expressions for nets, indexed by net ID.
         * @return On success, the z3 expression representing the combined Boolean function; otherwise, an error.
         */
        Result<z3::expr> get_subgraph_z3_function(const GateTypeProperty subgraph_property, const Net* subgraph_output, z3::context& ctx, std::vector<z3::expr>& net_cache);

        /**
         * @brief Get the z3 expressions for a vector of combined Boolean functions of a filtered subgraph.
         *
         * This overload computes one z3 expression per output net in `subgraph_outputs`, considering all gates for which
         * the provided filter returns true. The variables of each resulting Boolean function are created from the subgraph
         * input nets using `BooleanFunctionNetDecorator::get_boolean_variable`.
         *
         * @param[in] subgraph_filter A callable that returns true for gates to include in the subgraph.
         * @param[in] subgraph_outputs The list of subgraph output nets for which to generate Boolean functions.
         * @param[in] ctx The Z3 context in which to build expressions.
         * @return On success, a vector of z3 expressions representing each combined Boolean function; otherwise, an error.
         */
        Result<std::vector<z3::expr>> get_subgraph_z3_functions(const std::function<bool(const Gate*)> subgraph_filter, const std::vector<Net*>& subgraph_outputs, z3::context& ctx);

        /**
         * @brief Get the z3 expressions for a vector of combined Boolean functions of a specified subgraph.
         *
         * This overload takes an explicit list of gates and computes one z3 expression per output net in
         * `subgraph_outputs`. The variables of each resulting Boolean function are created from the subgraph input nets
         * using `BooleanFunctionNetDecorator::get_boolean_variable`.
         *
         * @param[in] subgraph_gates The gates making up the subgraph to consider.
         * @param[in] subgraph_outputs The list of subgraph output nets for which to generate Boolean functions.
         * @param[in] ctx The Z3 context in which to build expressions.
         * @return On success, a vector of z3 expressions representing each combined Boolean function; otherwise, an error.
         */
        Result<std::vector<z3::expr>> get_subgraph_z3_functions(const std::vector<Gate*>& subgraph_gates, const std::vector<Net*>& subgraph_outputs, z3::context& ctx);

        /**
         * @brief Get the z3 expressions for a vector of combined Boolean functions of a subgraph filtered by property.
         *
         * This overload filters gates by the given type property and computes one z3 expression per output net in
         * `subgraph_outputs`. The variables of each resulting Boolean function are created from the subgraph input nets
         * using `BooleanFunctionNetDecorator::get_boolean_variable`.
         *
         * @param[in] subgraph_property The gate type property that defines which gates to include.
         * @param[in] subgraph_outputs The list of subgraph output nets for which to generate Boolean functions.
         * @param[in] ctx The Z3 context in which to build expressions.
         * @return On success, a vector of z3 expressions representing each combined Boolean function; otherwise, an error.
         */
        Result<std::vector<z3::expr>> get_subgraph_z3_functions(const GateTypeProperty subgraph_property, const std::vector<Net*>& subgraph_outputs, z3::context& ctx);

        /**
         * @brief Get the z3 expressions for a vector of combined Boolean functions of a filtered subgraph with caching.
         *
         * This overload computes one z3 expression per output net in `subgraph_outputs`, considering all gates for which
         * the provided filter returns true. Provided caches avoid redundant computation. The variables of each resulting
         * Boolean function are created from the subgraph input nets using
         * `BooleanFunctionNetDecorator::get_boolean_variable`.
         *
         * @param[in] subgraph_filter A callable that returns true for gates to include in the subgraph.
         * @param[in] subgraph_outputs The list of subgraph output nets for which to generate Boolean functions.
         * @param[in] ctx The Z3 context in which to build expressions.
         * @param[in,out] net_cache A map (keyed by net ID) caching previously computed z3 expressions for nets.
         * @param[in,out] gate_cache A map caching previously computed BooleanFunction objects for (net ID, gate pin) pairs.
         * @return On success, a vector of z3 expressions representing each combined Boolean function; otherwise, an error.
         */
        Result<std::vector<z3::expr>> get_subgraph_z3_functions(const std::function<bool(const Gate*)> subgraph_filter,
                                                                const std::vector<Net*>& subgraph_outputs,
                                                                z3::context& ctx,
                                                                std::map<u32, z3::expr>& net_cache,
                                                                std::map<std::pair<u32, const GatePin*>, BooleanFunction>& gate_cache);

        /**
         * @brief Get the z3 expressions for a vector of combined Boolean functions of a specified subgraph with caching.
         *
         * This overload takes an explicit list of gates, computes one z3 expression per output net in `subgraph_outputs`,
         * and uses provided caches to avoid redundant computation. The variables of each resulting Boolean function are
         * created from the subgraph input nets using `BooleanFunctionNetDecorator::get_boolean_variable`.
         *
         * @param[in] subgraph_gates The gates making up the subgraph to consider.
         * @param[in] subgraph_outputs The list of subgraph output nets for which to generate Boolean functions.
         * @param[in] ctx The Z3 context in which to build expressions.
         * @param[in,out] net_cache A map (keyed by net ID) caching previously computed z3 expressions for nets.
         * @param[in,out] gate_cache A map caching previously computed BooleanFunction objects for (net ID, gate pin) pairs.
         * @return On success, a vector of z3 expressions representing each combined Boolean function; otherwise, an error.
         */
        Result<std::vector<z3::expr>> get_subgraph_z3_functions(const std::vector<Gate*>& subgraph_gates,
                                                                const std::vector<Net*>& subgraph_outputs,
                                                                z3::context& ctx,
                                                                std::map<u32, z3::expr>& net_cache,
                                                                std::map<std::pair<u32, const GatePin*>, BooleanFunction>& gate_cache);

        /**
         * @brief Get the z3 expressions for a vector of combined Boolean functions of a subgraph filtered by property with caching.
         *
         * This overload filters gates by the given type property, computes one z3 expression per output net in
         * `subgraph_outputs`, and uses provided caches to avoid redundant computation. The variables of each resulting
         * Boolean function are created from the subgraph input nets using
         * `BooleanFunctionNetDecorator::get_boolean_variable`.
         *
         * @param[in] subgraph_property The gate type property that defines which gates to include.
         * @param[in] subgraph_outputs The list of subgraph output nets for which to generate Boolean functions.
         * @param[in] ctx The Z3 context in which to build expressions.
         * @param[in,out] net_cache A map (keyed by net ID) caching previously computed z3 expressions for nets.
         * @param[in,out] gate_cache A map caching previously computed BooleanFunction objects for (net ID, gate pin) pairs.
         * @return On success, a vector of z3 expressions representing each combined Boolean function; otherwise, an error.
         */
        Result<std::vector<z3::expr>> get_subgraph_z3_functions(const GateTypeProperty subgraph_property,
                                                                const std::vector<Net*>& subgraph_outputs,
                                                                z3::context& ctx,
                                                                std::map<u32, z3::expr>& net_cache,
                                                                std::map<std::pair<u32, const GatePin*>, BooleanFunction>& gate_cache);

        /**
         * @brief Get the z3 expressions for a vector of combined Boolean functions of a filtered subgraph using a pre-allocated net cache.
         *
         * This overload computes one z3 expression per output net in `subgraph_outputs`, considering all gates for which
         * the provided filter returns true. It uses a vector-based cache for net expressions to avoid redundant computation.
         * The variables of each resulting Boolean function are created from the subgraph input nets using
         * `BooleanFunctionNetDecorator::get_boolean_variable`.
         *
         * @warning The `net_cache` vector must be pre-allocated with size at least `max_net_id + 1` and filled with empty z3::expr, where `max_net_id` is the
         *   highest net ID in the design. Each index in the vector corresponds to a net ID, and the cache relies on direct
         *   indexing by net ID.
         *
         * @param[in] subgraph_filter A callable that returns true for gates to include in the subgraph.
         * @param[in] subgraph_outputs The list of subgraph output nets for which to generate Boolean functions.
         * @param[in] ctx The Z3 context in which to build expressions.
         * @param[in,out] net_cache A vector caching previously computed z3 expressions for nets, indexed by net ID.
         * @return On success, a vector of z3 expressions representing each combined Boolean function; otherwise, an error.
         */
        Result<std::vector<z3::expr>>
            get_subgraph_z3_functions(const std::function<bool(const Gate*)> subgraph_filter, const std::vector<Net*>& subgraph_outputs, z3::context& ctx, std::vector<z3::expr>& net_cache);

        /**
         * @brief Get the z3 expressions for a vector of combined Boolean functions of a specified subgraph using a pre-allocated net cache.
         *
         * This overload takes an explicit list of gates, computes one z3 expression per output net in `subgraph_outputs`,
         * and uses a vector-based cache for net expressions to avoid redundant computation. The variables of each resulting
         * Boolean function are created from the subgraph input nets using `BooleanFunctionNetDecorator::get_boolean_variable`.
         *
         * @warning The `net_cache` vector must be pre-allocated with size at least `max_net_id + 1` and filled with empty z3::expr, where `max_net_id` is the
         *   highest net ID in the design. Each index in the vector corresponds to a net ID, and the cache relies on direct
         *   indexing by net ID.
         *
         * @param[in] subgraph_gates The gates making up the subgraph to consider.
         * @param[in] subgraph_outputs The list of subgraph output nets for which to generate Boolean functions.
         * @param[in] ctx The Z3 context in which to build expressions.
         * @param[in,out] net_cache A vector caching previously computed z3 expressions for nets, indexed by net ID.
         * @return On success, a vector of z3 expressions representing each combined Boolean function; otherwise, an error.
         */
        Result<std::vector<z3::expr>>
            get_subgraph_z3_functions(const std::vector<Gate*>& subgraph_gates, const std::vector<Net*>& subgraph_outputs, z3::context& ctx, std::vector<z3::expr>& net_cache);

        /**
         * @brief Get the z3 expressions for a vector of combined Boolean functions of a subgraph filtered by property using a pre-allocated net cache.
         *
         * This overload filters gates by the given type property, computes one z3 expression per output net in
         * `subgraph_outputs`, and uses a vector-based cache for net expressions to avoid redundant computation.
         * The variables of each resulting Boolean function are created from the subgraph input nets using
         * `BooleanFunctionNetDecorator::get_boolean_variable`.
         *
         * @warning The `net_cache` vector must be pre-allocated with size at least `max_net_id + 1` and filled with empty z3::expr, where `max_net_id` is the
         *   highest net ID in the design. Each index in the vector corresponds to a net ID, and the cache relies on direct
         *   indexing by net ID.
         *
         * @param[in] subgraph_property The gate type property that defines which gates to include.
         * @param[in] subgraph_outputs The list of subgraph output nets for which to generate Boolean functions.
         * @param[in] ctx The Z3 context in which to build expressions.
         * @param[in,out] net_cache A vector caching previously computed z3 expressions for nets, indexed by net ID.
         * @return On success, a vector of z3 expressions representing each combined Boolean function; otherwise, an error.
         */
        Result<std::vector<z3::expr>>
            get_subgraph_z3_functions(const GateTypeProperty subgraph_property, const std::vector<Net*>& subgraph_outputs, z3::context& ctx, std::vector<z3::expr>& net_cache);
    }    // namespace z3_utils
}    // namespace hal
