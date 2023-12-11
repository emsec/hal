#include "gui/graph_widget/drag_controller.h"
#include "gui/graph_widget/graph_widget.h"
#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_widget/graphics_scene.h"
#include <QApplication>
#include <QDebug>

namespace hal {
    DragController::DragController(GraphWidget* gw, QObject *parent)
        : QObject(parent), mGraphWidget(gw), mDragNodeBox(nullptr)
    {;}

    void DragController::clear()
    {
        mDragNodeBox = nullptr;
        mAdditionalBoxes.clear();
        mDropAllowed = false;
        mWantSwap = false;
        GraphicsScene* sc = mGraphWidget->getContext()->getLayouter()->scene();
        for (NodeDragShadow* nds : mShadows.values())
        {
            if (sc) sc->removeItem(nds);
            delete nds;
        }
        mShadows.clear();
    }

    NodeDragShadow::DragCue DragController::dragCue() const
    {
        if (!mDropAllowed)
            return NodeDragShadow::DragCue::Rejected;
        if (mWantSwap)
            return NodeDragShadow::DragCue::Swappable;
        return NodeDragShadow::DragCue::Movable;
    }

    void DragController::set(GraphicsNode *drgItem, const QPoint &eventPos)
    {
        clear();
        // TODO: swap modifier -> deselect all but current

        QList<Node> nodesToMove;
        for (u32 mid : gSelectionRelay->selectedModules())
            nodesToMove.append(Node(mid,Node::Module));
        for (u32 gid : gSelectionRelay->selectedGates())
            nodesToMove.append(Node(gid,Node::Gate));

        auto context            = mGraphWidget->getContext();
        const GraphLayouter* layouter = context->getLayouter();
        if (!layouter->done()) return;

        mMousedownPosition = eventPos;

        for (const Node& nd : nodesToMove)
        {
            NodeBox* nb = layouter->boxes().boxForNode(nd);
            if (!nb) continue;
            if (nb->item() == drgItem)
                mDragNodeBox = nb;
            else
                mAdditionalBoxes.insert(nb);
        }

        if (!mAdditionalBoxes.isEmpty()) mWantSwap = false;
        qDebug() << "drag set  " << (mDragNodeBox ? mDragNodeBox->id() : -1) << mAdditionalBoxes.size();
    }

    void DragController::setSwapIntent(bool wantSwap)
    {
        if (wantSwap == mWantSwap) return;
        if (wantSwap) mAdditionalBoxes.clear();
        mWantSwap = wantSwap;
    }

    void DragController::addShadow(const NodeBox* nb)
    {
        NodeDragShadow* nds = new NodeDragShadow;
        nds->setVisualCue(dragCue());
        nds->start(nb->item()->pos(), nb->item()->boundingRect().size());
        GraphicsScene* sc = mGraphWidget->getContext()->getLayouter()->scene();
        if (sc) sc->addItem(nds);
        mShadows.insert(nb,nds);
    }

    void DragController::enterDrag(bool wantSwap)
    {
        qDebug() << "drag enter" << (mDragNodeBox ? mDragNodeBox->id() : -1) << mAdditionalBoxes.size();
        if (!mDragNodeBox) return;
        setSwapIntent(wantSwap);
        mCurrentGridpos = mDragNodeBox->gridPosition();
        mDropAllowed = false;
        addShadow(mDragNodeBox);
        for (NodeBox* nb : mAdditionalBoxes)
            addShadow(nb);
    }

    void DragController::move(const QPoint& eventPos, bool wantSwap, const QPoint& gridPos)
    {
        Q_UNUSED(eventPos);
        if (!mDragNodeBox || (wantSwap == mWantSwap && gridPos == mCurrentGridpos)) return;

        setSwapIntent(wantSwap);
        qDebug() << "drag set" << (mDragNodeBox ? mDragNodeBox->id() : -1) << mAdditionalBoxes.size();
        mCurrentGridpos = gridPos;
        mDropAllowed = (isDropAllowed()==0);

        QPoint delta = mCurrentGridpos - mDragNodeBox->gridPosition();
        for (auto it = mShadows.constBegin(); it != mShadows.constEnd(); ++it)
        {
            it.value()->setVisualCue(dragCue());
            QPoint p = it.key()->gridPosition() + delta;
            float x = mGraphWidget->getContext()->getLayouter()->gridXposition(p.x());
            float y = mGraphWidget->getContext()->getLayouter()->gridYposition(p.y());
            it.value()->setPos(QPointF(x,y));
        }
    }

