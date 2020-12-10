#pragma once

#include <QPoint>
#include <QHash>
#include <QSet>
#include <map>
#include <limits.h>
#include "hal_core/defines.h"

uint qHash(const QPoint& pnt);

namespace hal {

    class Module;
    class Gate;
    class Node;

    class CoordinateFromData : public QPoint
    {
    public:
        CoordinateFromData(int x_=INT_MIN, int y_=INT_MIN);
        bool isUndefined() const { return x()==INT_MIN || y()==INT_MIN; }
        static CoordinateFromData fromNode(const Node& nd);
    private:
        static CoordinateFromData fromData(const std::map<std::tuple<std::string, std::string>, std::tuple<std::string, std::string>>& dc);
    };

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
