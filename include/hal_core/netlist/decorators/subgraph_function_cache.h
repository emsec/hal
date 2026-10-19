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

#include "hal_core/defines.h"
#include "hal_core/netlist/boolean_function.h"

#include <map>
#include <unordered_set>
#include <utility>

namespace hal
{
    class Gate;
    class GatePin;
    class Netlist;

    /**
     * A reusable store for the Boolean functions that `SubgraphNetlistDecorator::get_subgraph_function` computes for the output pins of the gates of one specific subgraph.
     *
     * The subgraph a cache belongs to is sealed in when it is created with `SubgraphNetlistDecorator::make_subgraph_function_cache`,
     * and the cache can only ever be used for exactly that subgraph.
     *
     * The cache belongs to the netlist it was created for and must be dropped when the netlist is modified, as its entries are not invalidated by netlist events.
     *
     * @ingroup decorators
     */
    class NETLIST_API SubgraphFunctionCache
    {
    public:
        SubgraphFunctionCache(SubgraphFunctionCache&&)                 = default;
        SubgraphFunctionCache& operator=(SubgraphFunctionCache&&)      = default;
        SubgraphFunctionCache(const SubgraphFunctionCache&)            = delete;
        SubgraphFunctionCache& operator=(const SubgraphFunctionCache&) = delete;

        /**
         * Get the number of cached Boolean functions.
         *
         * @returns The number of cached Boolean functions.
         */
        size_t size() const
        {
            return m_functions.size();
        }

    private:
        friend class SubgraphNetlistDecorator;

        SubgraphFunctionCache(const Netlist* netlist, std::unordered_set<const Gate*> subgraph_gates) : m_netlist(netlist), m_subgraph_gates(std::move(subgraph_gates))
        {
        }

        const Netlist* m_netlist;
        std::unordered_set<const Gate*> m_subgraph_gates;
        std::map<std::pair<u32, const GatePin*>, BooleanFunction> m_functions;
    };
}    // namespace hal
