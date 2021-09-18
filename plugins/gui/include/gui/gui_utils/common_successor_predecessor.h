#pragma once

#include "hal_core/defines.h"
#include <QHash>
#include <QList>

namespace hal {
    class Gate;

    class CommonSuccessorPredecessorGateQueue
    {
        int mIndex;
        Gate* mGate;
        QHash <Gate*, Gate*> mLastWaypoint;
        QList<Gate*> mQueue;
    public:
        CommonSuccessorPredecessorGateQueue(int i, Gate* g);
        QList<Gate*> getNext(bool forward);
        void setQueue(const QList<Gate*>& q) { mQueue = q; }
        int mask() const { return 1 << mIndex; }
        QList<Gate*> waypoints(Gate* g) const;
    };

    class CommonSuccessorPredecessor
    {
        bool mForward;
        int mMaskAll;
        QList<CommonSuccessorPredecessorGateQueue> mQueues;
        QHash<Gate*,int> mFoundBit;
        QSet<Gate*> mFoundSet;
    public:
        CommonSuccessorPredecessor(const QList<u32>& gateIds, bool forw, int maxRound);
        QSet<Gate*> result() const;
    };
}
