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
#include <QMap>
#include <QHash>

class QGraphicsEllipseItem;
class QGraphicsLineItem;

QPointF scenePoint(const QPoint& p);

typedef quint32 u32;

namespace hal {

    /**
     * @ingroup graph-layouter
     */
    class NetLayoutDirection
    {
    public:
        enum DirectionType {Undefined = -1, Left = 0, Right = 1, Up = 2, Down = 3, MaxDir = 4};
        NetLayoutDirection(DirectionType dir = Undefined) : mDir(dir) {;}
        NetLayoutDirection(int idir);
        bool isHorizontal() const { return mDir == Left || mDir == Right; }
        int iHorizontal() const { return index()/2; }
        bool isVertical() const { return mDir == Up || mDir == Down; }
        bool isNull() const { return mDir == Undefined; }
        QPoint step(bool omitEndpoint=false) const;
        DirectionType direction() const { return mDir; }
        u32 toPattern() const { return  1 << mDir; }
        NetLayoutDirection operator++(int);
        NetLayoutDirection operator++();
        int index() const { return static_cast<int>(mDir); }
        bool isLeft() const { return mDir==Left; }
        bool isUp()   const { return mDir==Up; }
        bool isMax()  const { return mDir==MaxDir; }
        bool operator==(const NetLayoutDirection& other) const { return mDir == other.mDir; }
    private:
        static DirectionType numberToDirection(int idir);
        DirectionType mDir;
    };

    /**
     * @ingroup graph-layouter
     */
    class NetLayoutPoint : public QPoint
    {
    public:
        NetLayoutPoint(int x_=INT_MIN, int y_=INT_MIN);
        NetLayoutPoint(const QPoint& p);
        QGraphicsEllipseItem* graphicsFactory(float r) const;
        int distanceTo(const NetLayoutPoint& other) const;
        int yGrid() const;
        QPoint gridPoint() const { return QPoint(x(),yGrid()); }
        bool isEndpoint() const;
        bool isUndefined() const { return x()==INT_MIN || y()==INT_MIN; }
        NetLayoutPoint nextPoint(const NetLayoutDirection& dir, bool omitEndpoint=false) const;
        static NetLayoutPoint fromBox(const QPoint& boxPosition, bool isInput);
        static QList<NetLayoutPoint> orderByDistance(const QList<NetLayoutPoint>& points);
    };

    /**
     * @ingroup graph-layouter
     */
    class NetLayoutWire
    {
    public:
        enum WirePointType {SourcePoint, DestinationPoint};
        NetLayoutWire(const NetLayoutPoint& p, const NetLayoutDirection& dir, bool isEnd);
        QGraphicsLineItem* graphicsFactory() const;

        NetLayoutPoint endPoint(WirePointType pnt) const;
        bool isEndpoint() const { return mIsEndpoint; }
        bool isHorizontal() const { return mDir.isHorizontal(); }
        bool operator==(const NetLayoutWire& other) const;
    private:
        NetLayoutPoint mPoint;
        NetLayoutDirection mDir;
        bool mIsEndpoint;
    };

    /**
     * @ingroup graph-layouter
     */
    class NetLayoutConnection : public QList<NetLayoutWire>
    {
    public:
        NetLayoutConnection() {;}
        NetLayoutConnection(const NetLayoutPoint& pa, const NetLayoutPoint& pb);
        QList<NetLayoutPoint> wayPoints() const { return mWaypointLinks.keys(); }
        NetLayoutPoint closestPoint(const NetLayoutPoint& pnt) const;
        void add(const NetLayoutConnection& other, bool atomicNet);
    private:
        QHash<NetLayoutPoint,QList<int>> mWaypointLinks;

        NetLayoutPoint addWire(const NetLayoutPoint& pnt, const NetLayoutDirection& dir, bool omitEndpoint);
    };

    /**
     * @ingroup graph-layouter
     */
    class NetLayoutMetric
    {
    public:
        NetLayoutMetric(u32 id, const NetLayoutConnection* con);
        u32 getId() const { return mId; }
        int firstMoment() const {return mFirst; }
        int secondMoment() const {return mSecond; }
        bool operator< (const NetLayoutMetric& other) const;
    private:
        void evaluate(const QMap<int,QMap<int,int>>& map);
        u32 mId;
        int mFirst;
        int mSecond;
    };

    /**
     * @ingroup graph-layouter
     */
    class NetLayoutConnectionFactory
    {
    public:
        NetLayoutConnectionFactory(const QList<NetLayoutPoint>& sources, const QList<NetLayoutPoint>& destinations);
        QList<NetLayoutPoint> points() const { return mPoints; }
        QList<NetLayoutPoint> mJunctions;
        NetLayoutConnection* connection;
        void dump(const QString& stub) const;
    private:
        QList<NetLayoutPoint> mSources;
        QList<NetLayoutPoint> mDestinations;
        QList<NetLayoutPoint> mPoints;
    };

    /**
     * @ingroup graph-layouter
     */
    class NetLayoutConnectionMetric : public QMap<NetLayoutMetric,NetLayoutConnection*>
    {
    public:
        NetLayoutConnectionMetric() {;}
        ~NetLayoutConnectionMetric();
        void clearAll();
    };

    uint qHash(const hal::NetLayoutWire& w);
    uint qHash(const hal::NetLayoutPoint& p);
}

uint qHash(const QPoint& p);
