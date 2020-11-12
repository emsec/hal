#include "gui/graph_widget/layouters/wait_to_be_seated.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/endpoint.h"
#include "gui/gui_globals.h"
#include <QTextStream>
#include <QDebug>

namespace hal
{
   WaitToBeSeatedEntry::WaitToBeSeatedEntry(Node::NodeType t, u32 id)
   {
       mNode = Node(id,t);
   }

   void WaitToBeSeatedEntry::setPredecessorIds(const QMap<u32, WaitToBeSeatedEntry *>& gateMap)
   {
       if (getId() <= 0) return;

       std::vector<Net*> inputNets;
       if (isModule())
       {
           const Module* m = gNetlist->get_module_by_id(getId());
           if (m)
               inputNets  = m->get_input_nets();
       }
       else
       {
           const Gate* g = gNetlist->get_gate_by_id(getId());
           if (g)
               inputNets  = g->get_fan_in_nets();
       }

       for (Net* n: inputNets)
       {
           for (const Endpoint* ep : n->get_sources())
           {
               Gate* inpGate = ep->get_gate();
               if (inpGate->is_gnd_gate() || inpGate->is_vcc_gate()) break;
               u32 inpGateId = inpGate->get_id();
               WaitToBeSeatedEntry* inpEntry = gateMap.value(inpGateId);
               if (inpEntry)
                   mPredecessorSet.insert(inpEntry);
           }
       }
   }

   QString WaitToBeSeatedEntry::tagName() const
   {
       return QString("%1%2").arg(isModule()?'M':'G').arg(getId());
   }

   bool WaitToBeSeatedEntry::isModule() const
   {
       return mNode.isModule() && mNode.id() > 0;
   }

   //---------------------------------------------------
   bool compareWaitToBeSeated(const WaitToBeSeatedEntry* a, const WaitToBeSeatedEntry* b)
   {
       return a->getId() < b->getId();
   }

   double WaitToBeSeatedEntry::distance(const QPoint &pos, double defaultDistance) const
   {
       double retval = 0;
       int n = mPredecessorSet.size();
       Q_ASSERT (n>0);

       // distance to predecessors already placed
       for(const QPoint& source : mPredecessorPositions)
           retval += distance(source, pos);

       // predecessors not placed yet
       retval += (n - mPredecessorPositions.size()) * defaultDistance * 4;

       // average
       retval /= n;

       // to break a tie : add module/gate id
       retval += getId() / 1000000.;
       return retval;
   }

   int WaitToBeSeatedEntry::distance(const QPoint& source, const QPoint& pos)
   {
       int dx = abs(source.x()+1-pos.x());
       int dy = abs(source.y()-pos.y()) - 1;
       if (dy < 0)
         {
           // immediate neighbour
           if (!dx) return 1;
           dy = 0;
         }
       return 5 + 4*(dx+dy);
   }

   //---------------------------------------------------
   WaitToBeSeatedList::WaitToBeSeatedList()
       : mPlacementRound(0), mSideLength(0)
   {;}

   WaitToBeSeatedList::~WaitToBeSeatedList()
   {
       for (WaitToBeSeatedEntry* wtse : *this)
           delete wtse;
   }

   void WaitToBeSeatedList::setLinks()
   {
       for (WaitToBeSeatedEntry* wtse : *this)
           wtse->setPredecessorIds(mGateMap);

       for (WaitToBeSeatedEntry* wtse : *this)
           for (WaitToBeSeatedEntry* wtsePred : wtse->mPredecessorSet)
               if (wtsePred) wtsePred->mSuccessorSet.insert(wtse);

       for (WaitToBeSeatedEntry* wtse : *this)
           if (wtse->mPredecessorSet.isEmpty())
           {
               if (wtse->mSuccessorSet.isEmpty())
                   mIsolated.append(wtse);
               else
                   mStartpoint.append(wtse);
           }

       if (mIsolated.size() > 1)
           std::sort(mIsolated.begin(), mIsolated.end(), compareWaitToBeSeated);

       if (mStartpoint.size() > 1)
           std::sort(mStartpoint.begin(), mStartpoint.end(), compareWaitToBeSeated);

       mSideLength = sqrt(size());
   }

