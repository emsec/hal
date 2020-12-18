#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QBrush>
#include <QPen>
#include <QSet>
#include <QDir>
#include <QMultiMap>
#include <stdlib.h>
#include <QDebug>
#include <QPoint>
#include "gui/graph_widget/layouters/net_layout_junction.h"

QColor colorFromId(u32 netId)
{
    int hue = (netId-1) * 25;
    return QColor::fromHsv(hue,255,255);
}

namespace hal {

    NetLayoutJunction::NetLayoutJunction(const NetLayoutJunctionEntries& entries)
        : mEntries(entries), mError(Ok)
    {
        memset(maxRoad,0,sizeof(maxRoad));
        for (NetLayoutDirection dir(0); !dir.isMax(); ++dir)
        {
            int iHoriz = dir.iHorizontal();
            int n = mEntries.size(dir);
            if (n>maxRoad[iHoriz]) maxRoad[iHoriz] = n;
            for (int i=0; i<n; i++)
            {
                u32 entryId = mEntries.id(dir,i);
                if (entryId)
                mNetsInput[entryId].addEntry(dir,i);
            }
        }
        routeAllStraight(0,1);
        routeAllStraight(2,3);
        for (auto netIt=mNetsInput.begin(); netIt!=mNetsInput.end(); ++netIt)
            if (netIt.value().numberEntries()==4)
                fourWayJunctions(netIt);
        routeT();
        routeAllCorner(0,2);
        routeAllCorner(0,3);
        routeAllCorner(1,2);
        routeAllCorner(1,3);
        calculateRect();
    }


    void NetLayoutJunction::calculateRect()
    {
        int x0, y0, x1, y1;
        x0 = y0 = x1 = y1 = 0;
        if (!mOccupied[0].isEmpty())
        {
            for (int y : mOccupied[0].keys())
            {
                if (y<y0)   y0 = y;
                if (y+1>y1) y1 = y+1;
            }
        }
        if (!mOccupied[1].isEmpty())
        {
            for (int x : mOccupied[1].keys())
            {
                if (x<x0)   x0 = x;
                if (x+1>x1) x1 = x+1;
            }
        }
        if (y1<=0) y1 = 1;
        if (x1<=0) x1 = 1;
        mRect = QRect(x0,y0,x1-x0,y1-y0);

        for (int ihoriz = 0; ihoriz<2; ihoriz++)
            for (auto itOcc = mOccupied[ihoriz].constBegin(); itOcc!=mOccupied[ihoriz].constEnd(); ++itOcc)
            {
                 int iroad = itOcc.key();
                 for (const NetLayoutJunctionRange& rng : itOcc.value())
                 {
                     NetLayoutJunctionWire w = rng.toWire(ihoriz,iroad);
                     if (w.mFirst == NetLayoutJunctionRange::sMinInf)
                     {
                         if (w.mHorizontal==0)
                             w.mFirst = x0-1;
                         else
                             w.mFirst = y0-1;
                     }
                     if (w.mLast == NetLayoutJunctionRange::sMaxInf)
                     {
                         if (w.mHorizontal==0)
                             w.mLast = x1;
                         else
                             w.mLast = y1;
                     }
                     mNetsOutput[rng.netId()].addWire(w);
                 }
            }
    }

    void NetLayoutJunction::fourWayJunctions(QHash<u32, NetLayoutJunctionNet>::iterator& netIt)
    {
        QList<NetLayoutJunctionWire> wires[2];
        for (const NetLayoutJunctionWire& w : netIt.value().mWires)
            wires[w.mHorizontal].append(w);

        for (const NetLayoutJunctionWire& hw : wires[0])
        {
            if (!hw.isEntry()) continue;
            for (const NetLayoutJunctionWire& vw: wires[1])
            {
                if (!vw.isEntry()) continue;
                NetLayoutJunctionWireIntersection pnt = hw.intersection(vw);
                if (pnt.isValid())
                {
                    netIt->setJunctionPoint(pnt);
                    return;
                }
            }
        }
    }

    void NetLayoutJunction::dump() const
    {
        qDebug() << "-net-";
        for (u32 id : mNetsInput.keys() )
        {
            qDebug() << id << mNetsInput.value(id).toString();
        }
        qDebug() << mEntries.dump();
    }

