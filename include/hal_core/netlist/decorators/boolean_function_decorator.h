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
#include "hal_core/netlist/net.h"

namespace hal
{
    class NETLIST_API BooleanFunctionDecorator
    {
    public:
        /**
         * Construct new BooleanFunction object.
         * 
         * @param[in] bf - The Boolean function to operate on. 
         */
        BooleanFunctionDecorator(const BooleanFunction& bf);

        /**
         * Substitute all Boolean function variables fed by power or ground gates by constant '1' and '0'. 
         * 
         * @param[in] nl - The netlist to operate on.
         * @return The resulting Boolean function on success, an error otherwise.
         */
        Result<BooleanFunction> substitute_power_ground_nets(const Netlist* nl) const;

        /**
         * Get the Boolean function that is the concatenation of variable names corresponding to nets of a netlist.
         * The Boolean function can optionally be extended to any desired size greater the size of the given net vector.
         * 
         * @param[in] nets - The nets to concatenate. 
         * @param[in] extend_to_size - The size to which to extend the Boolean function. Set to 0 to prevent extension. Defaults to 0.
         * @param[in] sign_extend - Set `true` to sign extend, `false` to zero extend. Defaults to `false`.
         * @return The resulting Boolean function on success, an error otherwise.
         */
        static Result<BooleanFunction> get_boolean_function_from(const std::vector<Net*>& nets, u32 extend_to_size = 0, bool sign_extend = false);

    private:
        const BooleanFunction& m_bf;
    };
}    // namespace hal