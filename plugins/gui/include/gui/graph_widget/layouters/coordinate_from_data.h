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

#include "hal_core/defines.h"

#include <QPoint>
#include <QHash>
#include <QSet>
#include <map>
#include <limits.h>

uint qHash(const QPoint& pnt);

namespace hal {

    class Module;
    class Gate;
    class Node;

    /**
     * @ingroup graph-layouter
     * @brief Utility class that extracts (if possible) x and y coordinates contained in the netlist.
     */
    class CoordinateFromData : public QPoint
    {
    public:
        CoordinateFromData(int x_=INT_MIN, int y_=INT_MIN);
        bool isUndefined() const { return x()==INT_MIN || y()==INT_MIN; }
        static CoordinateFromData fromNode(const Node& nd);
    private:
        static CoordinateFromData fromData(const std::map<std::tuple<std::string, std::string>, std::tuple<std::string, std::string>>& dc);
    };

    /**
     * @ingroup graph-layouter
     * @brief Utility class that stores the nodes that contained existing x and y coordinates.
     */
    class CoordinateFromDataMap : public QHash<hal::Node,CoordinateFromData>
    {
    public:
        CoordinateFromDataMap(const QSet<u32>& modules, const QSet<u32>& gates);
        bool good() const;
        void simplify();
        void clear();
        bool isPlacementComplete() const;
        const QSet<u32>& placedGates() const { return mPlacedGates; }
        const QSet<u32>& placedModules() const { return mPlacedModules; }
    private:
        int mUndefCount;
        int mPlacedCount;
        int mDoubleCount;
        QHash<QPoint,int> mPositionHash;
        void insertNode(const hal::Node& nd, const CoordinateFromData& cfd);

        QSet<u32> mPlacedGates;
        QSet<u32> mPlacedModules;
    };
}