    void NetLayoutJunction::routeT()
    {
        for (auto itNet=mNetsInput.begin(); itNet!=mNetsInput.end();++itNet)
        {
            if (itNet.value().numberEntries() == 3)
            {
                int i0, i1, ilink, ihoriz;
                u32 reducedPattern;
                switch (itNet.value().pattern())
                {
                case 7:    // L U R
                    i0 = 0;
                    i1 = 1;
                    ilink = 2;
                    ihoriz = 1;
                    reducedPattern = itNet.value().roadNumber(i1) > itNet.value().roadNumber(i0) ? 5 : 6;
                    break;
                case 11:    // L D R
                    i0 = 0;
                    i1 = 1;
                    ilink = 3;
                    ihoriz = 1;
                    reducedPattern = itNet.value().roadNumber(i1) > itNet.value().roadNumber(i0) ? 10 : 9;
                    break;
                case 13:   //  U L D
                    i0 = 2;
                    i1 = 3;
                    ilink = 0;
                    ihoriz = 0;
                    reducedPattern = itNet.value().roadNumber(i1) > itNet.value().roadNumber(i0) ? 5 : 9;
                    break;
                case 14:   //  U R D
                    i0 = 2;
                    i1 = 3;
                    ilink = 1;
                    ihoriz = 0;
                    reducedPattern = itNet.value().roadNumber(i1) > itNet.value().roadNumber(i0) ? 10 : 6;
                    break;
                default:
                    return;
                }

                int road0 = itNet.value().roadNumber(i0);
                int road1 = itNet.value().roadNumber(i1);
                int roadLink = itNet.value().roadNumber(ilink);
                int jhoriz = 1-ihoriz;
                int roadj;
                if (canJoin(jhoriz,road0,itNet.key(),ilink))
                    roadj = road0;
                else if (canJoin(jhoriz,road1,itNet.key(),ilink))
                    roadj = road1;
                else
                {
                    qDebug() << "T-join not found";
                    return;
                }

                NetLayoutJunctionRange rngT =
                        NetLayoutJunctionRange::entryRange(ilink,roadj,itNet.key());
                if (conflict(ihoriz,roadLink,rngT))
                    itNet->setPattern(reducedPattern);
                else
                {
                    place(ihoriz,roadLink,rngT);
                    itNet->setPlaced();
                    itNet->setJunctionPoint(
                                NetLayoutJunctionWireIntersection(
                                    true,
                                    ihoriz?roadLink:roadj,
                                    ihoriz?roadj:roadLink));
                }
            }
        }
    }

    void NetLayoutJunction::routeAllCorner(NetLayoutDirection dirHoriz, NetLayoutDirection dirVertic)
    {
        u32 searchPattern = dirHoriz.toPattern() | dirVertic.toPattern();
        if (dirVertic.isUp())
            for (auto it = mEntries.mEntries[dirHoriz.index()].begin();
                 it != mEntries.mEntries[dirHoriz.index()].end(); ++it)
            {
                if (*it == 0) continue;
                NetLayoutJunctionNet net = mNetsInput.value(*it);
                if (net.hasPattern(searchPattern) && !net.isPlaced())
                    routeSingleCorner(*it,dirHoriz,dirVertic);
            }
        else
            for (auto it = mEntries.mEntries[dirHoriz.index()].rbegin();
                 it != mEntries.mEntries[dirHoriz.index()].rend(); ++it)
            {
                if (*it == 0) continue;
                NetLayoutJunctionNet net = mNetsInput.value(*it);
                if (net.hasPattern(searchPattern) && !net.isPlaced())
                    routeSingleCorner(*it,dirHoriz,dirVertic);
            }
    }

