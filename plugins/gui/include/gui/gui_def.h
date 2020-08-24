//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
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

#include "hal_core/def.h"

namespace hal
{
    enum class item_type
    {
        module,
        gate,
        net
    };

    enum class node_type
    {
        module,
        gate
    };

    struct node
    {
        node_type type;
        u32 id;

        bool operator<(const node& rhs) const
        {
            if (type < rhs.type)
                return true;
            else if (rhs.type < type)
                return false;
            else if (id < rhs.id)
                return true;
            else
                return false;
        }

        bool operator==(const node& rhs) const
        {
            return type == rhs.type && id == rhs.id;
        }

        bool operator!=(const node& rhs) const
        {
            return !(*this == rhs);
        }
    };

    enum class placement_mode
    {
        standard = 0,
        prefer_left = 1,
        prefer_right = 2
    };

    struct placement_hint
    {
        placement_mode mode;
        node preferred_origin;

        bool operator<(const placement_hint& rhs) const
        {
            if (mode < rhs.mode)
                return true;
            else if (rhs.mode < mode)
                return false;
            else if (preferred_origin < rhs.preferred_origin)
                return true;
            else
                return false;
        }

        bool operator==(const placement_hint& rhs) const
        {
            return mode == rhs.mode && preferred_origin == rhs.preferred_origin;
        }
    };

}
