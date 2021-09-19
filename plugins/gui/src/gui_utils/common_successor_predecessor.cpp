#include "gui/gui_utils/common_successor_predecessor.h"
#include "gui/gui_globals.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_utils.h"
#include "hal_core/utilities/log.h"

//#include "hal_core/netlist/

namespace hal {

    CommonSuccessorPredecessorGateQueue::CommonSuccessorPredecessorGateQueue(int i, Gate* g)
        : mIndex(i), mGate(g)
    {
        mLastWaypoint[g] = nullptr;
        mQueue.append(g);
    }

    QList<Gate*> CommonSuccessorPredecessorGateQueue::getNext(bool forward)
    {
        QList<Gate*> retval;

        for (Gate* origin : mQueue)
        {
            for (Gate*g : netlist_utils::get_next_gates(origin, forward, 1))
            {
                if (mLastWaypoint.contains(g)) continue;
                mLastWaypoint[g] = origin;
                retval.append(g);
            }
        }

        return retval;
    }

    QList<Gate*> CommonSuccessorPredecessorGateQueue::waypoints(Gate* g) const
    {
        QList<Gate*> retval;
        while (g)
        {
            retval.append(g);
            g = mLastWaypoint.value(g);
        }
        return retval;
    }

    CommonSuccessorPredecessor::CommonSuccessorPredecessor(const QList<u32>& gateIds, bool forw, int maxRound)
        : mForward(forw), mMaskAll(0)
    {
        if (gateIds.size() > 31)
        {
            log_warning("gui","Cannot find common {} for more than 31 gates", (forw?"successor":"predecessor"));
            return;
        }

        for (u32 id : gateIds)
        {
            Gate* g = gNetlist->get_gate_by_id(id);
            Q_ASSERT(g);
            CommonSuccessorPredecessorGateQueue cspgq(mQueues.size(),g);
            int msk = cspgq.mask();
            mMaskAll |= msk;
            mFoundBit[g] |= msk;
            mQueues.append(cspgq);
        }

        for (int iRound = 0; iRound<maxRound; iRound++)
        {
            for (auto it = mQueues.begin(); it!=mQueues.end(); ++it)
            {
                QList nextGates = it->getNext(mForward);
                for (Gate* g: nextGates)
                    if ((mFoundBit[g] |= it->mask()) == mMaskAll)
                        mFoundSet.insert(g);

                it->setQueue(nextGates);
            }
            if (!mFoundSet.isEmpty()) break;
        }

        if (mFoundSet.isEmpty())
            log_info("gui", "No common {} found.", (forw?"successor":"predecessor"));
    }

    QSet<Gate *> CommonSuccessorPredecessor::result() const
    {
        QSet<Gate*> retval;
        for (Gate* g : mFoundSet)
        {
            for (const CommonSuccessorPredecessorGateQueue& cspgq : mQueues)
                for (Gate* wayp : cspgq.waypoints(g))
                    retval.insert(wayp);
        }
        return retval;
    }
}

