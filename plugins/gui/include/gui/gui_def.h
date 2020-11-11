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

#include "hal_core/defines.h"

namespace hal
{
    enum class ItemType
    {
        None, Module, Gate, Net
    };

    class Node
    {
    public:
        enum NodeType {None, Module, Gate};

        Node(u32 i=0, NodeType t=None) : mId(i), mType(t) {;}
        NodeType type() const { return mType; }
        u32 id() const { return mId; }

        bool isNull()   const { return mType == None; }
        bool isGate()   const { return mType == Gate; }
        bool isModule() const { return mType == Module; }

        bool operator<(const Node& rhs) const
        {
            if (mType < rhs.mType)
                return true;
            if (mType > rhs.mType)
                return false;
            return mId < rhs.mId;
        }

        bool operator==(const Node& rhs) const
        {
            return mType == rhs.mType && mId == rhs.mId;
        }

        bool operator!=(const Node& rhs) const
        {
            return !(*this == rhs);
        }

    private:

        u32 mId;
        NodeType mType;
    };

    class PlacementHint
    {
    public:
        enum PlacementModeType {Standard = 0, PreferLeft = 1, PreferRight = 2};
        PlacementHint(PlacementModeType mod = Standard, const Node& orign=Node())
            : mMode(mod), mPreferredOrigin(orign) {;}
        PlacementModeType mode() const { return mMode; }
        Node preferredOrigin() const { return mPreferredOrigin; }

        bool operator<(const PlacementHint& rhs) const
        {
            if (mMode < rhs.mMode)
                return true;
            if (mMode > rhs.mMode)
                return false;
            return mPreferredOrigin < rhs.mPreferredOrigin;
        }

        bool operator==(const PlacementHint& rhs) const
        {
            return mMode == rhs.mMode && mPreferredOrigin == rhs.mPreferredOrigin;
        }
    private:
        PlacementModeType mMode;
        Node mPreferredOrigin;

    };

}
