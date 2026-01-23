#include "gui/graph_widget/layouters/net_layout_junction.h"
#ifdef JUNCTION_DEBUG
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#endif
#include <QBrush>
#include <QPen>
#include <QSet>
#include <QMultiMap>
#include <stdlib.h>
#include <QDebug>
#include <QPoint>
#include <QDir>
#include <QDateTime>
#include "hal_core/netlist/project_manager.h"

#ifdef JUNCTION_DEBUG
QMap<u32,QColor> debugColorMap;

QColor colorFromId(u32 netId)
{
    QColor retval = debugColorMap.value(netId);
    if (!retval.isValid())
        return QColor("white");
    return retval;
}

void setColorMap(const hal::NetLayoutJunctionEntries& entries)
{
    debugColorMap.clear();

    QMap<u32,int> pattern;
    for (int i=0; i<4; i++)
    {
        int mask = 1 << i;
        for (u32 id : entries.mEntries[i])
            pattern[id] |= mask;
    }
    QSet<int> danglingWire = { 1, 2, 4, 8};
    int count = 0;

    for (auto it = pattern.begin(); it != pattern.end(); ++it)
    {
        if (danglingWire.contains(it.value()))
        {
            debugColorMap[it.key()] = QColor::fromRgb(23,25,29);
            continue;
        }

        int hue = count++ * 25;
        if (hue > 255)
        {
            int cc = (count-10)%20;
            int cd = (count-10)/20;
            int h = cc * 10;
            int s = 80 - cd*40;
            int v = 255;
            qDebug() << cc << cd << h << s << v;
            if (s <= 0)
            {
                int g = (20*count) % 128 + 128;
                debugColorMap[it.key()] = QColor::fromRgb(g,g,g);
            }
            else
                debugColorMap[it.key()] = QColor::fromHsv(h,s,v);
        }
        else
            debugColorMap[it.key()] = QColor::fromHsv(hue,255,255);
    }
}

QGraphicsEllipseItem* getEllipse(QPoint p, u32 netId)
{
    float dd = DELTA / 2.;
    float x0 = FIRST + p.x() * DELTA - dd/2;
    float y0 = FIRST + p.y() * DELTA - dd/2;
    QGraphicsEllipseItem* retval = new QGraphicsEllipseItem(x0,y0,dd,dd);
    retval->setBrush(colorFromId(netId));
    return retval;
}
#endif

namespace hal {

    NetLayoutJunction::NetLayoutJunction(const NetLayoutJunctionEntries& entries)
        : mEntries(entries), mError(Ok)
    {
        memset(maxRoad,0,sizeof(maxRoad));
        for (NetLayoutDirection dir(0); !dir.isMax(); ++dir)
        {
            int iHoriz = dir.iHorizontal();
            int n = mEntries.mEntries[dir.index()].size();
            if (n>maxRoad[iHoriz]) maxRoad[iHoriz] = n;
            for (int i=0; i<n; i++)
            {
//                qDebug() << "entry***" << i << n << dir.index() << mEntries.id(dir,i);
                mNets[mEntries.id(dir,i)].addEntry(dir,i);
            }
        }
        routeAllStraight(0,1);
        routeAllStraight(2,3);
        /*
        for (auto netIt=mNets.begin(); netIt!=mNets.end(); ++netIt)
            if (netIt.value().numberEntries()==4)
                fourWayJunctions(netIt);
                */
        routeT();
        routeAllCorner(0,2);
        routeAllCorner(0,3);
        routeAllCorner(1,2);
        routeAllCorner(1,3);
        routeAllMultiPin(0);
        routeAllMultiPin(1);
        findJunctions();
        calculateRect();

    }

    void NetLayoutJunction::findJunctions()
    {
        for (auto it = mNets.begin(); it!= mNets.end(); ++it)
        {
            QList<NetLayoutJunctionWire> hNets, vNets;
            for (const NetLayoutJunctionWire& nljw : it->mWires )
                if (nljw.mIndex.hvIndex() == LaneIndex::Horizontal)
                    hNets.append(nljw);
                else
                    vNets.append(nljw);

            for (const NetLayoutJunctionWire& hn : hNets)
                for (const NetLayoutJunctionWire& vn : vNets)
                    if ((hn.mRange.innerPos(vn.mIndex.laneIndex()) && vn.mRange.contains(hn.mIndex.laneIndex())) ||
                            (hn.mRange.contains(vn.mIndex.laneIndex()) && vn.mRange.innerPos(hn.mIndex.laneIndex())))
                        it->mKnots.append(QPoint(vn.mIndex.laneIndex(),hn.mIndex.laneIndex()));
        }
    }

