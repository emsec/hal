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

#include "hal_core/utilities/result.h"

#include "z3++.h"

#include <vector>


namespace hal
{
    class Net;
    class Netlist;

    namespace z3_utils
    {
        /**
         * @brief Compare two nets from two different netlist. 
         * 
         * This is done on a functional level by buidling the subgraph function of each net considering all combinational gates of the netlist.
         * In order for this two work the sequential gates of both netlists must have identical names and only the combinational gates may differ.
         * 
         * @param[in] netlist_a - The first netlist.
         * @param[in] netlist_b - The second netlist.
         * @param[in] net_a - First net, from netlist_a.
         * @param[in] net_b - Second net, from netlist_b.
         * @param[in] fail_on_unknown - Determines whether the function returns false or true incase the SAT solver returns unknown.
         * @param[in] solver_timeout - The timeout for the SAT solver query in seconds.
         * @returns Ok and a Boolean indicating whether the two nets are functionally equivalent, an error otherwise.
         */
        Result<bool> compare_nets(const Netlist* netlist_a, const Netlist* netlist_b, const Net* net_a, const Net* net_b, const bool fail_on_unknown = true, const u32 solver_timeout = 10);

        /**
         * @brief Compare pairs of nets from two different netlist. 
         * 
         * This is done on a functional level by buidling the subgraph function of each net considering all combinational gates of the netlist.
         * In order for this two work the sequential gates of both netlists must have identical names and only the combinational gates may differ.
         * 
         * @param[in] netlist_a - The first netlist.
         * @param[in] netlist_b - The second netlist.
         * @param[in] nets - The pairs of nets to compare against each other.
         * @param[in] fail_on_unknown - Determines whether the function returns false or true incase the SAT solver returns unknown.
         * @param[in] solver_timeout - The timeout for each SAT solver query in seconds.
         * @returns Ok and a Boolean indicating whether the two nets are functionally equivalent, an error otherwise.
         */
        Result<bool> compare_nets(const Netlist* netlist_a, const Netlist* netlist_b, const std::vector<std::pair<Net*, Net*>>& nets, const bool fail_on_unknown = true, const u32 solver_timeout = 10);

        /**
         * @brief Compares two netlist on a functional level.
         * 
         * This is done by finding a corresponding partner for each sequential gate in the netlist and checking whether they are identical.
         * This is done on a functional level by buidling the subgraph function of all their input nets considering all combinational gates of the netlist.
         * In order for this two work the sequential gates of both netlists must have identical names and only the combinational gates may differ.
         * 
         * @param[in] netlist_a - The first netlist.
         * @param[in] netlist_b - The second netlist.
         * @param[in] fail_on_unknown - Determines whether the function returns false or true incase the SAT solver returns unknown.
         * @param[in] solver_timeout - The timeout for each SAT solver query in seconds.
         * 
         * @returns Ok and a Boolean indicating whether the two netlists are functionally equivalent, an error otherwise.
         */
        Result<bool> compare_netlists(const Netlist* netlist_a, const Netlist* netlist_b, const bool fail_on_unknown = true, const u32 solver_timeout = 10);
    }    // namespace z3_utils
}    // namespace hal
