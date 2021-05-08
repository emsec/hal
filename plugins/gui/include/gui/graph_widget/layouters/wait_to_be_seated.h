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

#include <QList>
#include <QMap>
#include <QSet>
#include <QPoint>
#include "gui/gui_def.h"

namespace hal
{
   class WaitToBeSeatedList;
   class WaitToBeSeatedWaiting;

   /**
    * @ingroup graph-layouter
    */
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

   /**
    * @ingroup graph-layouter
    */
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