    void NetLayoutJunction::routeAllMultiPin(NetLayoutDirection leftOrRight)
    {
        int nEntries = mEntries.mEntries[leftOrRight.index()].size();

        QHash<u32,NetLayoutJunctionMultiPin> multiPinHash;

        for (int iroad=0; iroad<nEntries; iroad++)
        {
            LaneIndex rinx(LaneIndex::Horizontal,iroad);
            bool isRouted = false;
            u32 id = mEntries.mEntries[leftOrRight.index()].at(iroad);
            auto itNet = mNets.find(id);
            Q_ASSERT(itNet != mNets.end());

            if (itNet.value().pattern() == (u32) (1 << leftOrRight.index() ))
            {
                /// dangling wire
                isRouted = true;
            }
            else
            {
                /// nets should be routed to junction entry by now ...
                for (const NetLayoutJunctionRange& rng : mOccupied.value(rinx))
                {
                    if (rng.isEntry(leftOrRight.index()))
                    {
                        isRouted = true;
                        break;
                    }
                    // ... unless in the multipin case (isRouted=false)
                }
            }

            if (isRouted)
                multiPinHash[id].mLane = iroad;
            else
                multiPinHash[id].mConnector.append(iroad);
        }

        for (auto it = multiPinHash.begin(); it!= multiPinHash.end(); ++it)
        {
            if (it.value().mConnector.isEmpty()) continue;
//            qDebug() << leftOrRight.index() << "MultiPin" << it.key() << it.value().mRoad << it.value().mConnector;
            routeSingleMultiPin(it.key(), leftOrRight, it.value());
        }


    }

    void NetLayoutJunction::routeSingleMultiPin(u32 netId, NetLayoutDirection leftOrRight, const NetLayoutJunctionMultiPin& nmpin)
    {
        int ymin = nmpin.mLane;
        int ymax = nmpin.mLane;
        for (int iroad : nmpin.mConnector)
        {
            if (iroad < ymin) ymin = iroad;
            if (iroad > ymax) ymax = iroad;
        }

        int x0, x1, dx;
        if (leftOrRight.isLeft())
        {
            x0 = -1;
            x1 = NetLayoutJunctionRange::MinInf;
            dx = -1;
        }
        else
        {
            x0 = maxRoad[NetLayoutDirection::Right];
            x1 = NetLayoutJunctionRange::MaxInf;
            dx = 1;
        }

        for (int x=x0; x!=x1; x+=dx)
        {
            QList<int> roads;
            QList<NetLayoutJunctionRange> rngs;
            roads.append(x); // vertical first
            rngs.append(NetLayoutJunctionRange(netId,ymin,ymax));
            for (int iroad : nmpin.mConnector)
            {
                roads.append(iroad);
                if (leftOrRight.isLeft())
                    rngs.append(NetLayoutJunctionRange(netId,x1,x));
                else
                    rngs.append(NetLayoutJunctionRange(netId,x,x1));
            }
            bool hasConflict = false;

            // store connecting vertical road at i=0
            for (int i=0; i<roads.size();i++)
            {
                if (conflict(LaneIndex(i?LaneIndex::Horizontal:LaneIndex::Vertical,roads.at(i)),rngs.at(i)))
                {
                    hasConflict = true;
                    break;
                }
            }
            if (!hasConflict)
            {
                for (int i=0; i<roads.size();i++)
                    place(LaneIndex(i?LaneIndex::Horizontal:LaneIndex::Vertical,roads.at(i)),rngs.at(i));
                break;
            }
        }
    }

