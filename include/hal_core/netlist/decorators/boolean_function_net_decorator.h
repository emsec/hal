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
    class NETLIST_API BooleanFunctionNetDecorator
    {
    public:
        /**
         * Construct new BooleanFunctionNetDecorator object.
         * 
         * @param[in] net - The net to operate on. 
         */
        BooleanFunctionNetDecorator(const Net& net);

        /**
         * Generate a unique Boolean variable representing the given net.
         * 
         * @return The Boolean variable.
         */
        BooleanFunction get_boolean_variable() const;

        /**
         * Generate a unique Boolean variable name representing the given net.
         * 
         * @return The Boolean variable name.
         */
        std::string get_boolean_variable_name() const;

        /**
         * Get the net represented by a unique Boolean variable.
         * 
         * @param[in] netlist - The netlist on which to operate.
         * @param[in] var - The Boolean variable.
         * @return The specified net on success, an error otherwise.
         */
        static Result<Net*> get_net_from(const Netlist* netlist, const BooleanFunction& var);

        /**
         * Get the net represented by a unique Boolean variable name.
         * 
         * @param[in] netlist - The netlist on which to operate.
         * @param[in] var_name - The Boolean variable name.
         * @return The specified net on success, an error otherwise.
         */
        static Result<Net*> get_net_from(const Netlist* netlist, const std::string& var_name);

        /**
         * Get the net id represented by a unique Boolean variable.
         * 
         * @param[in] var - The Boolean variable.
         * @return The specified net id on success, an error otherwise.
         */
        static Result<u32> get_net_id_from(const BooleanFunction& var);

        /**
         * Get the net id represented by a unique Boolean variable name.
         * 
         * @param[in] var_name - The Boolean variable name.
         * @return The specified net id on success, an error otherwise.
         */
        static Result<u32> get_net_id_from(const std::string& var_name);

    private:
        static const std::string VAR_NET_PREFIX;
        const Net& m_net;
    };
}    // namespace hal