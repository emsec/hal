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

    class NetLayoutPoint : public QPoint
    {
    public:
        NetLayoutPoint(int x_=INT_MIN, int y_=INT_MIN);
        NetLayoutPoint(const QPoint& p);
        QGraphicsEllipseItem* graphicsFactory(float r) const;
        int distanceTo(const NetLayoutPoint& other) const;
        int yGrid() const { return (y() + 1) / 2; }
        bool isEndpoint() const;
        bool isUndefined() const { return x()==INT_MIN || y()==INT_MIN; }
        NetLayoutPoint nextPoint(const NetLayoutDirection& dir, bool omitEndpoint=false) const;
        static NetLayoutPoint fromBox(const QPoint& boxPosition, bool isInput);
        static QList<NetLayoutPoint> orderByDistance(const QList<NetLayoutPoint>& points);
    };

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