    void NetLayoutJunction::calculateRect()
    {
        int x0, y0, x1, y1;
        x0 = y0 = x1 = y1 = 0;
        int x,y;

        for (LaneIndex ri : mOccupied.keys())
        {
            switch (ri.hvIndex())
            {
            case LaneIndex::Horizontal:
                y = ri.laneIndex();
                if (y<y0)   y0 = y;
                if (y+1>y1) y1 = y+1;
                break;
            case LaneIndex::Vertical:
                x = ri.laneIndex();
                if (x<x0)   x0 = x;
                if (x+1>x1) x1 = x+1;
                break;
            }
        }

        // empty junction has size 1
        if (y1<=0) y1 = 1;
        if (x1<=0) x1 = 1;
        mRect = QRect(x0,y0,x1-x0,y1-y0);

        //TODO: truncate nets to rect might be range function
        for (auto itNet = mNets.begin(); itNet != mNets.end(); ++itNet)
        {
            auto itWire = itNet->mWires.begin();
            while (itWire != itNet->mWires.end())
            {
                if (itWire->mRange.first() == NetLayoutJunctionRange::MinInf)
                {
                    if (itWire->mIndex.hvIndex()==LaneIndex::Horizontal)
                        itWire->mRange.setFirst(x0);
                    else
                        itWire->mRange.setFirst(y0);
                }
                if (itWire->mRange.last() == NetLayoutJunctionRange::MaxInf)
                {
                    if (itWire->mIndex.hvIndex()==LaneIndex::Horizontal)
                        itWire->mRange.setLast(x1-1);
                    else
                        itWire->mRange.setLast(y1-1);
                }
                if (itWire->mRange.length() <=0)
                    itWire = itNet->mWires.erase(itWire);
                else
                    ++itWire;
            }
        }
    }

#ifdef JUNCTION_DEBUG
    void NetLayoutJunction::toScene(QGraphicsScene* scene) const
    {
        scene->setBackgroundBrush(QBrush(QColor::fromRgb(80,80,80)));
        QRectF bg(FIRST + (mRect.x() - 0.5) * DELTA,
                  FIRST + (mRect.y() - 0.5) * DELTA,
                  mRect.width() * DELTA,
                  mRect.height() * DELTA);

        float xscene0 = bg.left() < 100 ? bg.left() - 100 : 0;
        float xscene1 = bg.right() > 900 ? bg.right() + 100 : 1000;
        float yscene0 = bg.top() < 100 ? bg.top() - 100 : 0;
        float yscene1 = bg.bottom() > 900 ? bg.bottom() + 100 : 1000;
        scene->setSceneRect(xscene0,yscene0,xscene1,yscene1);

        QGraphicsRectItem* item = new QGraphicsRectItem(bg);
        item->setBrush(QBrush(Qt::black));
        scene->addItem(item);

        for (auto netIt = mNets.constBegin(); netIt != mNets.constEnd(); ++netIt)
        {
            for(QPoint jp : netIt.value().junctionPoints())
            {
                QGraphicsEllipseItem* item = getEllipse(jp,netIt.key());
                scene->addItem(item);
            }
        }


        for (NetLayoutDirection dir(0); !dir.isMax(); ++dir)
        {
            int nEntries = mEntries.mEntries[dir.index()].size();
            for (int ientry=0; ientry<nEntries; ientry++)
            {
                u32 id = mEntries.mEntries[dir.index()].at(ientry);
                QColor col = colorFromId(id);
                float qEntry = FIRST + ientry * DELTA;
                if (id==0) continue;

                QGraphicsLineItem* item = nullptr;
                switch (dir.direction()) {
                case NetLayoutDirection::Left:
                    item = new QGraphicsLineItem(xscene0, qEntry, bg.left(), qEntry);
                    break;
                case NetLayoutDirection::Right:
                    item = new QGraphicsLineItem(bg.right(), qEntry,xscene1, qEntry);
                    break;
                case NetLayoutDirection::Up:
                    item = new QGraphicsLineItem(qEntry, yscene0, qEntry, bg.top());
                    break;
                case NetLayoutDirection::Down:
                    item = new QGraphicsLineItem(qEntry, bg.bottom(), qEntry, yscene1);
                    break;
                default:
                    break;
                }
                item->setPen(QPen(QBrush(col),3.));
                scene->addItem(item);
            }
        }        
    }

