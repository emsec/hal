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

#include "hal_core/netlist/decorators/subgraph_function_cache.h"
#include "hal_core/defines.h"
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/result.h"

#include <functional>

namespace hal
{
    /**
     * A netlist decorator that operates on an existing subgraph of the associated netlist to, e.g., copy the subgraph as a new netlist object or compute a Boolean function describing the subgraph.
     *
     * @ingroup decorators
     */
    class NETLIST_API SubgraphNetlistDecorator
    {
    public:
        /**
         * Construct new SubgraphNetlistDecorator object.
         * 
         * @param[in] netlist - The netlist to operate on.
         */
        SubgraphNetlistDecorator(const Netlist& netlist);

        /**
         * Get a deep copy of a netlist subgraph including all of its gates and nets, but excluding modules and groupings.
         * 
         * @param[in] subgraph_gates - The gates making up the subgraph that shall be copied from the netlist.
         * @param[in] all_global_io - Set `true` to mark all nets as global input or output that lost at least one source or destination in the copied netlist, `false` to only mark them if all sources or destinations were removed. Global inputs and outputs of the parent netlist will always also be annotated as global inputs or outputs. Defaults to `false`.
         * @return The copied subgraph netlist on success, an error otherwise.
         */
        Result<std::unique_ptr<Netlist>> copy_subgraph_netlist(const std::vector<const Gate*>& subgraph_gates, const bool all_global_io = false) const;

        /**
         * Get a deep copy of a netlist subgraph including all of its gates and nets, but excluding modules and groupings.
         * 
         * @param[in] subgraph_gates - The gates making up the subgraph that shall be copied from the netlist.
         * @param[in] all_global_io - Set `true` to mark all nets as global input or output that lost at least one source or destination in the copied netlist, `false` to only mark them if all sources or destinations were removed. Global inputs and outputs of the parent netlist will always also be annotated as global inputs or outputs. Defaults to `false`.
         * @return The copied subgraph netlist on success, an error otherwise.
         */
        Result<std::unique_ptr<Netlist>> copy_subgraph_netlist(const std::vector<Gate*>& subgraph_gates, const bool all_global_io = false) const;

        /**
         * Get a deep copy of a netlist subgraph including all of its gates and nets, but excluding modules and groupings.
         * 
         * @param[in] subgraph_module - The module making up the subgraph that shall be copied from the netlist, including the gates of all of its submodules.
         * @param[in] all_global_io - Set `true` to mark all nets as global input or output that lost at least one source or destination in the copied netlist, `false` to only mark them if all sources or destinations were removed. Global inputs and outputs of the parent netlist will always also be annotated as global inputs or outputs. Defaults to `false`.
         * @return The copied subgraph netlist on success, an error otherwise.
         */
        Result<std::unique_ptr<Netlist>> copy_subgraph_netlist(const Module* subgraph_module, const bool all_global_io = false) const;

        /**
         * Get the combined Boolean function of a subgraph starting at the source of the provided subgraph output net.
         * The function is composed backwards through the gates of the subgraph. A gate is composed into it only if its gate type is combinational
         * and it has a Boolean function for the output pin that the walk reaches it through. At any other gate, at a gate outside of the subgraph,
         * at a net without a source, and at a global input net, the walk stops and the net becomes a variable of the function. Hence, flip-flops, memories, and gates
         * without a Boolean function bound the subgraph function even if they are part of the `subgraph_gates`.
         * If the Boolean function of a gate cannot be built in terms of nets, e.g., because one of its input pins has no net or a gate parameter it depends on has no value, a warning is logged and the walk stops at that gate as well.
         * The variables of the resulting Boolean function are created from the subgraph input nets using `BooleanFunctionNetDecorator::get_boolean_variable`.
         * For repeated calls on the same subgraph, create a cache with `make_subgraph_function_cache` and pass it to `get_subgraph_function` instead.
         *
         * @param[in] subgraph_gates - The gates making up the subgraph to consider.
         * @param[in] subgraph_output - The subgraph output net for which to generate the Boolean function.
         * @return The combined Boolean function of the subgraph on success, an error otherwise.
         */
        Result<BooleanFunction> get_subgraph_function(const std::vector<const Gate*>& subgraph_gates, const Net* subgraph_output) const;

