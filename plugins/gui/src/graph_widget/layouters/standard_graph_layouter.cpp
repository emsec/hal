#include "gui/graph_widget/layouters/standard_graph_layouter.h"

#include "gui/implementations/qpoint_extension.h"
#include "gui/graph_widget/layouters/position_generator.h"
#include "gui/graph_widget/layouters/wait_to_be_seated.h"
#include "gui/gui_globals.h"
#include "gui/graph_widget/layouters/coordinate_from_data.h"
#include "gui/settings/settings_items/settings_item_checkbox.h"

#include <QDebug>

namespace hal
{
    StandardGraphLayouter::StandardGraphLayouter(GraphContext* context) : GraphLayouter(context), mParseLayout(true), mLayoutBoxes(true)
    {
    }

    QString StandardGraphLayouter::name() const
    {
        return "Standard Layouter";
    }

    QString StandardGraphLayouter::mDescription() const
    {
        return "<p>PLACEHOLDER</p>";
    }

    void StandardGraphLayouter::add(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets, PlacementHint placement)
    {
        switch(placement.mode())
        {
        case PlacementHint::Standard:
            addCompact(modules, gates, nets);
            break;
        case PlacementHint::PreferLeft:
            addVertical(modules, gates, nets, true, placement.preferredOrigin());
            break;
        case PlacementHint::PreferRight:
            addVertical(modules, gates, nets, false, placement.preferredOrigin());
            break;
        case PlacementHint::GridPosition:
            addGridPosition(modules, gates, nets, placement.gridPosition());
            break;
        }
//        qDebug() << "StandardGraphLayouter::add (box mod gat)" << boxes().size() << modules.size() << gates.size();
    }

    void StandardGraphLayouter::addWaitToBeSeated(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& nets)
    {
        Q_UNUSED(nets)

        CoordinateFromDataMap cfdMap(modules,gates);
        if (mPositionToNodeMap.isEmpty() &&
        	mParseLayout &&
                !gFileStatusManager->modifiedFilesExisting() &&
                cfdMap.good())
        {
            cfdMap.simplify();
            for (auto it = cfdMap.begin(); it != cfdMap.end(); ++it)
                setNodePosition(it.key(),it.value());
            if (cfdMap.isPlacementComplete()) return;
        }
        else
            cfdMap.clear();

        WaitToBeSeatedList wtbsl;
        for (QSet<u32>::const_iterator it = modules.constBegin();
             it != modules.constEnd(); ++it)
        {
            if (cfdMap.good() && cfdMap.placedModules().contains(*it))
                continue; // already placed
            wtbsl.add(new WaitToBeSeatedEntry(Node::Module, *it));
        }

        for (QSet<u32>::const_iterator it = gates.constBegin();
             it != gates.constEnd(); ++it)
        {
            if (cfdMap.good() && cfdMap.placedGates().contains(*it))
                continue; // already placed
            wtbsl.add(new WaitToBeSeatedEntry(Node::Gate, *it));
        }

        wtbsl.setLinks();

        PositionGenerator pg;

        while(!wtbsl.placementDone())
        {
            QPoint p(pg.position());
            while (positionToNodeMap().contains(p))
                p = pg.next();
            const WaitToBeSeatedEntry* wtbse = wtbsl.nextPlacement(p);
            if (! wtbse) return;
            Node pNode = wtbse->getNode();
            setNodePosition(pNode, p);
        }
    }

    void StandardGraphLayouter::addCompact(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& nets)
    {
        Q_UNUSED(nets)

        if (mLayoutBoxes)
        {
            addWaitToBeSeated(modules, gates, nets);
            return;
        }

        QList<Node> nodeList;
        for (u32 id : modules)
            nodeList.append(Node(id,Node::Module));
        for (u32 id : gates)
            nodeList.append(Node(id,Node::Gate));

        PositionGenerator pg;

        for (const Node& n : nodeList)
        {
            QPoint p(pg.position());
            while (positionToNodeMap().contains(p))
                p = pg.next();
            setNodePosition(n, p);
        }
    }

    void StandardGraphLayouter::addGridPosition(const QSet<u32>& modules, const QSet<u32>& gates,
                                                const QSet<u32>& nets, const QHash<Node,QPoint>& pos)
    {
        QList<Node> nodeList;
        QList<Node> nodeNotPlaced;

        for (u32 id : modules)
            nodeList.append(Node(id,Node::Module));
        for (u32 id : gates)
            nodeList.append(Node(id,Node::Gate));

        for (Node nd : nodeList)
        {
            auto it = pos.find(nd);
            if (it == pos.constEnd())
                nodeNotPlaced.append(nd);
            else
            {
               QPoint p = it.value();
               if (positionToNodeMap().contains(p))
                    nodeNotPlaced.append(nd);
               else
                   setNodePosition(nd, p);
            }
        }

        if (!nodeNotPlaced.isEmpty())
        {
            QSet<u32> modsNotPlaced;
            QSet<u32> gatsNotPlaced;
            for (Node nd : nodeNotPlaced)
            {
                switch (nd.type()) {
                case Node::Module:
                    modsNotPlaced.insert(nd.id());
                    break;
                case Node::Gate:
                    gatsNotPlaced.insert(nd.id());
                    break;
                default:
                    break;
                }
            }
            addCompact(modsNotPlaced,gatsNotPlaced,nets);
        }
    }

    void StandardGraphLayouter::addVertical(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& nets, bool left, const Node &preferredOrigin) {
        Q_UNUSED(nets);

        int x;
        int y;
        NetLayoutPoint originPos = positonForNode(preferredOrigin);

        if (originPos.isUndefined())
        {
            // create a new column right- respectively leftmost of all current nodes
            x = left ? minXIndex() - 1 : minXIndex() + xValues().size();
            // center column of new ndoes vertically relative to the entire grid
            y = minYIndex() + (yValues().size()-1) / 2;
        }
        else
        {
            // place all new nodes right respectively left of the origin node
            x = originPos.x() + (left ? -1 : 1);
            // vertically center the column of new nodes relative to the origin node
            int totalNodes = modules.size() + gates.size();
            y = originPos.y() - (totalNodes-1)/2;
        }

        for (const u32 mid : modules)
        {
            Node n(mid, Node::Module);
            QPoint p;
            do
            {
                // skip over positions that are already taken
                p = QPoint(x,y++);
            }
            while(positionToNodeMap().contains(p));
            setNodePosition(n, p);
        }
        for (const u32 gid : gates)
        {
            Node n(gid, Node::Gate);
            QPoint p;
            do
            {
                // skip over positions that are already taken
                p = QPoint(x,y++);
            }
            while(positionToNodeMap().contains(p));
            setNodePosition(n, p);
        }
    }

    void StandardGraphLayouter::remove(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets)
    {
        Q_UNUSED(nets)

        for (u32 id : modules)
            removeNodeFromMaps(Node(id,Node::Module));

        for (u32 id : gates)
            removeNodeFromMaps(Node(id,Node::Gate));
    }

    bool StandardGraphLayouter::parseLayoutEnabled()
    {
        return mParseLayout;
    }

    void StandardGraphLayouter::setParseLayoutEnabled(bool enabled)
    {
        mParseLayout = enabled;
    }

    bool StandardGraphLayouter::layoutBoxesEnabled()
    {
        return mLayoutBoxes;
    }

    void StandardGraphLayouter::setLayoutBoxesEnabled(bool enabled)
    {
        mLayoutBoxes = enabled;
    }
}