    void NetLayoutJunction::toSceneStep(QGraphicsScene* scene, int istep)
    {
        if (istep >= mOccupied.mHistory.size()) return;

        QRectF bg(FIRST + (mRect.x() - 0.5) * DELTA,
                  FIRST + (mRect.y() - 0.5) * DELTA,
                  mRect.width() * DELTA,
                  mRect.height() * DELTA);

        const QPair<u32,NetLayoutJunctionWire>& pw = mOccupied.mHistory.at(istep);
        QColor col = colorFromId(pw.first);

        float x0, x1, y0, y1;

        const LaneIndex& ri = pw.second.mIndex;
        const NetLayoutJunctionRange& rng = pw.second.mRange;
        if (ri.hvIndex()==LaneIndex::Horizontal)
        {
            y0 = y1 = FIRST + ri.laneIndex() * DELTA;
            x0 = FIRST + rng.first() * DELTA;
            if (x0 < bg.left()) x0 = bg.left();
            x1 = FIRST + rng.last() * DELTA;
            if (x1 > bg.right()) x1 = bg.right();
            qDebug() << "---" << ri.laneIndex() << rng.first() << rng.last();
        }
        else
        {
            x0 = x1 = FIRST + ri.laneIndex() * DELTA;
            y0 = FIRST + rng.first() * DELTA;
            if (y0 < bg.top()) y0 = bg.top();
         y1 = FIRST + rng.last() * DELTA;
            if (y1 > bg.bottom()) y1 = bg.bottom();
            qDebug() << " | " << ri.laneIndex() << rng.first() << rng.last();
        }
        QGraphicsLineItem* item = new QGraphicsLineItem(x0,y0,x1,y1);
        item->setPen(QPen(QBrush(col),3.));
        scene->addItem(item);
    }
#endif