        /**
         * Get the combined Boolean function of a subgraph starting at the source of the provided subgraph output net.
         * The function is composed backwards through the gates of the subgraph. A gate is composed into it only if its gate type is combinational
         * and it has a Boolean function for the output pin that the walk reaches it through. At any other gate, at a gate outside of the subgraph,
         * at a net without a source, and at a global input net, the walk stops and the net becomes a variable of the function. Hence, flip-flops, memories, and gates
         * without a Boolean function bound the subgraph function even if they are part of the `subgraph_gates`.
         * If the Boolean function of a gate cannot be built in terms of nets, e.g., because one of its input pins has no net or a gate parameter it depends on has no value, a warning is logged and the walk stops at that gate as well.
         * The variables of the resulting Boolean function are created from the subgraph input nets using `BooleanFunctionNetDecorator::get_boolean_variable`.
         * For repeated calls on the same subgraph, create a cache with `make_subgraph_function_cache` and pass it to `get_subgraph_function` instead.
         *
         * @param[in] subgraph_gates - The gates making up the subgraph to consider.
         * @param[in] subgraph_output - The subgraph output net for which to generate the Boolean function.
         * @return The combined Boolean function of the subgraph on success, an error otherwise.
         */
        Result<BooleanFunction> get_subgraph_function(const std::vector<Gate*>& subgraph_gates, const Net* subgraph_output) const;

        /**
         * Get the combined Boolean function of a subgraph starting at the source of the provided subgraph output net.
         * The function is composed backwards through the gates of the subgraph. A gate is composed into it only if its gate type is combinational
         * and it has a Boolean function for the output pin that the walk reaches it through. At any other gate, at a gate outside of the subgraph,
         * at a net without a source, and at a global input net, the walk stops and the net becomes a variable of the function. Hence, flip-flops, memories, and gates
         * without a Boolean function bound the subgraph function even if they are part of the `subgraph_module`.
         * If the Boolean function of a gate cannot be built in terms of nets, e.g., because one of its input pins has no net or a gate parameter it depends on has no value, a warning is logged and the walk stops at that gate as well.
         * The variables of the resulting Boolean function are created from the subgraph input nets using `BooleanFunctionNetDecorator::get_boolean_variable`.
         * For repeated calls on the same subgraph, create a cache with `make_subgraph_function_cache` and pass it to `get_subgraph_function` instead.
         *
         * @param[in] subgraph_module - The module making up the subgraph to consider, including the gates of all of its submodules.
         * @param[in] subgraph_output - The subgraph output net for which to generate the Boolean function.
         * @return The combined Boolean function of the subgraph on success, an error otherwise.
         */
        Result<BooleanFunction> get_subgraph_function(const Module* subgraph_module, const Net* subgraph_output) const;

        /**
         * Get the combined Boolean function of a subgraph starting at the source of the provided subgraph output net.
         * The subgraph consists of all gates of the netlist for which the `subgraph_filter` evaluates to `true`.
         * The walk stops at gates as described for `get_subgraph_function(const Net*, SubgraphFunctionCache&)`.
         * The variables of the resulting Boolean function are created from the subgraph input nets using `BooleanFunctionNetDecorator::get_boolean_variable`.
         * For repeated calls on the same subgraph, create a cache with `make_subgraph_function_cache` and pass it to `get_subgraph_function` instead.
         *
         * @param[in] subgraph_filter - Filter condition a gate has to meet to be part of the subgraph.
         * @param[in] subgraph_output - The subgraph output net for which to generate the Boolean function.
         * @return The combined Boolean function of the subgraph on success, an error otherwise.
         */
        Result<BooleanFunction> get_subgraph_function(const std::function<bool(const Gate*)>& subgraph_filter, const Net* subgraph_output) const;

        /**
         * Get the combined Boolean function of a subgraph starting at the source of the provided subgraph output net.
         * The subgraph consists of all gates of the netlist whose gate type has the given property.
         * The walk stops at gates as described for `get_subgraph_function(const Net*, SubgraphFunctionCache&)`.
         * The variables of the resulting Boolean function are created from the subgraph input nets using `BooleanFunctionNetDecorator::get_boolean_variable`.
         * For repeated calls on the same subgraph, create a cache with `make_subgraph_function_cache` and pass it to `get_subgraph_function` instead.
         *
         * @param[in] subgraph_property - The gate type property a gate has to have to be part of the subgraph.
         * @param[in] subgraph_output - The subgraph output net for which to generate the Boolean function.
         * @return The combined Boolean function of the subgraph on success, an error otherwise.
         */
        Result<BooleanFunction> get_subgraph_function(const GateTypeProperty subgraph_property, const Net* subgraph_output) const;