    void NetLayoutJunction::routeAllStraight(NetLayoutDirection dirFrom, NetLayoutDirection dirTo)
    {
        QHash<int,int> straightConnected;
        u32 searchPattern = dirFrom.toPattern() | dirTo.toPattern();

        for (u32 netId : mEntries.mEntries[dirFrom.index()])
        {
            if (netId==0) continue;
            auto itNet = mNetsInput.find(netId);
            Q_ASSERT(itNet != mNetsInput.end());
            if (itNet.value().hasPattern(searchPattern))
            {
                int iroadIn  = itNet.value().roadNumber(dirFrom);
                int iroadOut = itNet.value().roadNumber(dirTo);
                if (straightConnected.value(iroadOut,-1) == iroadIn) // swap detected
                    routeSingleSwap(netId, (searchPattern==3?0:1), iroadIn, iroadOut);
                else
                    routeSingleStraight(netId, (searchPattern==3?0:1), iroadIn, iroadOut);
                straightConnected[iroadIn] = iroadOut;
                if (itNet.value().numberEntries() % 2 ==0)
                    itNet->setPlaced();
            }
        }
    }

    bool NetLayoutJunction::conflict(int ihoriz, int iroad, const NetLayoutJunctionRange& testRng) const
    {
         auto itConflict = mOccupied[ihoriz].find(iroad);
         if (itConflict == mOccupied[ihoriz].end()) return false;
         return itConflict.value().conflict(testRng);
    }

    bool NetLayoutJunction::canJoin(int ihoriz, int iroad, u32 netId, int pos) const
    {
        auto itJoin = mOccupied[ihoriz].find(iroad);
        if (itJoin == mOccupied[ihoriz].end()) return false;
        return itJoin.value().canJoin(netId,pos);
    }

    void NetLayoutJunction::place(int ihoriz, int iroad, const NetLayoutJunctionRange &range)
    {
        mOccupied[ihoriz][iroad].append(range);
        auto netIt = mNetsInput.find(range.netId());
        Q_ASSERT(netIt!=mNetsInput.end());
        netIt->addWire(NetLayoutJunctionWire(ihoriz,iroad,
                                             range.endPosition(0),
                                             range.endPosition(1)));
    }

    void NetLayoutJunction::routeSingleSwap(u32 netId, int iMain, int iroadIn, int iroadOut)
    {
        int iJump = 1-iMain;
        int iroadJump0 = -1;
        int iroadJump1 = maxRoad[iJump];
        int iroadDetour = maxRoad[iMain];
        int maxSearch = std::max(2*(maxRoad[0]+maxRoad[1]),12)*3;
        int count = 0;
        while (count++ < maxSearch) // break when route found
        {
            NetLayoutJunctionRange rngIn(netId, NetLayoutJunctionRange::sMinInf, iroadJump0);
            NetLayoutJunctionRange rngJ0(netId, iroadIn, iroadDetour);
            NetLayoutJunctionRange rngDt(netId, iroadJump0, iroadJump1);
            NetLayoutJunctionRange rngJ1(netId, iroadOut, iroadDetour);
            NetLayoutJunctionRange rngOut(netId, iroadJump1, NetLayoutJunctionRange::sMaxInf);
            if (conflict(iMain,iroadDetour,rngDt))
            {
                iroadDetour ++;
                continue;
            }
            if (conflict(iJump,iroadJump1,rngJ1) ||
                    conflict(iMain,iroadOut,rngOut))
            {
                iroadJump1 ++;
                continue;
            }
            if (conflict(iJump,iroadJump0,rngJ0) ||
                    conflict(iMain,iroadIn,rngIn))
            {
                iroadJump0 --;
                continue;
            }
            place(iMain,iroadIn,rngIn);
            place(iJump,iroadJump0,rngJ0);
            place(iMain,iroadDetour,rngDt);
            place(iJump,iroadJump1,rngJ1);
            place(iMain,iroadOut,rngOut);
            break;
        }
    }