    void NetLayoutJunction::routeT()
    {
        for (auto itNet=mNets.begin(); itNet!=mNets.end();++itNet)
        {
            if (itNet.value().numberEntries() >= 3)
            {
                int i0, i1, ilink;
                LaneIndex::HVIndex itravers, imaindir;
                u32 reducedPattern;
                switch (itNet.value().pattern())
                {
                case 7:    // L U R
                    i0 = 0;
                    i1 = 1;
                    ilink = 2;
                    itravers = LaneIndex::Vertical;
                    reducedPattern = itNet.value().laneIndex(i1) > itNet.value().laneIndex(i0) ? 5 : 6;
                    break;
                case 11:    // L D R
                    i0 = 0;
                    i1 = 1;
                    ilink = 3;
                    itravers = LaneIndex::Vertical;
                    reducedPattern = itNet.value().laneIndex(i1) > itNet.value().laneIndex(i0) ? 10 : 9;
                    break;
                case 13:   //  U L D
                    i0 = 2;
                    i1 = 3;
                    ilink = 0;
                    itravers = LaneIndex::Horizontal;
                    reducedPattern = itNet.value().laneIndex(i1) > itNet.value().laneIndex(i0) ? 5 : 9;
                    break;
                case 14:   //  U R D
                    i0 = 2;
                    i1 = 3;
                    ilink = 1;
                    itravers = LaneIndex::Horizontal;
                    reducedPattern = itNet.value().laneIndex(i1) > itNet.value().laneIndex(i0) ? 10 : 6;
                    break;
                default:
                    continue;
                }

                int roadLink = itNet.value().laneIndex(ilink);
                imaindir = (itravers == LaneIndex::Horizontal ? LaneIndex::Vertical : LaneIndex::Horizontal);
                QList<NetLayoutJunctionWire> mainWires = itNet.value().wireAtPos(roadLink,imaindir);
                if (mainWires.isEmpty())
                {
                    qDebug() << "T-join not found";
                    continue;
                }

                int minlen = 0;
                NetLayoutJunctionRange rngT = NetLayoutJunctionRange::entryRange(ilink,0,itNet.key());

                for (const NetLayoutJunctionWire& nljw : mainWires)
                {
                    NetLayoutJunctionRange rng =
                        NetLayoutJunctionRange::entryRange(ilink,nljw.mIndex.laneIndex(),itNet.key());
                    if (!minlen || rng.length() < rngT.length())
                    {
                        rngT = rng;
                        minlen = rng.length();
                    }
                }

                if (conflict(LaneIndex(itravers,roadLink),rngT))
                    itNet->setPattern(reducedPattern);
                else
                {
                    place(LaneIndex(itravers,roadLink),rngT);
                    itNet->setPlaced();
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
                NetLayoutJunctionNet net = mNets.value(*it);
                if (net.hasPattern(searchPattern) && !net.isPlaced())
                    routeSingleCorner(*it,dirHoriz,dirVertic);
            }
        else
            for (auto it = mEntries.mEntries[dirHoriz.index()].rbegin();
                 it != mEntries.mEntries[dirHoriz.index()].rend(); ++it)
            {
                NetLayoutJunctionNet net = mNets.value(*it);
                if (net.hasPattern(searchPattern) && !net.isPlaced())
                    routeSingleCorner(*it,dirHoriz,dirVertic);
            }
    }

    void NetLayoutJunction::routeAllStraight(NetLayoutDirection dirFrom, NetLayoutDirection dirTo)
    {
        QMap<int,int> straightConnected;
        u32 searchPattern = dirFrom.toPattern() | dirTo.toPattern();

        for (u32 netId : mEntries.mEntries[dirFrom.index()])
        {
            auto itNet = mNets.find(netId);
            Q_ASSERT(itNet != mNets.end());
//            qDebug () << "straight" << searchPattern << netId << itNet.value().hasPattern(searchPattern) << itNet.value().pattern();
            if (itNet.value().hasPattern(searchPattern))
            {
                int iroadIn  = itNet.value().laneIndex(dirFrom);
                int iroadOut = itNet.value().laneIndex(dirTo);
                if (straightConnected.contains(iroadIn) && straightConnected.contains(iroadOut))
                    routeSingleSwap(netId, (searchPattern==3?0:1), iroadIn, iroadOut);
                else
                    routeSingleStraight(netId, (searchPattern==3?0:1), iroadIn, iroadOut);
                ++straightConnected[iroadIn];
                ++straightConnected[iroadOut];
                if (itNet.value().numberEntries() % 2 ==0)
                    itNet->setPlaced();
            }
        }
    }

    bool NetLayoutJunction::conflict(const LaneIndex&ri, const NetLayoutJunctionRange& testRng) const
    {
         auto itConflict = mOccupied.find(ri);
         if (itConflict == mOccupied.end()) return false;
         return itConflict.value().conflict(testRng);
    }

    bool NetLayoutJunction::canJoin(const LaneIndex &ri, u32 netId, int pos) const
    {
        auto itJoin = mOccupied.find(ri);
        if (itJoin == mOccupied.end()) return false;
        return itJoin.value().canJoin(netId,pos);
    }

    void NetLayoutJunction::place(const LaneIndex &ri, const NetLayoutJunctionRange &range)
    {
        NetLayoutJunctionOccupiedHash::AddOrMerge aom = mOccupied.addOrMerge(ri,range);

        auto netIt = mNets.find(range.netId());
        Q_ASSERT(netIt!=mNets.end());
        switch (aom.mType )
        {
        case NetLayoutJunctionOccupiedHash::AddOrMerge::Added:
        {
            NetLayoutJunctionWire nljw(range,ri);
            netIt->addWire(nljw);
            break;
        }
        case NetLayoutJunctionOccupiedHash::AddOrMerge::Merged:
        {
            NetLayoutJunctionWire nljw(*aom.mNewRange,ri);
            netIt->replaceWire(*aom.mOldRange,nljw);
            break;
        }
        default:
            break;
        }
    }

    void NetLayoutJunction::routeSingleSwap(u32 netId, int iMain, int iroadIn, int iroadOut)
    {
        int iJump = 1-iMain;
        int iroadJump0 = -1;
        int iroadJump1 = maxRoad[iJump];
        int iroadDetour = maxRoad[iMain];

        LaneIndex riDetour(iMain?LaneIndex::Vertical:LaneIndex::Horizontal,iroadDetour);
        LaneIndex riJump0(iJump?LaneIndex::Vertical:LaneIndex::Horizontal,iroadJump0);
        LaneIndex riJump1(iJump?LaneIndex::Vertical:LaneIndex::Horizontal,iroadJump1);
        LaneIndex riMainIn(iMain?LaneIndex::Vertical:LaneIndex::Horizontal,iroadIn);
        LaneIndex riMainOut(iMain?LaneIndex::Vertical:LaneIndex::Horizontal,iroadOut);

        for (;;) // break when route found
        {
            NetLayoutJunctionRange rngIn(netId, NetLayoutJunctionRange::MinInf, riJump0.laneIndex());
            NetLayoutJunctionRange rngJ0(netId, iroadIn, riDetour.laneIndex());
            NetLayoutJunctionRange rngDt(netId, riJump0.laneIndex(), riJump1.laneIndex());
            NetLayoutJunctionRange rngJ1(netId, iroadOut, riDetour.laneIndex());
            NetLayoutJunctionRange rngOut(netId, riJump1.laneIndex(), NetLayoutJunctionRange::MaxInf);


            if (conflict(riDetour,rngDt))
            {
                ++riDetour;
                continue;
            }
            if (conflict(riJump1,rngJ1) ||
                    conflict(riMainOut,rngOut))
            {
                ++riJump1;
                continue;
            }
            if (conflict(riJump0,rngJ0) ||
                    conflict(riMainIn,rngIn))
            {
                --riJump0;
                continue;
            }
            place(riMainIn,rngIn);
            place(riJump0,rngJ0);
            place(riDetour,rngDt);
            place(riJump1,rngJ1);
            place(riMainOut,rngOut);
            break;
        }
    }

    void NetLayoutJunction::routeSingleStraight(u32 netId, int iMain, int iroadIn, int iroadOut)
    {   
        int iJump = 1-iMain;
        if (iroadIn == iroadOut)
        {
            NetLayoutJunctionRange rng(netId,NetLayoutJunctionRange::MinInf, NetLayoutJunctionRange::MaxInf);
            place(LaneIndex(iMain?LaneIndex::Vertical:LaneIndex::Horizontal,iroadIn),rng);
            return;
        }
        LaneIndex riMainIn(iMain?LaneIndex::Vertical:LaneIndex::Horizontal,iroadIn);
        LaneIndex riMainOut(iMain?LaneIndex::Vertical:LaneIndex::Horizontal,iroadOut);

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

        // check whether the incoming lane is occupied at some point
        //    in this case we must jump to outgoing lane before this point
        auto itMightBeOccupied = mOccupied.find(riMainIn);
        if (itMightBeOccupied != mOccupied.constEnd())
        {
            int iOccupiedFromHere = NetLayoutJunctionRange::MaxInf;
            for (const NetLayoutJunctionRange& nljr : (*itMightBeOccupied))
            {
                isearchIncr = -1;
                if (nljr.first() < iOccupiedFromHere)
                {
                    iOccupiedFromHere = nljr.first();
                    iroadJump = iOccupiedFromHere - 1;
                }
            }
        }

        for (;;iroadJump+=isearchIncr)
        {
            if (iroadJump <= NetLayoutJunctionRange::MinInf ||
                    iroadJump >= NetLayoutJunctionRange::MaxInf)
            {
                qDebug() << "cannot route straight" << (iMain ? "vertical" : "horizontal")
                         << netId << iroadIn << iroadOut;
                return;
            }
            NetLayoutJunctionRange rngIn(netId,
                                                 NetLayoutJunctionRange::MinInf,
                                                 iroadJump);
            NetLayoutJunctionRange rngJump(netId, iroadLo, iroadHi);
            NetLayoutJunctionRange rngOut(netId,
                                                  iroadJump,
                                                  NetLayoutJunctionRange::MaxInf);

            LaneIndex riJump(iJump?LaneIndex::Vertical:LaneIndex::Horizontal,iroadJump);
            if (!conflict(riJump,rngJump) &&
                    !conflict(riMainOut,rngOut) &&
                    !conflict(riMainIn,rngIn))
            {
                place(riJump,rngJump);
                place(riMainIn,rngIn);
                place(riMainOut,rngOut);
                break;
            }
        }
    }

    NetLayoutJunctionOccupiedHash::AddOrMerge NetLayoutJunctionOccupiedHash::addOrMerge(const LaneIndex& ri, const NetLayoutJunctionRange &rng)
    {
        // operator will create entry if not existing
        NetLayoutJunctionOccupied& nljo = this->operator[](ri);

        AddOrMerge retval;

        for (auto it = nljo.begin(); it!= nljo.end(); ++it)
        {
            if (*it == rng)
            {
                retval.mType = AddOrMerge::AlreadyExisting;
                break;
            }
            if (it->canJoin(rng))
            {
                retval.mOldRange = new NetLayoutJunctionRange(*it);
                it->expand(rng);
                retval.mNewRange = new NetLayoutJunctionRange(*it);
                retval.mType = AddOrMerge::Merged;
                break;
            }
        }

        if (retval.mType == AddOrMerge::Added)
            nljo.append(rng);

#ifdef JUNCTION_DEBUG
        mHistory.append(qMakePair(rng.netId(),NetLayoutJunctionWire(rng,ri)));
#endif
        return retval;
    }

    void NetLayoutJunction::routeSingleCorner(u32 netId, NetLayoutDirection dirHoriz, NetLayoutDirection dirVertic)
    {
        auto netIt = mNets.find(netId);
        Q_ASSERT(netIt != mNets.end());

        int iroadHoriz = netIt.value().laneIndex(dirHoriz);
        int iroadVertic = netIt.value().laneIndex(dirVertic);

        // Try to connect straight lines directly without detour
        NetLayoutJunctionRange rngH =
                NetLayoutJunctionRange::entryRange(dirHoriz,iroadVertic,netId);
        NetLayoutJunctionRange rngV =
                NetLayoutJunctionRange::entryRange(dirVertic,iroadHoriz,netId);
        if (!conflict(LaneIndex::horizontal(iroadHoriz),rngH) && !conflict(LaneIndex::vertical(iroadVertic),rngV))
        {
            place(LaneIndex::horizontal(iroadHoriz),rngH);
            place(LaneIndex::vertical(iroadVertic),rngV);
            netIt->setPlaced();
            return;
        }

        // OK, we need around corner detour
        int hcroad = dirVertic.isUp()  ? -1 : mEntries.mEntries[dirHoriz.index()].size();
        int hstep  = dirVertic.isUp()  ? -1 : 1;
        int vcroad = dirHoriz.isLeft() ? -1 : mEntries.mEntries[dirVertic.index()].size();
        int vstep  = dirHoriz.isLeft() ? -1 : 1;

        int icount = 0;
        int tryMax = 2 * mOccupied.size();
        for (int i=0; i<4; i++)
            tryMax += 2* mEntries.mEntries[i].size();
        for(;;)
        {
            NetLayoutJunctionRange rngVc(netId,iroadHoriz,hcroad);
            NetLayoutJunctionRange rngHc(netId,iroadVertic,vcroad);
            NetLayoutJunctionRange rngHe =
                    NetLayoutJunctionRange::entryRange(dirHoriz,vcroad,netId);
            NetLayoutJunctionRange rngVe =
                    NetLayoutJunctionRange::entryRange(dirVertic,hcroad,netId);

            if (++icount > tryMax)
            {
                qDebug() << "giving up";
                mError = CornerRouteError;
                return;
            }

//            qDebug() << "try" << netId << iroadHoriz << iroadVertic << hcroad << vcroad;

            if (conflict(LaneIndex::horizontal(hcroad),rngHc) || conflict(LaneIndex::vertical(iroadVertic),rngVe))
                hcroad += hstep;
            else if (conflict(LaneIndex::vertical(vcroad),rngVc) || conflict(LaneIndex::horizontal(iroadHoriz),rngHe))
                vcroad += vstep;
            else
            {
                place(LaneIndex::horizontal(hcroad),rngHc);
                place(LaneIndex::vertical(vcroad),rngVc);

                place(LaneIndex::horizontal(iroadHoriz),rngHe);
                place(LaneIndex::vertical(iroadVertic),rngVe);
                netIt->setPlaced();
                break;
            }
        }
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
            mLaneIndex[dir.index()] = -1;
    }

    void NetLayoutJunctionNet::addEntry(NetLayoutDirection dir, int laneInx)
    {
        mPattern |= dir.toPattern();
        mLaneIndex[dir.index()] = laneInx;
        ++mEntries;
    }

    bool NetLayoutJunctionNet::hasPattern(u32 searchPattern) const
    {
        return (mPattern & searchPattern) == searchPattern;
    }

    void NetLayoutJunctionNet::replaceWire(const NetLayoutJunctionRange &rng, const NetLayoutJunctionWire &wire)
    {
        for (auto it = mWires.begin(); it!= mWires.end(); ++it)
            if (rng == *it)
            {
                *it = wire;
                return;
            }
    }

    QList<NetLayoutJunctionWire> NetLayoutJunctionNet::wireAtPos(int pos, LaneIndex::HVIndex hvi)
    {
        QList<NetLayoutJunctionWire> retval;
        for (auto it = mWires.begin(); it!= mWires.end(); ++it)
            if (it->mIndex.hvIndex() == hvi && it->mRange.contains(pos))
                retval.append(*it);
        return retval;
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
                retval += QString("%1%2").arg(dirChar[dir.index()]).arg(mLaneIndex[dir.index()]);
            }
        }
        return retval;
    }