        /**
         * Create a cache for one specific subgraph, to be passed to `get_subgraph_function` or `get_subgraph_function_inputs` in place of the subgraph.
         * The cache shares the Boolean functions and inputs computed for the gate outputs of the subgraph across calls, so that logic which several
         * subgraph output nets have in common is traversed only once.
         * The cache belongs to the netlist of this decorator and must be dropped when the netlist is modified.
         *
         * @param[in] subgraph_gates - The gates making up the subgraph to consider.
         * @return The cache on success, an error otherwise.
         */
        Result<SubgraphFunctionCache> make_subgraph_function_cache(const std::vector<const Gate*>& subgraph_gates) const;

        /**
         * Create a cache for one specific subgraph, to be passed to `get_subgraph_function` or `get_subgraph_function_inputs` in place of the subgraph.
         * The cache shares the Boolean functions and inputs computed for the gate outputs of the subgraph across calls, so that logic which several
         * subgraph output nets have in common is traversed only once.
         * The cache belongs to the netlist of this decorator and must be dropped when the netlist is modified.
         *
         * @param[in] subgraph_gates - The gates making up the subgraph to consider.
         * @return The cache on success, an error otherwise.
         */
        Result<SubgraphFunctionCache> make_subgraph_function_cache(const std::vector<Gate*>& subgraph_gates) const;

        /**
         * Create a cache for one specific subgraph, to be passed to `get_subgraph_function` or `get_subgraph_function_inputs` in place of the subgraph.
         * The cache shares the Boolean functions and inputs computed for the gate outputs of the subgraph across calls, so that logic which several
         * subgraph output nets have in common is traversed only once.
         * The cache belongs to the netlist of this decorator and must be dropped when the netlist is modified.
         *
         * @param[in] subgraph_module - The module making up the subgraph to consider, including the gates of all of its submodules.
         * @return The cache on success, an error otherwise.
         */
        Result<SubgraphFunctionCache> make_subgraph_function_cache(const Module* subgraph_module) const;

        /**
         * Create a cache for one specific subgraph, to be passed to `get_subgraph_function` or `get_subgraph_function_inputs` in place of the subgraph.
         * The cache shares the Boolean functions and inputs computed for the gate outputs of the subgraph across calls, so that logic which several
         * subgraph output nets have in common is traversed only once.
         * The cache belongs to the netlist of this decorator and must be dropped when the netlist is modified.
         *
         * @param[in] subgraph_filter - Filter condition a gate of the netlist has to meet to be part of the subgraph. Evaluated once per gate when the cache is created.
         * @return The cache on success, an error otherwise.
         */
        Result<SubgraphFunctionCache> make_subgraph_function_cache(const std::function<bool(const Gate*)>& subgraph_filter) const;

        /**
         * Create a cache for one specific subgraph, to be passed to `get_subgraph_function` or `get_subgraph_function_inputs` in place of the subgraph.
         * The cache shares the Boolean functions and inputs computed for the gate outputs of the subgraph across calls, so that logic which several
         * subgraph output nets have in common is traversed only once.
         * The cache belongs to the netlist of this decorator and must be dropped when the netlist is modified.
         *
         * @param[in] subgraph_property - The gate type property a gate of the netlist has to have to be part of the subgraph.
         * @return The cache on success, an error otherwise.
         */
        Result<SubgraphFunctionCache> make_subgraph_function_cache(const GateTypeProperty subgraph_property) const;

