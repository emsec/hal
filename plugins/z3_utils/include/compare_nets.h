//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

namespace hal
{
    /* forward declaration */
    class Netlist;
    class Net;

    namespace z3_utils
    {
        /**
         * Compare two nets from two different netlist. This is done on a functional level and 
         * compares the combinational gates infront of the nets until hitting a sequential gate.
         * In order for this two work the sequential gates of both netlists must be identical and only the combinational gates my differ.
         * If replace_net_ids is set the function subtitutes the input nets of the functions with their source gate and pin.
         * This allows to also change the output nets of the sequential gates.
         * 
         * @param[in] netlist_a - The first netlist.
         * @param[in] netlist_b - The second netlist.
         * @param[in] net_a - First net, from netlist_a.
         * @param[in] net_b - Second net, from netlist_b.
         * @param[in] replace_net_ids - If set, the input_net_ids are substituted by their source gate and pin.
         * 
         */
       bool compare_nets(const Netlist* netlist_a, const Netlist* netlist_b, const Net* net_a, const Net* net_b,  bool replace_net_ids=true);
    }    // namespace z3_utils
}    // namespace hal