    NetLayoutJunctionRange::NetLayoutJunctionRange(u32 netId_, int first, int last)
        : mNetId(netId_), mFirst(first), mLast(last)
    {
        if (last < first)
        {
            mFirst = last;
            mLast  = first;
        }
    }

    NetLayoutJunctionRange NetLayoutJunctionRange::entryRange(NetLayoutDirection dir, int ilane, u32 netId)
    {
        const int first[4] = { MinInf, ilane, MinInf, ilane };
        const int last[4]  = { ilane, MaxInf, ilane, MaxInf };
        return NetLayoutJunctionRange(netId, first[dir.index()], last[dir.index()]);
    }

    bool NetLayoutJunctionRange::canJoin(const NetLayoutJunctionRange& other) const
    {
        if (mNetId != other.mNetId) return false;
        if (mFirst > other.mLast) return false;
        if (other.mFirst > mLast) return false;
        return true;
    }

    bool NetLayoutJunctionRange::operator==(const NetLayoutJunctionRange& other) const
    {
        return (mNetId == other.mNetId &&
                mFirst == other.mFirst &&
                mLast  == other.mLast);
    }

    bool NetLayoutJunctionRange::operator==(const NetLayoutJunctionWire& wire) const
    {
        return (mFirst == wire.mRange.mFirst && mLast == wire.mRange.mLast);
    }


