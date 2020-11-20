#include "gui/graph_widget/layouters/net_layout_point.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QBrush>
#include <QPen>
#include <QSet>
#include <QMultiMap>
#include <stdlib.h>
#include <QDebug>
#include <QTextStream>

QPointF scenePoint(const QPoint& p)
{
    return QPointF(
                p.x() * 200 + 50,
                p.y() * 100 + 50);
}

namespace hal {

uint qHash(const hal::NetLayoutPoint& p)
{
    return qHash(static_cast<QPoint>(p));
}

uint qHash(const hal::NetLayoutWire& w)
{
    uint retval = qHash(w.endPoint(NetLayoutWire::SourcePoint)) << 1;
    return retval + (w.isHorizontal() ? 1 : 0);
}

//------------ Direction -----------------
NetLayoutDirection::NetLayoutDirection(int idir)
    : mDir(numberToDirection(idir))
{;}

NetLayoutDirection::DirectionType NetLayoutDirection::numberToDirection(int idir)
{
    if (idir < Left || idir > MaxDir) return Undefined;
    return static_cast<DirectionType>(idir);
}

NetLayoutDirection NetLayoutDirection::operator++(int)
{
    NetLayoutDirection retval = *this;
    mDir = numberToDirection(mDir + 1);
    return retval;
}

NetLayoutDirection NetLayoutDirection::operator++()
{
    mDir = numberToDirection(mDir + 1);
    return *this;
}

QPoint NetLayoutDirection::step(bool omitEndpoint) const
{
    int dy = omitEndpoint ? 2 : 1;
    switch (mDir) {
    case Left:
        return QPoint(-1,0);
    case Right:
        return QPoint(1,0);
    case Up:
        return QPoint(0,-dy);
    case Down:
        return QPoint(0,dy);
    default:
        break;
    }
    return QPoint();
}

//------------ Point ---------------------
NetLayoutPoint::NetLayoutPoint(int x_, int y_)
    : QPoint(x_,y_)
{;}

NetLayoutPoint::NetLayoutPoint(const QPoint& p)
    : QPoint(p)
{;}

NetLayoutPoint NetLayoutPoint::nextPoint(const NetLayoutDirection& dir, bool omitEndpoint) const
{
    QPoint p(*this);
    return NetLayoutPoint(p + dir.step(omitEndpoint));
}


NetLayoutPoint NetLayoutPoint::fromBox(const QPoint& boxPosition, bool isInput)
{
    return NetLayoutPoint(boxPosition.x() + (isInput?0:1), boxPosition.y() * 2);
}

bool NetLayoutPoint::isEndpoint() const
{
    return y() % 2 == 0;
}

int NetLayoutPoint::distanceTo(const NetLayoutPoint& other) const
{
    int dy = abs(other.y()-y());
    int dx = abs(other.x()-x());
    if (isEndpoint())
    {
        --dy;
        if (dy<0)
        {
            if (dx==0) return 0;
            dy = 1;
        }
        ++dy;
    }
    return dy*2 + dx*4;
}

QGraphicsEllipseItem* NetLayoutPoint::graphicsFactory(float r) const
{
    QPointF p0 = scenePoint(*this) - QPointF(r/2,r/2);
    QGraphicsEllipseItem* retval = new QGraphicsEllipseItem(p0.x(),p0.y(),r,r);
    retval->setPen(QPen(QBrush(Qt::black),1.));
    return retval;
}

QList<NetLayoutPoint> NetLayoutPoint::orderByDistance(const QList<NetLayoutPoint>& points)
{
    QMultiMap<int, QPair<int,int> > distanceMap;
    QList<NetLayoutPoint> retval;
    QSet<int> placed;

    int n = points.size();
    for (int i=1; i<n; i++)
        for (int j=0; j<i; j++)
            distanceMap.insert(points.at(i).distanceTo(points.at(j)), qMakePair(i,j));

    /*
    for (auto it = distanceMap.begin(); it != distanceMap.end(); ++it)
    {
        int i = it.value().first;
        int j = it.value().second;
        qDebug() << it.key() << i << j << (QPoint) points.at(i) << (QPoint) points.at(j);
    }
    */

    bool isFirst = true;
    while (retval.size() < points.size())
    {
        auto it = distanceMap.begin();
        if (isFirst)
            isFirst = false;
        else
        {
            // search entry where exactly one has been placed
            while(it != distanceMap.end() &&
                  placed.contains(it.value().first) == placed.contains(it.value().second))
                ++it;
        }

        for (int ipair=0; ipair<2; ipair++)
        {
            int i = ipair ? it.value().second : it.value().first;
            if (!placed.contains(i))
            {
                retval.append(points.at(i));
                placed.insert(i);
            }
        }
        distanceMap.erase(it);
    }

    return retval;
}

//------------ Wire ----------------------
NetLayoutWire::NetLayoutWire(const NetLayoutPoint& p, const NetLayoutDirection& dir, bool isEnd)
    : mPoint(p), mDir(dir), mIsEndpoint(isEnd)
{
    switch (dir.direction())
    {
    case NetLayoutDirection::Left:
        mPoint = p.nextPoint(dir);
        mDir = NetLayoutDirection::Right;
        break;
    case NetLayoutDirection::Up:
        mPoint = p.nextPoint(dir, !isEnd);
        mDir = NetLayoutDirection::Down;
        break;
    default:
        break;
    }
}

bool NetLayoutWire::operator==(const NetLayoutWire& other) const
{
    return (mPoint==other.mPoint &&
            mDir==other.mDir &&
            mIsEndpoint==other.mIsEndpoint);
}

QGraphicsLineItem* NetLayoutWire::graphicsFactory() const
{
    if (mDir.isNull()) return nullptr;
    NetLayoutPoint p = mPoint.nextPoint(mDir,!mIsEndpoint);
    QPointF p0 = scenePoint(mPoint);
    QPointF p1 = scenePoint(p);
    QGraphicsLineItem* retval = new QGraphicsLineItem(QLineF(p0,p1));
    retval->setPen(QPen(QBrush(Qt::black),3.));
    return retval;
}

NetLayoutPoint NetLayoutWire::endPoint(WirePointType pnt) const
{
    if (pnt == SourcePoint) return mPoint;
    return NetLayoutPoint(static_cast<QPoint>(mPoint)+mDir.step(!mIsEndpoint));
}

//------------ Connection ----------------
NetLayoutConnection::NetLayoutConnection(const NetLayoutPoint& pa, const NetLayoutPoint& pb)
{
    NetLayoutDirection hDir, vDir;

    if (pa==pb)
    {
        // connection has one junction point and no wire
        mWaypointLinks.insert(pa,QList<int>());
        return;
    }

    NetLayoutPoint waypoint = pa;
    int dx = pb.x() - pa.x();
    int dy = pb.y() - pa.y();

    NetLayoutDirection vdir, hdir;

    if (pa.isEndpoint())
    {
        vdir = (dy < 0) ? NetLayoutDirection::Up : NetLayoutDirection::Down;
        waypoint = addWire(waypoint, vdir, false);
    }

    if (dx)
    {
        hdir = (dx < 0) ? NetLayoutDirection::Left : NetLayoutDirection::Right;
        for (int i=0; i<abs(dx); i++)
            waypoint = addWire(waypoint, hdir, true);
    }

    dy = pb.y() - waypoint.y();
    if (abs(dy) > 1)
    {
        vdir = (dy < 0) ? NetLayoutDirection::Up : NetLayoutDirection::Down;
        int ysteps = abs(dy) / 2;
        for (int i=0; i<ysteps; i++)
            waypoint = addWire(waypoint, vdir, true);
    }

    if (pb.isEndpoint())
    {
        dy = pb.y() - waypoint.y();
        vdir = (dy < 0) ? NetLayoutDirection::Up : NetLayoutDirection::Down;
        addWire(waypoint, vdir, false);
    }
}

NetLayoutPoint NetLayoutConnection::closestPoint(const NetLayoutPoint &pnt) const
{
    NetLayoutPoint retval;
    int bestDistance = 0;
    for (const NetLayoutPoint& testP : mWaypointLinks.keys())
    {
        if (testP.isUndefined()) continue;
        int distance = pnt.distanceTo(testP);
        if (retval.isUndefined() || distance < bestDistance)
        {
            bestDistance = distance;
            retval = testP;
        }
    }
    if (retval.isUndefined())
    {
        qDebug() << "undefined closest point" << pnt.x() << pnt.y();
        for (const NetLayoutPoint& testP : mWaypointLinks.keys())
            qDebug() << (QPoint) testP;
        qDebug() << "-----------";
    }
    return retval;
}

void NetLayoutConnection::add(const NetLayoutConnection& other, bool atomicNet)
{
    for (const NetLayoutWire& w : other)
    {
        int n = size();
        if (atomicNet && !w.isHorizontal() && !w.isEndpoint())
        {
            // split vertical wires so that only atomic parts get stored
            NetLayoutPoint pA = w.endPoint(NetLayoutWire::SourcePoint);
            NetLayoutWire wA(pA, NetLayoutDirection::Down, true);
            NetLayoutPoint pB = wA.endPoint(NetLayoutWire::DestinationPoint);
            NetLayoutWire wB(pB, NetLayoutDirection::Down, true);
            NetLayoutPoint pC = wB.endPoint(NetLayoutWire::DestinationPoint);
            append(wA);
            append(wB);
            mWaypointLinks[pA].append(n);
            mWaypointLinks[pB].append(n);
            mWaypointLinks[pB].append(n+1);
            mWaypointLinks[pC].append(n+1);
        }
        else
        {
            append(w);
            mWaypointLinks[w.endPoint(NetLayoutWire::SourcePoint)].append(n);
            mWaypointLinks[w.endPoint(NetLayoutWire::DestinationPoint)].append(n);
        }
    }
}

NetLayoutPoint NetLayoutConnection::addWire(const NetLayoutPoint &pnt, const NetLayoutDirection &dir, bool omitEndpoint)
{
    int n = size();
    append(NetLayoutWire(pnt,dir,!omitEndpoint));
    mWaypointLinks[pnt].append(n);
    NetLayoutPoint nextP = pnt.nextPoint(dir,omitEndpoint);
    mWaypointLinks[nextP].append(n);
    return nextP;
}

//------------ Metric --------------------
NetLayoutMetric::NetLayoutMetric(u32 id, const NetLayoutConnection *con)
    : mId(id), mFirst(0), mSecond(0)
{
    QMap<int,QMap<int,int>> horizontalMap;
    QMap<int,QMap<int,int>> verticalMap;

    for (const NetLayoutWire& w : *con)
    {
        NetLayoutPoint p = w.endPoint(NetLayoutWire::SourcePoint);
        if (w.isHorizontal())
            horizontalMap[p.y()].insert(p.x(),0);
        else
            verticalMap[p.x()].insert(p.y(),0);
    }
    evaluate(horizontalMap);
    evaluate(verticalMap);
}

bool NetLayoutMetric::operator< (const NetLayoutMetric& other) const
{
    if (mSecond > other.mSecond) return true;
    if (mSecond < other.mSecond) return false;
    if (mFirst > other.mFirst) return true;
    if (mFirst < other.mFirst) return false;
    return (mId < other.mId);
}

void NetLayoutMetric::evaluate(const QMap<int, QMap<int, int> >& map)
{
    for (QMap<int,int> set : map.values() )
    {
        while (!set.isEmpty())
        {

            auto it = set.begin();
            int q = it.key() + 1;
            int n = 0;
            auto jt = set.find(q);
            while (jt != set.end())
            {
                ++q;
                ++n;
                set.erase(jt);
                jt = set.find(q);
            }
            mFirst  += n;
            mSecond += n*n;
            set.erase(it);
        }
    }
}

//------------ Factory -------------------
NetLayoutConnectionFactory::NetLayoutConnectionFactory(const QList<NetLayoutPoint>& sources, const QList<NetLayoutPoint>& destinations)
    : connection(nullptr), mSources(sources), mDestinations(destinations)
{
    mPoints.append(mSources);
    mPoints.append(mDestinations);
    mPoints = NetLayoutPoint::orderByDistance(mPoints);

    /*
    for (const NetLayoutPoint& p : mPoints)
    {
        qDebug() << "point" << (QPoint) p;
    }
*/

    int n=mPoints.size();
    NetLayoutConnection seedConnection(mPoints.at(0),mPoints.at(1));
    for (int i=2; i<n; i++)
    {
        const NetLayoutPoint& nextPoint = mPoints.at(i);
        const NetLayoutPoint& juncPoint = seedConnection.closestPoint(nextPoint);
        mJunctions.append(juncPoint);
        NetLayoutConnection nextConnection(nextPoint,juncPoint);
        seedConnection.add(nextConnection,false);
    }
    connection = new NetLayoutConnection;
    connection->add(seedConnection,true);
}

void NetLayoutConnectionFactory::dump(const QString& stub) const
{
    QTextStream xout(stdout, QIODevice::WriteOnly);
    xout << stub << "\n";
    xout << "src:";
    for (const NetLayoutPoint& pnt : mSources)
    {
        xout << QString(" <%1,%2>").arg(pnt.x()).arg(pnt.y());
    }
    xout << "\ndst:";
    for (const NetLayoutPoint& pnt : mDestinations)
    {
        xout << QString(" <%1,%2>").arg(pnt.x()).arg(pnt.y());
    }
    xout << "\nwire:";
    for (const NetLayoutWire& w : *connection)
    {
        xout << QString(" <%1,%2>%3")
                .arg(w.endPoint(NetLayoutWire::SourcePoint).x())
                .arg(w.endPoint(NetLayoutWire::SourcePoint).y())
                .arg(w.isHorizontal()?'-':'|');
    }
    xout << "\n===========================\n";
}

NetLayoutConnectionMetric::~NetLayoutConnectionMetric()
{
    clearAll();
}

void NetLayoutConnectionMetric::clearAll()
{
    for (NetLayoutConnection* nlc : values())
        delete nlc;
    clear();
}

} // namespace hal