   void WaitToBeSeatedList::add(WaitToBeSeatedEntry* wtse)
   {
       Q_ASSERT(wtse);
       append(wtse);
       if (wtse->isModule())
       {
           const Module* m = gNetlist->get_module_by_id(wtse->getId());
           if (m)
           {
               for (const Gate* g : m->get_gates(nullptr,true))
                   if (g) mGateMap.insert(g->get_id(),wtse);
           }
       }
       else
           mGateMap.insert(wtse->getId(),wtse);
   }

   const WaitToBeSeatedEntry* WaitToBeSeatedList::nextPlacement(const QPoint& pos)
   {
       bool isEdge = pos.x() == 0 || pos.y() == 0;
       if (!mIsolated.isEmpty()
           && (isEdge || mWaiting.isEmpty() ) )
       {
            return doPlacement(pos,mIsolated.takeFirst());
       }
       if (!mStartpoint.isEmpty()
           && (isEdge || mWaiting.isEmpty() ) )
       {
            return doPlacement(pos,mStartpoint.takeFirst());
       }

       if (mWaiting.isEmpty() && !placementDone())
       {
           for (WaitToBeSeatedEntry* wtse : *this)
               if (!mPlaced.contains(wtse))
               {
                   mWaiting.insert(wtse,mPlacementRound);
                   break;
               }
       }

       if (!mWaiting.isEmpty())
       {
           double minDistance = 0;
           QMap<WaitToBeSeatedEntry*,int>::iterator jt = mWaiting.end();
            for (QMap<WaitToBeSeatedEntry*,int>::iterator it =
                 mWaiting.begin(); it != mWaiting.end(); ++it)
            {
                double distance = it.key()->distance(pos,mSideLength)
                        - 0.5 * (mPlacementRound - it.value());
                if (jt == mWaiting.end() || distance < minDistance)
                {
                    minDistance = distance;
                    jt = it;
                }
            }
            Q_ASSERT (jt != mWaiting.end());
            WaitToBeSeatedEntry* closest = jt.key();
            mWaiting.erase(jt);
            return doPlacement(pos, closest);
       }

       return nullptr;
   }

   const WaitToBeSeatedEntry* WaitToBeSeatedList::doPlacement(const QPoint &pos, WaitToBeSeatedEntry *wtse)
   {
       assert (!mPlaced.contains(wtse));
       ++mPlacementRound;
       mPlaced.insert(wtse);
       for (WaitToBeSeatedEntry* wtseSucc : wtse->mSuccessorSet)
       {
           wtseSucc->mPredecessorPositions.append(pos);
           if (!mWaiting.contains(wtseSucc) && !mPlaced.contains(wtseSucc))
               mWaiting.insert(wtseSucc,mPlacementRound);
       }
       return wtse;
   }

   void WaitToBeSeatedList::dump() const
   {
       QTextStream xout(stdout, QIODevice::WriteOnly);
       xout << "WaitToBeSeatedList\n";
       for (WaitToBeSeatedEntry* wtse : *this)
       {
           xout.setFieldWidth(4);
           xout << wtse->getId();
           xout << ( wtse->isModule() ? "MOD" : "GAT");
           xout << "<<<";
           for (WaitToBeSeatedEntry* wtsePred : wtse->mPredecessorSet)
           {
                xout << wtsePred->tagName();
           }
           xout << ">>>";
           for (WaitToBeSeatedEntry* wtseSucc : wtse->mSuccessorSet)
           {
                xout << wtseSucc->tagName();
           }
           xout.setFieldWidth(0);
           xout << "\n";
       }
       xout << "------------------------\n";
   }

}
