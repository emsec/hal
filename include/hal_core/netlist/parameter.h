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
#include "hal_core/utilities/enums.h"
#include "hal_core/utilities/result.h"

#include <map>
#include <string>
#include <vector>

namespace hal
{
    /**
     * A typed, named parameter declaration.
     * 
     * @ingroup netlist
     */
    struct NETLIST_API Parameter
    {
        /**
         * Type tag for typed parameters across the netlist.
         */
        enum class Type
        {
            /** A multi-bit unsigned integer value. */
            BitVector,
            /** A finite, ordered set of named values; encoded as an integer index. */
            Enum,
        };

        /** Parameter name. */
        std::string name;
        /** Data type of the parameter. */
        Type type;
        /** Bit-width of the parameter. For enums this is `ceil(log2(values.size()))` (>= 1). */
        u16 size = 0;
        /** Default value. */
        std::string default_value;
        /** Ordered list of enum values; empty for non-enum parameters. */
        std::vector<std::string> enum_values;

        bool operator==(const Parameter& other) const;
        bool operator!=(const Parameter& other) const;
    };

    template<>
    std::map<Parameter::Type, std::string> EnumStrings<Parameter::Type>::data;
}    // namespace hal