        /**
         * Get the combined Boolean function of the subgraph of the cache starting at the source of the provided subgraph output net.
         * The function is composed backwards through the gates of the subgraph. A gate is composed into it only if its gate type is combinational
         * and it has a Boolean function for the output pin that the walk reaches it through. At any other gate, at a gate outside of the subgraph,
         * at a net without a source, and at a global input net, the walk stops and the net becomes a variable of the function. Hence, flip-flops, memories, and gates
         * without a Boolean function bound the subgraph function even if they are part of the subgraph of the `cache`.
         * If the Boolean function of a gate cannot be built in terms of nets, e.g., because one of its input pins has no net or a gate parameter it depends on has no value, a warning is logged and the walk stops at that gate as well.
         * The variables of the resulting Boolean function are created from the subgraph input nets using `BooleanFunctionNetDecorator::get_boolean_variable`.
         * The Boolean functions computed for the gate outputs of the subgraph are stored in the `cache`, and those stored by earlier calls are reused
         * instead of being computed again. The `cache` must have been created for the netlist of this decorator.
         *
         * @param[in] subgraph_output - The subgraph output net for which to generate the Boolean function.
         * @param[inout] cache - The cache holding the subgraph and the Boolean functions computed for it.
         * @return The combined Boolean function of the subgraph on success, an error otherwise.
         */
        Result<BooleanFunction> get_subgraph_function(const Net* subgraph_output, SubgraphFunctionCache& cache) const;

        /**
         * Get the inputs of the combined Boolean function of a subgraph starting at the source of the provided subgraph output net.
         * This does not actually build the Boolean function but only determines the inputs the subgraph function would have, which is a lot faster.
         * The subgraph is traversed backwards through its gates. A gate is traversed only if its gate type is combinational and it has a Boolean
         * function for the output pin that the walk reaches it through. At any other gate, at a gate outside of the subgraph, at a net without
         * a source, and at a global input net, the walk stops and the net becomes an input of the function. Hence, flip-flops, memories, and gates without a Boolean function
         * bound the subgraph function even if they are part of the `subgraph_gates`.
         * If the Boolean function of a gate cannot be built in terms of nets, e.g., because one of its input pins has no net or a gate parameter it depends on has no value, a warning is logged and the walk stops at that gate as well.
         * For repeated calls on the same subgraph, create a cache with `make_subgraph_function_cache` and pass it to `get_subgraph_function_inputs` instead.
         *
         * @param[in] subgraph_gates - The gates making up the subgraph to consider.
         * @param[in] subgraph_output - The subgraph output net from which to start the back propagation.
         * @return The input nets that would be the input for the subgraph function on success, an error otherwise.
         */
        Result<std::set<const Net*>> get_subgraph_function_inputs(const std::vector<const Gate*>& subgraph_gates, const Net* subgraph_output) const;

        /**
         * Get the inputs of the combined Boolean function of a subgraph starting at the source of the provided subgraph output net.
         * This does not actually build the Boolean function but only determines the inputs the subgraph function would have, which is a lot faster.
         * The subgraph is traversed backwards through its gates. A gate is traversed only if its gate type is combinational and it has a Boolean
         * function for the output pin that the walk reaches it through. At any other gate, at a gate outside of the subgraph, at a net without
         * a source, and at a global input net, the walk stops and the net becomes an input of the function. Hence, flip-flops, memories, and gates without a Boolean function
         * bound the subgraph function even if they are part of the `subgraph_gates`.
         * If the Boolean function of a gate cannot be built in terms of nets, e.g., because one of its input pins has no net or a gate parameter it depends on has no value, a warning is logged and the walk stops at that gate as well.
         * For repeated calls on the same subgraph, create a cache with `make_subgraph_function_cache` and pass it to `get_subgraph_function_inputs` instead.
         *
         * @param[in] subgraph_gates - The gates making up the subgraph to consider.
         * @param[in] subgraph_output - The subgraph output net from which to start the back propagation.
         * @return The input nets that would be the input for the subgraph function on success, an error otherwise.
         */
        Result<std::set<const Net*>> get_subgraph_function_inputs(const std::vector<Gate*>& subgraph_gates, const Net* subgraph_output) const;