    void NetLayoutJunction::routeSingleStraight(u32 netId, int iMain, int iroadIn, int iroadOut)
    {
        int iJump = 1-iMain;
        if (iroadIn == iroadOut)
        {
            NetLayoutJunctionRange rng(netId,NetLayoutJunctionRange::sMinInf, NetLayoutJunctionRange::sMaxInf);
            place(iMain,iroadIn,rng);
            return;
        }
        int iroadLo = iroadIn;
        int iroadHi = iroadOut;
        int iroadJump = -1;
        int isearchIncr = -1;
        if (iroadIn > iroadOut)
        {
            iroadLo = iroadOut;
            iroadHi = iroadIn;
            iroadJump = maxRoad[iJump];
            isearchIncr = 1;
        }
        int maxSearch = std::max(2*(maxRoad[0]+maxRoad[1]),12);
        for (;;iroadJump+=isearchIncr)
        {
            if (iroadJump <= -maxSearch ||
                    iroadJump >= maxSearch)
            {
                mError = StraightRouteError;
                return;
            }
            NetLayoutJunctionRange rngIn(netId,
                                                 NetLayoutJunctionRange::sMinInf,
                                                 iroadJump);
            NetLayoutJunctionRange rngJump(netId, iroadLo, iroadHi);
            NetLayoutJunctionRange rngOut(netId,
                                                  iroadJump,
                                                  NetLayoutJunctionRange::sMaxInf);

            if (!conflict(iJump,iroadJump,rngJump) &&
                    !conflict(iMain,iroadOut,rngOut) &&
                    !conflict(iMain,iroadIn,rngIn))
            {
                place(iJump,iroadJump,rngJump);
                place(iMain,iroadIn,rngIn);
                place(iMain,iroadOut,rngOut);
                break;
            }
        }
    }

    void NetLayoutJunction::routeSingleCorner(u32 netId, NetLayoutDirection dirHoriz, NetLayoutDirection dirVertic)
    {
        auto netIt = mNetsInput.find(netId);
        Q_ASSERT(netIt != mNetsInput.end());

        int iroadHoriz = netIt.value().roadNumber(dirHoriz);
        int iroadVertic = netIt.value().roadNumber(dirVertic);

        NetLayoutJunctionRange rngH =
                NetLayoutJunctionRange::entryRange(dirHoriz,iroadVertic,netId);
        NetLayoutJunctionRange rngV =
                NetLayoutJunctionRange::entryRange(dirVertic,iroadHoriz,netId);
        if (!conflict(0,iroadHoriz,rngH) && !conflict(1,iroadVertic,rngV))
        {
            netIt->setJunctionPoint(
                        NetLayoutJunctionWireIntersection(
                            true,
                            iroadVertic,
                            iroadHoriz),
                        NetLayoutJunctionWireIntersection::Endpoint);
            mOccupied[0][iroadHoriz].append(rngH);
            mOccupied[1][iroadVertic].append(rngV);
            netIt->setPlaced();
            return;
        }

        int hcroad = dirVertic.isUp()  ? -1 : mEntries.mEntries[dirHoriz.index()].size();
        int hstep  = dirVertic.isUp()  ? -1 : 1;
        int vcroad = dirHoriz.isLeft() ? -1 : mEntries.mEntries[dirVertic.index()].size();
        int vstep  = dirHoriz.isLeft() ? -1 : 1;

        int icount = 0;
        for(;;)
        {
            NetLayoutJunctionRange rngVc(netId,iroadHoriz,hcroad);
            NetLayoutJunctionRange rngHc(netId,iroadVertic,vcroad);
            NetLayoutJunctionRange rngHe =
                    NetLayoutJunctionRange::entryRange(dirHoriz,vcroad,netId);
            NetLayoutJunctionRange rngVe =
                    NetLayoutJunctionRange::entryRange(dirVertic,hcroad,netId);

            if (++icount > 12 && icount > 2*(maxRoad[0]+maxRoad[1]))
            {
                mError = CornerRouteError;
                return;
            }

//            qDebug() << "try" << netId << iroadHoriz << iroadVertic << hcroad << vcroad;

            if (conflict(0,hcroad,rngHc) || conflict(1,iroadVertic,rngVe))
                hcroad += hstep;
            else if (conflict(1,vcroad,rngVc) || conflict(0,iroadHoriz,rngHe))
                vcroad += vstep;
            else
            {
                if(netIt->numberEntries()==3)
                {
                    netIt->setJunctionPoint(
                                NetLayoutJunctionWireIntersection(
                                    true,
                                    vcroad,
                                    iroadHoriz),
                                NetLayoutJunctionWireIntersection::Verify);
                    netIt->setJunctionPoint(
                                NetLayoutJunctionWireIntersection(
                                    true,
                                    iroadVertic,
                                    hcroad),
                                NetLayoutJunctionWireIntersection::Verify);
                }
                place(0,hcroad,rngHc);
                place(1,vcroad,rngVc);
                place(0,iroadHoriz,rngHe);
                place(1,iroadVertic,rngVe);
                mOccupied[1][vcroad].append(rngVc);
                mOccupied[0][iroadHoriz].append(rngHe);
                mOccupied[1][iroadVertic].append(rngVe);
                netIt->setPlaced();
                break;
            }
        }
    }