    bool NetLayoutJunctionRange::conflict(const NetLayoutJunctionRange& other) const
    {
        if (mNetId == other.mNetId) return false;
        if (other.mLast < mFirst || mLast < other.mFirst) return false;
        return true;
    }

    int NetLayoutJunctionRange::endPosition(int iGetLast) const
    {
        if (!iGetLast) return mFirst;
        return mLast;
    }

    bool NetLayoutJunctionRange::isEntry(int iTestMax) const
    {
        if (iTestMax) return (mLast == MaxInf);
        return              (mFirst == MinInf);
    }

    void NetLayoutJunctionRange::expand(const NetLayoutJunctionRange &other)
    {
        if (other.mFirst < mFirst) mFirst = other.mFirst;
        if (other.mLast  > mLast)  mLast  = other.mLast;
    }

    bool NetLayoutJunctionOccupied::canJoin(u32 netId, int pos) const
    {
        for (const NetLayoutJunctionRange& r : *this)
        {
            if (r.netId() != netId) continue;
            if (r.contains(pos)) return true;
        }
        return false;
    }

    bool NetLayoutJunctionOccupied::conflict(const NetLayoutJunctionRange& test) const
    {
        for (const NetLayoutJunctionRange& r : *this)
            if (r.conflict(test)) return true;
        return false;
    }

