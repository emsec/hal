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

    void StandardGraphLayouter::add(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets, hal::placement_hint placement)
    {
        switch(placement.mode)
        {
        case hal::placement_mode::standard: {
            add_compact(modules, gates, nets);
            break;
        }
        case hal::placement_mode::prefer_left: {
            add_vertical(modules, gates, nets, true, placement.preferred_origin);
            break;
        }
        case hal::placement_mode::prefer_right: {
            add_vertical(modules, gates, nets, false, placement.preferred_origin);
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
            wtbsl.add(new WaitToBeSeatedEntry(node_type::module, *it));

        for (QSet<u32>::const_iterator it = gates.constBegin();
             it != gates.constEnd(); ++it)
            wtbsl.add(new WaitToBeSeatedEntry(node_type::gate, *it));

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

        QList<node> nodeList;

        for (QSet<u32>::const_iterator it = modules.constBegin();
             it != modules.constEnd(); ++it)
            nodeList.append(node{node_type::module,*it});

        for (QSet<u32>::const_iterator it = gates.constBegin();
             it != gates.constEnd(); ++it)
            nodeList.append(node{node_type::gate,*it});

        PositionGenerator pg;

        for (const node& n : nodeList)
        {
            QPoint p(pg.position());
            while (position_to_node_map().contains(p))
                p = pg.next();
            set_node_position(n, p);
        }
    }

    void StandardGraphLayouter::add_vertical(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& nets, bool left, const hal::node& preferred_origin) {
        Q_UNUSED(nets);

        int x;
        int y;

        if (preferred_origin.id != 0 && node_to_position_map().contains(preferred_origin))
        {
            // place all new nodes right respectively left of the origin node
            QPoint originPoint = node_to_position_map().value(preferred_origin);
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


        for (const u32 m : modules)
        {
            hal::node n{hal::node_type::module, m};
            QPoint p;
            do
            {
                // skip over positions that are already taken
                p = QPoint(x,y++);
            }
            while(position_to_node_map().contains(p));
            set_node_position(n, p);
        }
        for (const u32 g : gates)
        {
            hal::node n{hal::node_type::gate, g};
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
            remove_node_from_maps({hal::node_type::module, id});

        for (u32 id : gates)
            remove_node_from_maps({hal::node_type::gate, id});
    }
}
