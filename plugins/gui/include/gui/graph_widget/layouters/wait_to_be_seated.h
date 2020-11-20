#pragma once

#include <QList>
#include <QMap>
#include <QSet>
#include <QPoint>
#include "gui/gui_def.h"

namespace hal
{
   class WaitToBeSeatedList;
   class WaitToBeSeatedWaiting;

   class WaitToBeSeatedEntry
   {
       friend class WaitToBeSeatedList;
       friend class WaitToBeSeatedWaiting;
       Node mNode;
       QList<QPoint> mPredecessorPositions;
       QSet<WaitToBeSeatedEntry*> mPredecessorSet;
       QSet<WaitToBeSeatedEntry*> mSuccessorSet;

       void setPredecessorIds(const QMap<u32,WaitToBeSeatedEntry*>& gateMap);

   public:
       WaitToBeSeatedEntry(Node::NodeType t=Node::Module, u32 id=0);

       Node getNode() const { return mNode; }
       u32 getId() const { return mNode.id(); }
       QString tagName() const;
       bool isModule() const;

       double distance(const QPoint& pos, double defaultDistance) const;
       static int distance(const QPoint& source, const QPoint& pos);

   };

   class WaitToBeSeatedList : public QList<WaitToBeSeatedEntry*>
   {
       QMap<u32,WaitToBeSeatedEntry*> mGateMap;

       QList<WaitToBeSeatedEntry*> mIsolated;
       QList<WaitToBeSeatedEntry*> mStartpoint;
       QMap<WaitToBeSeatedEntry*,int> mWaiting;
       QSet<WaitToBeSeatedEntry*>  mPlaced;

       int mPlacementRound;
       double mSideLength;

   public:
       WaitToBeSeatedList();
       ~WaitToBeSeatedList();

       void setLinks();
       const WaitToBeSeatedEntry* nextPlacement(const QPoint& pos);
       const WaitToBeSeatedEntry* doPlacement(const QPoint& pos, WaitToBeSeatedEntry* wtse);

       void add(WaitToBeSeatedEntry* wtse);

       void dump() const;

       bool placementDone() const { return mPlaced.size() >= size(); }
   };

   bool compareWaitToBeSeated(const WaitToBeSeatedEntry* a, const WaitToBeSeatedEntry* b);
}
