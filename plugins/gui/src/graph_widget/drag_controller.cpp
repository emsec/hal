#include "gui/graph_widget/drag_controller.h"
#include "gui/graph_widget/graph_widget.h"
#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_widget/graphics_scene.h"
#include <QApplication>
#include <QDebug>

namespace hal {
    DragController::DragController(GraphWidget* gw, QObject *parent)
        : QObject(parent), mGraphWidget(gw), mDragNodeBox(nullptr), mShadowScene(nullptr)
    {;}

    void DragController::clear()
    {
        mDragNodeBox = nullptr;
        mAdditionalBoxes.clear();
        mDropAllowed = false;
        mWantSwap = false;
        GraphicsScene* sc = mGraphWidget->getContext()->getLayouter()->scene();
        clearShadows(sc);
    }

    void DragController::clearShadows(GraphicsScene *sc)
    {
        if (sc && sc == mShadowScene)
        {
            // otherwise (if old scene deleted) items owned by scene already removed
            for (NodeDragShadow* nds : mShadows.values())
            {
                sc->removeItem(nds);
                delete nds;
            }
            mShadowScene->setDragController(nullptr);
        }
        mShadows.clear();
        mShadowScene = nullptr;
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
        if (!drgItem) return;
        // TODO: swap modifier -> deselect all but current

        QSet<Node> nodesToMove;
        QSet<u32> selGats = gSelectionRelay->selectedGates();
        QSet<u32> selMods = gSelectionRelay->selectedModules();
        bool isAlreadySelected = false;

        switch (drgItem->itemType())
        {
        case ItemType::Module:
            nodesToMove.insert(Node(drgItem->id(),Node::Module));
            if (selMods.contains(drgItem->id())) isAlreadySelected = true;
            break;
        case ItemType::Gate:
            nodesToMove.insert(Node(drgItem->id(),Node::Gate));
            if (selGats.contains(drgItem->id())) isAlreadySelected = true;
            break;
        default:
            break;
        }

        if (isAlreadySelected)
        {
            // multi-select requires that drag node was already selected before
            for (u32 mid : selMods)
            {
                nodesToMove.insert(Node(mid,Node::Module));
            }
            for (u32 gid : selGats)
            {
                nodesToMove.insert(Node(gid,Node::Gate));
            }
        }

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
    }

    void DragController::setSwapIntent(bool wantSwap)
    {
        if (wantSwap == mWantSwap) return;
        if (wantSwap)
        {
            GraphicsScene* sc = mGraphWidget->getContext()->getLayouter()->scene();
            if (sc && sc == mShadowScene)
            {
                for (NodeBox* nb : mAdditionalBoxes)
                {
                    NodeDragShadow* nds = mShadows.value(nb);
                    if (nds)
                    {
                        sc->removeItem(nds);
                        mShadows.remove(nb);
                        delete nds;
                    }
                }
            }
            mAdditionalBoxes.clear();
        }
        mWantSwap = wantSwap;
    }

    void DragController::addShadow(const NodeBox* nb)
    {
        NodeDragShadow* nds = new NodeDragShadow;
        nds->setVisualCue(dragCue());
        nds->start(nb->item()->pos(), nb->item()->boundingRect().size());
        mShadowScene = mGraphWidget->getContext()->getLayouter()->scene();
        if (mShadowScene)
        {
            mShadowScene->setDragController(this);
            mShadowScene->addItem(nds);
            mShadows.insert(nb,nds);
        }
        else
            delete nds;
    }

    void DragController::enterDrag(bool wantSwap)
    {
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
        mCurrentGridpos = gridPos;
        mDropAllowed = isDropAllowed();

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

    bool DragController::isDropAllowed() const
    {
        if (!mDragNodeBox) return false;
        if (mDragNodeBox->gridPosition() == mCurrentGridpos) return false;
        const NodeBoxes& boxes = mGraphWidget->getContext()->getLayouter()->boxes();

        if (mWantSwap)
        {
            return (boxes.boxForPoint(mCurrentGridpos) != nullptr);
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
            if (boxes.boxForPoint(p) != nullptr) return false;
        }
        return true;
    }

    GridPlacement *DragController::finalGridPlacement() const
    {
        GridPlacement* retval = mGraphWidget->getContext()->getLayouter()->gridPlacementFactory();
        retval->operator[](mDragNodeBox->getNode()) = mCurrentGridpos;
        if (mWantSwap)
        {
            Node targetNode = mGraphWidget->getContext()->getLayouter()->nodeAtPosition(mCurrentGridpos);
            if (!targetNode.isNull())
                retval->operator[](targetNode) = mDragNodeBox->gridPosition();
            return retval;
        }
        if (!mAdditionalBoxes.isEmpty())
        {
            QPoint delta = mCurrentGridpos - mDragNodeBox->gridPosition();
            for (const NodeBox* nb : mAdditionalBoxes)
            {
                retval->operator[](nb->getNode()) = nb->gridPosition() + delta;
            }
        }
        return retval;
    }
}
