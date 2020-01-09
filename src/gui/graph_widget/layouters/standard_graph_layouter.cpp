#include "gui/graph_widget/layouters/standard_graph_layouter.h"

#include "gui/implementations/qpoint_extension.h"

standard_graph_layouter::standard_graph_layouter(const graph_context* const context) : graph_layouter(context)
{
}

const QString standard_graph_layouter::name() const
{
    return "Standard Layouter";
}

const QString standard_graph_layouter::description() const
{
    return "<p>PLACEHOLDER</p>";
}

void standard_graph_layouter::add(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets, hal::placement_hint placement)
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

void standard_graph_layouter::add_compact(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& nets)
{  
    Q_UNUSED(nets)

    int x = 0;
    int y = 0;

    int x_pos = 0;
    int y_pos = 0;

    QSet<u32>::const_iterator i = modules.constBegin();

    module_id_loop:

    if (i != modules.constEnd())
    {
        u32 id = *i;
        ++i;

        module_position_loop:

        while (y_pos != y)
        {
            QPoint p(x, y_pos);
            ++y_pos;

            if (!position_to_node_map().contains(p))
            {
                hal::node n{hal::node_type::module, id};
                set_node_position(n, p);
                goto module_id_loop;
            }
        }

        while (x_pos != x)
        {
            QPoint p(x_pos, y);
            ++x_pos;

            if (!position_to_node_map().contains(p))
            {
                hal::node n{hal::node_type::module, id};
                set_node_position(n, p);
                goto module_id_loop;
            }
        }

        QPoint p(x, y);

        ++x;
        ++y;

        x_pos = 0;
        y_pos = 0;

        if (!position_to_node_map().contains(p))
        {
            hal::node n{hal::node_type::module, id};
            set_node_position(n, p);
            goto module_id_loop;
        }
        else
            goto module_position_loop;
    }

    i = gates.constBegin();

    gate_id_loop:

    if (i != gates.constEnd())
    {
        u32 id = *i;
        ++i;

        gate_position_loop:

        while (y_pos != y)
        {
            QPoint p(x, y_pos);
            ++y_pos;

            if (!position_to_node_map().contains(p))
            {
                hal::node n{hal::node_type::gate, id};
                set_node_position(n, p);
                goto gate_id_loop;
            }
        }

        while (x_pos != x)
        {
            QPoint p(x_pos, y);
            ++x_pos;

            if (!position_to_node_map().contains(p))
            {
                hal::node n{hal::node_type::gate, id};
                set_node_position(n, p);
                goto gate_id_loop;
            }
        }

        QPoint p(x, y);

        ++x;
        ++y;

        x_pos = 0;
        y_pos = 0;

        if (!position_to_node_map().contains(p))
        {
            hal::node n{hal::node_type::gate, id};
            set_node_position(n, p);
            goto gate_id_loop;
        }
        else
            goto gate_position_loop;
    }
}

void standard_graph_layouter::add_vertical(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& nets, bool left, const hal::node& preferred_origin) {
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


void standard_graph_layouter::remove(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets)
{
    Q_UNUSED(nets)

    for (u32 id : modules)
        remove_node_from_maps({hal::node_type::module, id});

    for (u32 id : gates)
        remove_node_from_maps({hal::node_type::gate, id});
}
