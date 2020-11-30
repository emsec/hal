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
    /**
     * @brief The ItemType enum provides the enum type to classify graphic items
     * into Modules, Gates, or Nets. The null-type None indicates that the ItemType
     * has not been initialized or that a function could not determine the ItemType
     */
    enum class ItemType
    {
        None, Module, Gate, Net
    };

    /**
     * @brief The Node class object represents a module or a gate.
     *
     * @ingroup gui
     *
     * The Node class has two private members, the Id and the type which
     * is of type Module or Gate. Node objects can be compared, used as
     * Hash keys or returned by functions. For the latter case there is
     * a null-Node object (type==None) to be returned if the function
     * failed to find an appropriate node.
     */
    class Node
    {
    public:
        enum NodeType {None, Module, Gate};

        Node(u32 i=0, NodeType t=None) : mId(i), mType(t) {;}

        /**
         * @brief type getter for type information
         * @return Module or Gate or None for null-Node
         */
        NodeType type() const { return mType; }

        /**
         * @brief id getter for ID information
         * @return ID as unsigned integer
         */
        u32 id() const { return mId; }

        /**
         * @brief isNull test for null-Node object typically returned from functions
         * @return true if node is null
         */
        bool isNull()   const { return mType == None; }

        /**
         * @brief isGate test whether node is a gate
         * @return true if node is a gate
         */
        bool isGate()   const { return mType == Gate; }

        /**
         * @brief isModule test wheter node is a module
         * @return true if node is a module
         */
        bool isModule() const { return mType == Module; }

        /**
         * @brief operator < to provide an ordering scheme for maps and ordered lists
         * @param rhs the other Node to be compared with
         * @return true if node is 'smaller' according to test
         */
        bool operator<(const Node& rhs) const
        {
            if (mType < rhs.mType)
                return true;
            if (mType > rhs.mType)
                return false;
            return mId < rhs.mId;
        }

        /**
         * @brief operator == to test whether two nodes are equal
         * @param rhs the other Node to be compared with
         * @return true if nodes are of same type and have same ID
         */
        bool operator==(const Node& rhs) const
        {
            return mType == rhs.mType && mId == rhs.mId;
        }

        /**
         * @brief operator != to test whether two nodes are not equal
         * @param rhs the other Node to be compared with
         * @return true if nodes have either different type or different ID
         */
        bool operator!=(const Node& rhs) const
        {
            return !(*this == rhs);
        }

    private:

        u32 mId;
        NodeType mType;
    };


    uint qHash(const Node &n);

    /**
     * @brief The PlacementHint class object provides hints for the layouter how new box objects
     * are placed on a view. In standard mode placement is done using the most compact squere-like
     * arrangement. Alternatively new box objects can be placed to the left or right to a node
     * which serves as 'origin'.
     */
    class PlacementHint
    {
    public:
        /**
         * @brief The PlacementModeType enum either most compact arrangement (Standard) or to the
         * left or right of given origin
         */
        enum PlacementModeType {Standard = 0, PreferLeft = 1, PreferRight = 2};

        /**
         * @brief PlacementHint constructor
         * @param mod placement mode must be either Standard or PreferLeft or PreferRight
         * @param orign node to start from when PreferLeft or PreferRight are set
         */
        PlacementHint(PlacementModeType mod = Standard, const Node& orign=Node())
            : mMode(mod), mPreferredOrigin(orign) {;}

        /**
         * @brief mode getter for placement mode type
         * @return either Standard or PreferLeft or PreferRight
         */
        PlacementModeType mode() const { return mMode; }

        /**
         * @brief preferredOrigin getter for placement origin if any.
         * @return the Node object
         *
         * It can be tested by preferredOrigin().isNull if no origin has been set.
         */
        Node preferredOrigin() const { return mPreferredOrigin; }

        /**
         * @brief operator < provide an order structure for placement hints
         * @param rhs the other placement hint to compare with
         * @return true if placement hint is 'smaller' according to test
         */
        bool operator<(const PlacementHint& rhs) const
        {
            if (mMode < rhs.mMode)
                return true;
            if (mMode > rhs.mMode)
                return false;
            return mPreferredOrigin < rhs.mPreferredOrigin;
        }

        /**
         * @brief operator == test whether two placement hints are equal
         * @param rhs the other placement hint to compare with
         * @return true if placement hints have same placement mode type and origin
         */
        bool operator==(const PlacementHint& rhs) const
        {
            return mMode == rhs.mMode && mPreferredOrigin == rhs.mPreferredOrigin;
        }
    private:
        PlacementModeType mMode;
        Node mPreferredOrigin;
    };

}