        /**
         * Get the inputs of the combined Boolean function of a subgraph starting at the source of the provided subgraph output net.
         * This does not actually build the Boolean function but only determines the inputs the subgraph function would have, which is a lot faster.
         * The subgraph is traversed backwards through its gates. A gate is traversed only if its gate type is combinational and it has a Boolean
         * function for the output pin that the walk reaches it through. At any other gate, at a gate outside of the subgraph, at a net without
         * a source, and at a global input net, the walk stops and the net becomes an input of the function. Hence, flip-flops, memories, and gates without a Boolean function
         * bound the subgraph function even if they are part of the `subgraph_module`.
         * If the Boolean function of a gate cannot be built in terms of nets, e.g., because one of its input pins has no net or a gate parameter it depends on has no value, a warning is logged and the walk stops at that gate as well.
         * For repeated calls on the same subgraph, create a cache with `make_subgraph_function_cache` and pass it to `get_subgraph_function_inputs` instead.
         *
         * @param[in] subgraph_module - The module making up the subgraph to consider, including the gates of all of its submodules.
         * @param[in] subgraph_output - The subgraph output net from which to start the back propagation.
         * @return The input nets that would be the input for the subgraph function on success, an error otherwise.
         */
        Result<std::set<const Net*>> get_subgraph_function_inputs(const Module* subgraph_module, const Net* subgraph_output) const;

        /**
         * Get the inputs of the combined Boolean function of a subgraph starting at the source of the provided subgraph output net.
         * This does not actually build the Boolean function but only determines the inputs the subgraph function would have, which is a lot faster.
         * The subgraph consists of all gates of the netlist for which the `subgraph_filter` evaluates to `true`.
         * The walk stops at gates as described for `get_subgraph_function_inputs(const Net*, SubgraphFunctionCache&)`.
         * For repeated calls on the same subgraph, create a cache with `make_subgraph_function_cache` and pass it to `get_subgraph_function_inputs` instead.
         *
         * @param[in] subgraph_filter - Filter condition a gate has to meet to be part of the subgraph.
         * @param[in] subgraph_output - The subgraph output net from which to start the back propagation.
         * @return The input nets that would be the input for the subgraph function on success, an error otherwise.
         */
        Result<std::set<const Net*>> get_subgraph_function_inputs(const std::function<bool(const Gate*)>& subgraph_filter, const Net* subgraph_output) const;

        /**
         * Get the inputs of the combined Boolean function of a subgraph starting at the source of the provided subgraph output net.
         * This does not actually build the Boolean function but only determines the inputs the subgraph function would have, which is a lot faster.
         * The subgraph consists of all gates of the netlist whose gate type has the given property.
         * The walk stops at gates as described for `get_subgraph_function_inputs(const Net*, SubgraphFunctionCache&)`.
         * For repeated calls on the same subgraph, create a cache with `make_subgraph_function_cache` and pass it to `get_subgraph_function_inputs` instead.
         *
         * @param[in] subgraph_property - The gate type property a gate has to have to be part of the subgraph.
         * @param[in] subgraph_output - The subgraph output net from which to start the back propagation.
         * @return The input nets that would be the input for the subgraph function on success, an error otherwise.
         */
        Result<std::set<const Net*>> get_subgraph_function_inputs(const GateTypeProperty subgraph_property, const Net* subgraph_output) const;

        /**
         * Get the inputs of the combined Boolean function of the subgraph of the cache starting at the source of the provided subgraph output net.
         * This does not actually build the Boolean function but only determines the inputs the subgraph function would have, which is a lot faster.
         * The subgraph is traversed backwards through its gates. A gate is traversed only if its gate type is combinational and it has a Boolean
         * function for the output pin that the walk reaches it through. At any other gate, at a gate outside of the subgraph, at a net without
         * a source, and at a global input net, the walk stops and the net becomes an input of the function. Hence, flip-flops, memories, and gates without a Boolean function
         * bound the subgraph function even if they are part of the subgraph of the `cache`.
         * If the Boolean function of a gate cannot be built in terms of nets, e.g., because one of its input pins has no net or a gate parameter it depends on has no value, a warning is logged and the walk stops at that gate as well.
         * The inputs determined for the gate outputs of the subgraph are stored in the `cache`, and those stored by earlier calls are reused
         * instead of being determined again. The `cache` must have been created for the netlist of this decorator.
         *
         * @param[in] subgraph_output - The subgraph output net from which to start the back propagation.
         * @param[inout] cache - The cache holding the subgraph and the inputs determined for it.
         * @return The input nets that would be the input for the subgraph function on success, an error otherwise.
         */
        Result<std::set<const Net*>> get_subgraph_function_inputs(const Net* subgraph_output, SubgraphFunctionCache& cache) const;

    private:
        const Netlist& m_netlist;
    };
}    // namespace hal