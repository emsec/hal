#include "graph_widget/layouters/standard_graph_layouter.h"

static bool operator< (const QPoint &p1, const QPoint &p2)
{
    if (p1.x() < p2.x())
        return true;
    else if (p2.x() < p1.x())
        return false;
    else if (p1.y() < p2.y())
        return true;
    else
        return false;
}

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

void standard_graph_layouter::add(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets)
{
    Q_UNUSED(modules)
    Q_UNUSED(gates)
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

void standard_graph_layouter::remove(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets)
{
    Q_UNUSED(modules)
    Q_UNUSED(gates)
    Q_UNUSED(nets)

    for (u32 id : modules)
        remove_node_from_maps({hal::node_type::module, id});

    for (u32 id : gates)
        remove_node_from_maps({hal::node_type::gate, id});
}
