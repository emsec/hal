#include "gui/graph_widget/layouters/standard_graph_layouter.h"

#include "gui/implementations/qpoint_extension.h"
#include "gui/graph_widget/layouters/position_generator.h"
#include "gui/graph_widget/layouters/wait_to_be_seated.h"
#include "gui/gui_globals.h"

namespace hal
{
    StandardGraphLayouter::StandardGraphLayouter(const GraphContext* const context) : GraphLayouter(context)
    {
    }

    QString StandardGraphLayouter::name() const
    {
        return "Standard Layouter";
    }

    QString StandardGraphLayouter::description() const
    {
        return "<p>PLACEHOLDER</p>";
    }

    void StandardGraphLayouter::add(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets, PlacementHint placement)
    {
        switch(placement.mode())
        {
        case PlacementHint::Standard: {
            add_compact(modules, gates, nets);
            break;
        }
        case PlacementHint::PreferLeft: {
            add_vertical(modules, gates, nets, true, placement.preferredOrigin());
            break;
        }
        case PlacementHint::PreferRight: {
            add_vertical(modules, gates, nets, false, placement.preferredOrigin());
            break;
        }
        }
    }

    void StandardGraphLayouter::add_waitToBeSeated(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& nets)
    {
        Q_UNUSED(nets)

        WaitToBeSeatedList wtbsl;
        for (QSet<u32>::const_iterator it = modules.constBegin();
             it != modules.constEnd(); ++it)
            wtbsl.add(new WaitToBeSeatedEntry(Node::Module, *it));

        for (QSet<u32>::const_iterator it = gates.constBegin();
             it != gates.constEnd(); ++it)
            wtbsl.add(new WaitToBeSeatedEntry(Node::Gate, *it));

        wtbsl.setLinks();

        PositionGenerator pg;

        while(!wtbsl.placementDone())
        {
            QPoint p(pg.position());
            while (position_to_node_map().contains(p))
                p = pg.next();
            const WaitToBeSeatedEntry* wtbse = wtbsl.nextPlacement(p);
            if (! wtbse) return;
            set_node_position(wtbse->getNode(), p);
        }
    }

    void StandardGraphLayouter::add_compact(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& nets)
    {
        Q_UNUSED(nets)

        if (g_settings_manager->get("graph_view/layout_boxes").toBool())
        {
            add_waitToBeSeated(modules, gates, nets);
            return;
        }

        QList<Node> nodeList;

        for (QSet<u32>::const_iterator it = modules.constBegin();
             it != modules.constEnd(); ++it)
            nodeList.append(Node(*it,Node::Module));

        for (QSet<u32>::const_iterator it = gates.constBegin();
             it != gates.constEnd(); ++it)
            nodeList.append(Node(*it,Node::Gate));

        PositionGenerator pg;

        for (const Node& n : nodeList)
        {
            QPoint p(pg.position());
            while (position_to_node_map().contains(p))
                p = pg.next();
            set_node_position(n, p);
        }
    }

    void StandardGraphLayouter::add_vertical(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& nets, bool left, const Node &preferredOrigin) {
        Q_UNUSED(nets);

        int x;
        int y;

        if (!preferredOrigin.isNull() && node_to_position_map().contains(preferredOrigin))
        {
            // place all new nodes right respectively left of the origin node
            QPoint originPoint = node_to_position_map().value(preferredOrigin);
            x = originPoint.x() + (left ? -1 : 1);
            // vertically center the column of new nodes relative to the origin node
            int totalNodes = modules.size() + gates.size();
            y = originPoint.y() - (totalNodes-1)/2;
        }
        else
        {
            // create a new column right- respectively leftmost of all current nodes
            x = left ? min_x_index() - 1 : min_x_index() + x_values().size();
            // center column of new ndoes vertically relative to the entire grid
            y = min_y_index() + (y_values().size()-1) / 2;
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
            while(position_to_node_map().contains(p));
            set_node_position(n, p);
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
            while(position_to_node_map().contains(p));
            set_node_position(n, p);
        }
    }


    void StandardGraphLayouter::remove(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets)
    {
        Q_UNUSED(nets)

        for (u32 id : modules)
            remove_node_from_maps(Node(id,Node::Module));

        for (u32 id : gates)
            remove_node_from_maps(Node(id,Node::Gate));
    }
}
