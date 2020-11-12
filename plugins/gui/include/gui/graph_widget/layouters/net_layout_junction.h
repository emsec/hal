#pragma once

#include <QPoint>
#include <QPointF>

#include <QList>
#include <QHash>
#include <QColor>
#include "net_layout_point.h"

class QGraphicsEllipseItem;
class QGraphicsLineItem;
class QGraphicsScene;

QColor colorFromId(u32 netId);

namespace hal {

    class NetLayoutJunctionWireIntersection : public QPoint
    {
        bool mValid;
    public:
        NetLayoutJunctionWireIntersection(bool valid=false, int x_=0, int y_=0)
            : QPoint(x_,y_), mValid(valid) {;}
        enum PlacementType { Normal, Verify, Endpoint };
        bool isValid() const { return mValid; }
        QGraphicsEllipseItem* graphicsFactory(u32 netId) const;
    };

    class NetLayoutJunctionWire
    {
    public:
        int mHorizontal;
        int mRoad;
        int mFirst;
        int mLast;
        NetLayoutJunctionWire(int hor, int rd, int fir, int las)
            : mHorizontal(hor), mRoad(rd), mFirst(fir), mLast(las) {;}
        NetLayoutJunctionWireIntersection intersection(const NetLayoutJunctionWire& other) const;
        bool isEntry() const;
        int centralEnd() const;
    };

    class NetLayoutJunctionEntries
    {
        friend class NetLayoutJunction;
    protected:
        QList<u32> mEntries[4];
    public:
        int size(NetLayoutDirection dir) const { return mEntries[dir.index()].size(); }
        u32 id(NetLayoutDirection dir, int i) const { return mEntries[dir.index()].at(i); }
        QString dump() const;
        void dumpFile(const NetLayoutPoint& pnt) const;
        void setEntries(NetLayoutDirection dir, const QList<u32>& entries_);
    };

    class NetLayoutJunctionNet
    {
        u32 mPattern;
        int mRoadNumber[4];  // index: Left = 0, Right = 1, Up = 2, Down = 3    value: -1 = unused
        int mEntries;
        bool mPlaced;
        NetLayoutJunctionWireIntersection mJunction;
    public:
        QList<NetLayoutJunctionWire> mWires;

        NetLayoutJunctionNet();
        void addEntry(NetLayoutDirection dir, int roadNo);
        QString toString() const;
        bool hasPattern(u32 searchPattern) const;
        int roadNumber(NetLayoutDirection dir) const { return mRoadNumber[dir.index()]; }
        int numberEntries() const { return mEntries; }
        u32 pattern() const { return mPattern; }
        void setPattern(u32 pat) { mPattern = pat; }
        bool isPlaced() const { return mPlaced; }
        void setPlaced() { mPlaced = true; }        
        void addWire(const NetLayoutJunctionWire& wire) { mWires.append(wire); }
        NetLayoutJunctionWireIntersection junctionPoint() const { return mJunction; }
        void setJunctionPoint(const NetLayoutJunctionWireIntersection& jp,
                              NetLayoutJunctionWireIntersection::PlacementType placement
                              = NetLayoutJunctionWireIntersection::Normal);
    };

    class NetLayoutJunctionRange
    {
        u32 mNetId;
        int mFirst;
        int mLast;
    public:
        static const int sMinInf = -32767;
        static const int sMaxInf =  32767;
        static const int sSceneDelta = 20;
        static const int sSceneFirst = 400;
        static const int sSceneGap   = 200;
        NetLayoutJunctionRange(u32 netId_, int first, int last);

        NetLayoutJunctionWire toWire(int hor, int rd) const;
        bool conflict(const NetLayoutJunctionRange& other) const;
        bool canJoin(u32 netId, int pos) const;
        u32 netId() const { return mNetId; }
        int endPosition(int inx) const;
        int graphFirst() const;
        int graphLast() const;
        static NetLayoutJunctionRange entryRange(NetLayoutDirection dir, int iroad, u32 netId);
    };

    class NetLayoutJunctionOccupied : public QList<NetLayoutJunctionRange>
    {
    public:
        bool conflict(const NetLayoutJunctionRange& test) const;
        bool canJoin(u32 netId, int pos) const;
    };

    class NetLayoutJunction
    {
    public:
        NetLayoutJunction(const NetLayoutJunctionEntries& entries);
        ~NetLayoutJunction() {;}
        QRect rect() const { return mRect; }
        void dump() const;
        NetLayoutJunctionNet netById(u32 id) const { return mNetsOutput.value(id); }
        enum mErrorT {StraightRouteError = -2, CornerRouteError = -1, Ok = 0 };
        mErrorT lastError() const { return mError; }
    private:
        void fourWayJunctions(QHash<u32, NetLayoutJunctionNet>::iterator& netIt);
        void routeT();
        void routeAllStraight(NetLayoutDirection dirFrom, NetLayoutDirection dirTo);
        void routeAllCorner(NetLayoutDirection dirHoriz, NetLayoutDirection dirVertic);
        void routeSingleStraight(u32 netId, int iMain, int iroadIn, int iroadOut);
        void routeSingleCorner(u32 netId, NetLayoutDirection dirHoriz, NetLayoutDirection dirVertic);
        void calculateRect();
        bool conflict(int ihoriz, int iroad, const NetLayoutJunctionRange& testRng) const;
        void place(int ihoriz, int iroad, const NetLayoutJunctionRange& range);
        bool canJoin(int ihoriz, int iroad, u32 netId, int pos) const;

        QRect mRect;
        NetLayoutJunctionEntries mEntries;
        QHash<u32,NetLayoutJunctionNet> mNetsInput;
        QHash<u32,NetLayoutJunctionNet> mNetsOutput;
        QHash<int,NetLayoutJunctionOccupied> mOccupied[2];  // 0=horizontal, 1=vertical
        int maxRoad[2];
        mErrorT mError;
    };

    class NetLayoutJunctionHash : public QHash<NetLayoutPoint,NetLayoutJunction*>
    {
    public:
        NetLayoutJunctionHash() {;}
        ~NetLayoutJunctionHash();
        void clearAll();
    };
}