    NetLayoutJunctionHash::~NetLayoutJunctionHash()
    {
        clearAll();
    }

    void NetLayoutJunctionHash::clearAll()
    {
        for (NetLayoutJunction* nlj : values())
            delete nlj;
        clear();
    }

    NetLayoutJunctionNet::NetLayoutJunctionNet()
        : mPattern(0), mEntries(0), mPlaced(false)
    {
        for (NetLayoutDirection dir(0); !dir.isMax(); ++dir)
            mRoadNumber[dir.index()] = -1;
    }

    void NetLayoutJunctionNet::addEntry(NetLayoutDirection dir, int roadNo)
    {
        mPattern |= dir.toPattern();
        mRoadNumber[dir.index()] = roadNo;
        ++mEntries;
    }

    bool NetLayoutJunctionNet::hasPattern(u32 searchPattern) const
    {
        return (mPattern & searchPattern) == searchPattern;
    }

    void NetLayoutJunctionNet::setJunctionPoint(const NetLayoutJunctionWireIntersection& jp,
            NetLayoutJunctionWireIntersection::PlacementType placement)
    {
        bool found = false;

        switch (placement)
        {
        case NetLayoutJunctionWireIntersection::Verify:
            for (const NetLayoutJunctionWire& wire : mWires)
            {
                if (wire.isEntry() &&
                        ((wire.mRoad == jp.x() && wire.mHorizontal == 1) ||
                         (wire.mRoad == jp.y() && wire.mHorizontal == 0))
                        )
                {
                    found = true;
                    break;
                }
            }
            if (!found) return;
            mJunction = jp;
            break;
        case NetLayoutJunctionWireIntersection::Endpoint:
            for (const NetLayoutJunctionWire& wire : mWires)
            {
                if (wire.isEntry() && wire.mRoad == jp.x() && wire.mHorizontal==1)
                {
                    mJunction = NetLayoutJunctionWireIntersection(true, jp.x(), wire.centralEnd());
                    return;
                }
                if (wire.isEntry() && wire.mRoad == jp.y() && wire.mHorizontal==0)
                {
                    mJunction = NetLayoutJunctionWireIntersection(true, wire.centralEnd(), jp.y());
                    return;
                }
            }
            break;
        default:
            mJunction = jp;
            break;
        }
    }


    QString NetLayoutJunctionNet::toString() const
    {
        QString retval;
        const char* dirChar = "LRUD";
        for (NetLayoutDirection dir(0); !dir.isMax(); ++dir)
        {
            if (mPattern & dir.toPattern())
            {
                if (!retval.isEmpty()) retval += ":";
                retval += QString("%1%2").arg(dirChar[dir.index()]).arg(mRoadNumber[dir.index()]);
            }
        }
        return retval;
    }

    QGraphicsEllipseItem* NetLayoutJunctionWireIntersection::graphicsFactory(u32 netId) const
    {
        float dd = NetLayoutJunctionRange::sSceneDelta / 2.;
        float x0 = NetLayoutJunctionRange::sSceneFirst + x() * NetLayoutJunctionRange::sSceneDelta - dd/2;
        float y0 = NetLayoutJunctionRange::sSceneFirst + y() * NetLayoutJunctionRange::sSceneDelta - dd/2;
        QGraphicsEllipseItem* retval = new QGraphicsEllipseItem(x0,y0,dd,dd);
        retval->setBrush(colorFromId(netId));
        return retval;
    }


    NetLayoutJunctionRange::NetLayoutJunctionRange(u32 netId_, int first, int last)
        : mNetId(netId_), mFirst(first), mLast(last)
    {
        if (last<first)
        {
            mFirst = last;
            mLast  = first;
        }
    }

