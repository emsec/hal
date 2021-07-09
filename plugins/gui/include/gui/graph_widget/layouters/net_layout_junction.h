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

#include <QPoint>
#include <QPointF>

#include <QList>
#include <QHash>
#include <QColor>
#include <QRect>
#include "net_layout_point.h"

class QGraphicsEllipseItem;
class QGraphicsLineItem;
class QGraphicsScene;

QColor colorFromId(u32 netId);

namespace hal {

    /**
     * @ingroup gui
     */
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

    /**
     * @ingroup graph-layouter
     */
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

    /**
     * @ingroup graph-layouter
     */
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

    class NetLayoutJunctionMultiPin
    {
    public:
        int mRoad;
        QList<int> mConnector;
        NetLayoutJunctionMultiPin() : mRoad(-1) {}
        void setRoad(int road) { mRoad = road; }
    };


    /**
     * @ingroup graph-layouter
     */
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

    /**
     * @ingroup graph-layouter
     */
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
        bool canJoin(const NetLayoutJunctionRange& other) const;
        u32 netId() const { return mNetId; }
        int endPosition(int inx) const;
        bool isEntry(int inx) const;
        int graphFirst() const;
        int graphLast() const;
        bool operator==(const NetLayoutJunctionRange& other) const;
        void expand(const NetLayoutJunctionRange& other);
        static NetLayoutJunctionRange entryRange(NetLayoutDirection dir, int iroad, u32 netId);
    };

    /**
     * @ingroup graph-layouter
     */
    class NetLayoutJunctionOccupied : public QList<NetLayoutJunctionRange>
    {
    public:
        bool conflict(const NetLayoutJunctionRange& test) const;
        bool canJoin(u32 netId, int pos) const;
        void add(const NetLayoutJunctionRange& rng);
    };

    /**
     * @ingroup graph-layouter
     */
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
        void routeSingleDetour(u32 netId, int iMain, int iroadIn, int iroadOut);
        void routeSingleCorner(u32 netId, NetLayoutDirection dirHoriz, NetLayoutDirection dirVertic);
        void routeAllMultiPin(NetLayoutDirection leftOrRight);
        void routeSingleMultiPin(u32 netId, NetLayoutDirection leftOrRight, const NetLayoutJunctionMultiPin &nmpin);
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

    /**
     * @ingroup graph-layouter
     */
    class NetLayoutJunctionHash : public QHash<NetLayoutPoint,NetLayoutJunction*>
    {
    public:
        NetLayoutJunctionHash() {;}
        ~NetLayoutJunctionHash();
        void clearAll();
    };
}

