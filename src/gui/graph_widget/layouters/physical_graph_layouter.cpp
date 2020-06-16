#include "gui/graph_widget/layouters/physical_graph_layouter.h"

#include "gui_globals.h"

#include <limits>
#include <set>

physical_graph_layouter::physical_graph_layouter(const graph_context* const context) : graph_layouter(context),
    m_min_x_distance(std::numeric_limits<float>::max()),
    m_min_y_distance(std::numeric_limits<float>::max())
{
}

QString physical_graph_layouter::name() const
{
    return "Physical Layouter";
}

QString physical_graph_layouter::description() const
{
    return "<p>PLACEHOLDER</p>";
}

void physical_graph_layouter::add(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets, hal::placement_hint placement)
{
    Q_UNUSED(modules)
    Q_UNUSED(gates)
    Q_UNUSED(nets)
    // TODO is it OK to ignore the placement hint in this layouter?
    Q_UNUSED(placement)

    // DOES THE CORE GUARANTEE UNIQUE LOCATION DATA ???
    // COLLAPSE / ALIGN

    std::set<float> x_coordinates;
    std::set<float> y_coordinates;

    for (u32 id : gates)
    {
        std::shared_ptr<Gate> g = g_netlist->get_gate_by_id(id);
        assert(g);

        if (g->has_location())
        {
            x_coordinates.insert(g->get_location_x());
            y_coordinates.insert(g->get_location_y());
        }
    }

    for (float x1 : x_coordinates)
        for (float x2 : x_coordinates)
            if (x1 != x2)
            {
                float x_distance = std::abs(x1 - x2);

                if (x_distance < m_min_x_distance)
                    m_min_x_distance = x_distance;
            }

    for (float y1 : y_coordinates)
        for (float y2 : y_coordinates)
            if (y1 != y2)
            {
                float y_distance = std::abs(y1 - y2);

                if (y_distance < m_min_y_distance)
                    m_min_y_distance = y_distance;
            }
}

void physical_graph_layouter::remove(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets)
{
    Q_UNUSED(modules)
    Q_UNUSED(gates)
    Q_UNUSED(nets)

    for (u32 id : modules)
        remove_node_from_maps({hal::node_type::module, id});

    for (u32 id : gates)
        remove_node_from_maps({hal::node_type::gate, id});
}