    bool NetLayoutJunctionWire::isEntry() const
    {
        return mRange.isEntry(0) || mRange.isEntry(1);
    }

    bool NetLayoutJunctionEntries::isTrivial() const
    {
        if (mEntries[NetLayoutDirection::Left].isEmpty() && mEntries[NetLayoutDirection::Right].isEmpty())
            return mEntries[NetLayoutDirection::Up] == mEntries[NetLayoutDirection::Down];
        if (mEntries[NetLayoutDirection::Up].isEmpty() && mEntries[NetLayoutDirection::Down].isEmpty())
            return mEntries[NetLayoutDirection::Left] == mEntries[NetLayoutDirection::Right];
        return false;
    }

    void NetLayoutJunctionEntries::dumpToFile(const QPoint &pnt) const
    {
        QFile ff(QString::fromStdString(ProjectManager::instance()->get_project_directory().get_filename("junction_data.txt").string()));
        if (!ff.open(QIODevice::WriteOnly | QIODevice::Append)) return;
        QTextStream xout(&ff);
        xout << "(" << pnt.x() << "," << pnt.y() << ")\n";
        for (NetLayoutDirection dir(0); !dir.isMax(); ++dir)
        {
            for (u32 id : mEntries[dir.index()])
                xout << " " << id;
            xout << "\n";
        }
        xout.flush();
    }

    void NetLayoutJunctionEntries::resetFile()
    {
        QFile ff(QString::fromStdString(ProjectManager::instance()->get_project_directory().get_filename("junction_data.txt").string()));
        if (!ff.open(QIODevice::WriteOnly)) return;
        ff.write(QDateTime::currentDateTime().toLocalTime().toString("--- dd.MM.yyyy hh:mm:ss ---\n").toUtf8());
    }

    QString NetLayoutJunctionEntries::toString() const
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

    uint qHash(const LaneIndex& ri) { return ri.mIndex; }
} // namespace hal
