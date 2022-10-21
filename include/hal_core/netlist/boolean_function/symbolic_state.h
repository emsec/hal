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

#include <map>

namespace hal
{
    namespace SMT
    {

        /**
         * Represents the data structure that keeps track of symbolic variable values (e.g., required for symbolic simplification).
         */
        class SymbolicState final
        {
        public:
            ////////////////////////////////////////////////////////////////////////////
            // Constructors, Destructors, Operators
            ////////////////////////////////////////////////////////////////////////////

            /**
             * Constructs a symbolic state and initializes the variables.
             * 
             * @param[in] variables - The list of variables.
             */
            explicit SymbolicState(const std::vector<BooleanFunction>& variables = {});

            ////////////////////////////////////////////////////////////////////////////
            // Interface
            ////////////////////////////////////////////////////////////////////////////

            /**
             * Looks up a Boolean function in the symbolic state.
             * 
             * @param[in] key - The Boolean function to look up.
             * @returns The Boolean function from the symbolic state or the key itself if it is not contained in the symbolic state.
             */
            const BooleanFunction& get(const BooleanFunction& key) const;

            /**
             * Sets a Boolean function equivalent in the symbolic state.
             * 
             * @param[in] key - The Boolean function.
             * @param[in] value - The equivalent Boolean function.
             */
            void set(const BooleanFunction& key, const BooleanFunction& value);

            ////////////////////////////////////////////////////////////////////////////
            // Members
            ////////////////////////////////////////////////////////////////////////////
        private:
            /// refers to the symbolic state map
            std::map<BooleanFunction, BooleanFunction> variable;
        };

    }    // namespace SMT
}    // namespace hal