    NetLayoutJunctionRange NetLayoutJunctionRange::entryRange(NetLayoutDirection dir, int iroad, u32 netId)
    {
        const int first[4] = { sMinInf, iroad, sMinInf, iroad };
        const int last[4]  = { iroad, sMaxInf, iroad, sMaxInf };
        return NetLayoutJunctionRange(netId, first[dir.index()], last[dir.index()]);
    }

    NetLayoutJunctionWire NetLayoutJunctionRange::toWire(int hor, int rd) const
    {
        return NetLayoutJunctionWire(hor, rd, mFirst, mLast);
    }

    bool NetLayoutJunctionRange::canJoin(u32 netId, int pos) const
    {
        if (netId != mNetId) return false;
        return mFirst <= pos && pos <= mLast;
    }

    int NetLayoutJunctionRange::graphFirst() const
    {
        if (mFirst == sMinInf) return sSceneFirst - sSceneGap;
        return mFirst * sSceneDelta + sSceneFirst;
    }

    int NetLayoutJunctionRange::graphLast() const
    {
        if (mLast == sMaxInf) return 1000 + sSceneGap - sSceneFirst;
        return mLast * sSceneDelta + sSceneFirst;
    }

    bool NetLayoutJunctionRange::conflict(const NetLayoutJunctionRange& other) const
    {
        if (mNetId == other.mNetId) return false;
        if (other.mLast < mFirst || mLast < other.mFirst) return false;
        return true;
    }

    int NetLayoutJunctionRange::endPosition(int inx) const
    {
        if (!inx) return mFirst;
        return mLast;
    }

    bool NetLayoutJunctionOccupied::canJoin(u32 netId, int pos) const
    {
        for (const NetLayoutJunctionRange& r : *this)
            if (r.canJoin(netId, pos)) return true;
        return false;
    }

    bool NetLayoutJunctionOccupied::conflict(const NetLayoutJunctionRange& test) const
    {
        for (const NetLayoutJunctionRange& r : *this)
            if (r.conflict(test)) return true;
        return false;
    }

    int NetLayoutJunctionWire::centralEnd() const
    {
        if (mFirst==NetLayoutJunctionRange::sMinInf)
            return mLast;
        return mFirst;
    }

    bool NetLayoutJunctionWire::isEntry() const
    {
        return mFirst == NetLayoutJunctionRange::sMinInf
                || mLast == NetLayoutJunctionRange::sMaxInf;
    }

    NetLayoutJunctionWireIntersection NetLayoutJunctionWire::intersection(const NetLayoutJunctionWire &other) const
    {
        if (other.mFirst <= mRoad && mRoad <= other.mLast &&
                mFirst <= other.mRoad && other.mRoad <= mLast)
        {
            if (mHorizontal)
                return NetLayoutJunctionWireIntersection(true,mRoad,other.mRoad);
            return NetLayoutJunctionWireIntersection(true,other.mRoad,mRoad);
        }
        return NetLayoutJunctionWireIntersection();
    }

    QString NetLayoutJunctionEntries::dump() const
    {
        QString retval;
        for (NetLayoutDirection dir(0); !dir.isMax(); ++dir)
        {
            retval += QString("  entries.mEntries[%1]").arg(dir.index());
            for (u32 id : mEntries[dir.index()])
                retval += QString(" << %1").arg(id);
            retval += ";\n";
        }
        return retval;
    }

    void NetLayoutJunctionEntries::dumpFile(const NetLayoutPoint& pnt) const
    {
        QString filename("/tmp/junction");
        QDir().mkpath(filename);
        QString columnStr;
        if (pnt.x() >= 26) columnStr += QString("%1").arg((char)('A'+ pnt.x()/26 - 1));
        columnStr += QString("%1").arg((char)('A'+ pnt.x()%26));
        filename += "/" + columnStr + QString("%1.jjj").arg(pnt.y());
        QFile ff(filename);
        if (!ff.open(QIODevice::WriteOnly)) return;
        for (NetLayoutDirection dir(0); !dir.isMax(); ++dir)
        {
            for (u32 id : mEntries[dir.index()])
                ff.write(" " + QByteArray::number(id));
            ff.write("\n");
        }
    }

    void NetLayoutJunctionEntries::setEntries(NetLayoutDirection dir, const QList<u32>& entries_)
    {
        mEntries[dir.index()] = entries_;
    }
} // namespace hal
