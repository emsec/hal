#include "gui/graph_widget/layouters/graph_layouter.h"

#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_widget/graphics_scene.h"

static bool operator< (const QPoint &p1, const QPoint &p2)
{
    return p1.x() + p1.y() < p2.x() + p2.y();
}

graph_layouter::graph_layouter(const graph_context* const context, QObject* parent) : QObject(parent),
    m_scene(new graphics_scene(this)),
    m_context(context)
{

}

graphics_scene* graph_layouter::scene() const
{
    return m_scene;
}

const QMap<hal::node, QPoint> graph_layouter::node_to_position_map() const
{
    return m_node_to_position_map;
}

const QMap<QPoint, hal::node> graph_layouter::position_to_node_map() const
{
    return m_position_to_node_map;
}

void graph_layouter::set_node_position(const hal::node &n, const QPoint &p)
{
    if (m_node_to_position_map.contains(n))
    {
        QPoint old_p = m_node_to_position_map.value(n);
        m_position_to_node_map.remove(old_p);
    }

    m_node_to_position_map.insert(n, p);
    m_position_to_node_map.insert(p, n);
}

void graph_layouter::remove_node_from_maps(const hal::node &n)
{
    if (m_node_to_position_map.contains(n))
    {
        QPoint old_p = m_node_to_position_map.value(n);
        m_node_to_position_map.remove(n);
        m_position_to_node_map.remove(old_p);
    }
}