    bool DragController::hasDragged(const QPoint &eventPos)
    {
        if (!mDragNodeBox) return false;
        return (eventPos - mMousedownPosition).manhattanLength() >= QApplication::startDragDistance();
    }

    int DragController::isDropAllowed() const
    {
        if (!mDragNodeBox) return -1;
        if (mDragNodeBox->gridPosition() == mCurrentGridpos) return -2;
        const NodeBoxes& boxes = mGraphWidget->getContext()->getLayouter()->boxes();

        if (mWantSwap)
        {
            if (boxes.boxForPoint(mCurrentGridpos) != nullptr)
                return 0;
            return -3;
        }

        QList<QPoint> pointsToCheck;
        QSet<QPoint> freedPositions;
        pointsToCheck.append(mCurrentGridpos);
        freedPositions.insert(mDragNodeBox->gridPosition());

        QPoint delta = mCurrentGridpos - mDragNodeBox->gridPosition();
        for (const NodeBox* nb : mAdditionalBoxes)
        {
            pointsToCheck.append(nb->gridPosition() + delta);
            freedPositions.insert(nb->gridPosition());
        }

        for (const QPoint& p : pointsToCheck)
        {
            if (freedPositions.contains(p)) continue;
            if (boxes.boxForPoint(p) != nullptr) return -4;
        }
        return 0;
    }
    /*

            auto context            = mGraphWidget->getContext();
            const GraphLayouter* layouter = context->getLayouter();
            assert(layouter->done());    // ensure grid stable

            QMap<QPoint, Node>::const_iterator node_iter = layouter->positionToNodeMap().find(mDragCurrentGridpos);

            NodeDragShadow::DragCue cue = NodeDragShadow::DragCue::Rejected;
            // disallow dropping an item on itself
            if (mDragCurrentGridpos != mDragStartGridpos)
            {
                if (swapModifier)
                {
                    if (node_iter != layouter->positionToNodeMap().end())
                    {
                        // allow move only on empty cells
                        cue = NodeDragShadow::DragCue::Swappable;
                    }
                }
                else
                {
                    if (mDragAdditionalGridpos.isEmpty())
                    {
                        // move single node
                        if (node_iter == layouter->positionToNodeMap().end())
                        {
                            // allow move only on empty cells
                            cue = NodeDragShadow::DragCue::Movable;
                        }
                    }
                    else
                    {
                        // move multi nodes
                        if (node_iter == layouter->positionToNodeMap().end() || mDragAdditionalGridpos.contains(mDragCurrentGridpos) )
                        {
                            cue = NodeDragShadow::DragCue::Movable;
                            for (QPoint p : mDragAdditionalGridpos)
                            {
                                p += mDragCurrentGridpos - mDragStartGridpos;
                                if (p != mDragStartGridpos && !mDragAdditionalGridpos.contains(p))
                                {
                                    if (layouter->positionToNodeMap().find(p) != layouter->positionToNodeMap().end())
                                    {
                                        cue = NodeDragShadow::DragCue::Rejected;
                                        break;
                                    }
                                }
                            }
                        }

                    }
                }
            }
            mDropAllowed = (cue != NodeDragShadow::DragCue::Rejected);

            static_cast<GraphicsScene*>(scene())->moveDragShadow(snap.second, cue);
            */


    QHash<Node,QPoint> DragController::finalNodePositions() const
    {
        QHash<Node,QPoint> retval = mGraphWidget->getContext()->getLayouter()->nodeToPositionHash();
        retval[mDragNodeBox->getNode()] = mCurrentGridpos;
        if (mWantSwap)
        {
            Node targetNode = mGraphWidget->getContext()->getLayouter()->nodeAtPosition(mCurrentGridpos);
            if (!targetNode.isNull())
                retval[targetNode] = mDragNodeBox->gridPosition();
            return retval;
        }
        if (!mAdditionalBoxes.isEmpty())
        {
            QPoint delta = mCurrentGridpos - mDragNodeBox->gridPosition();
            for (const NodeBox* nb : mAdditionalBoxes)
            {
                retval[nb->getNode()] = nb->gridPosition() + delta;
            }
        }
        return retval;
    }
}
