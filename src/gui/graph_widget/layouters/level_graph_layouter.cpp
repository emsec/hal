#include "gui/graph_widget/layouters/level_graph_layouter.h"

#include "netlist/module.h"

#include "gui_globals.h"
#include "gui/graph_widget/contexts/graph_context.h"

level_graph_layouter::level_graph_layouter(const graph_context* const context) : graph_layouter(context)
{
}

const QString level_graph_layouter::name() const
{
    return "Level Layouter";
}

const QString level_graph_layouter::description() const
{
    return "<p>PLACEHOLDER</p>";
}

void level_graph_layouter::add(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets)
{
    Q_UNUSED(modules)
    Q_UNUSED(gates)
    Q_UNUSED(nets)

    //    // ZERO GATES
    //    int level_x = 0;

    //    // ARTIFICIAL SCOPE TO AVOID SHADOWING
    //    {
    //        int x_offset = 0;
    //        int y        = 0;

    //        int root = sqrt(m_zero_nodes.size());
    //        for (const hal::node& node : m_zero_nodes)
    //        {
    //            m_boxes.append(create_box(node, level_x + x_offset, y));

    //            if (x_offset + 1 == root)
    //            {
    //                x_offset = 0;
    //                ++y;
    //            }
    //            else
    //                ++x_offset;
    //        }
    //        level_x += root;
    //    }

    //    // POSITIVE GATES
    //    for (QVector<hal::node>& v : m_positive_nodes)
    //    {
    //        int x_offset = 0;
    //        int y        = 0;

    //        int root = sqrt(v.size());
    //        for (const hal::node& node : v)
    //        {
    //            m_boxes.append(create_box(node, level_x + x_offset, y));

    //            if (x_offset + 1 == root)
    //            {
    //                x_offset = 0;
    //                ++y;
    //            }
    //            else
    //                ++x_offset;
    //        }
    //        level_x += root;
    //    }

    //    // NEGATIVE GATES
    //    level_x = -1;

    //    for (QVector<hal::node>& v : m_negative_nodes)
    //    {
    //        int x_offset = 0;
    //        int y        = 0;

    //        int root = sqrt(v.size());
    //        for (const hal::node& node : v)
    //        {
    //            m_boxes.append(create_box(node, level_x + x_offset, y));

    //            if (abs(x_offset - 1) == root)
    //            {
    //                x_offset = 0;
    //                ++y;
    //            }
    //            else
    //                --x_offset;
    //        }
    //        level_x -= root;
    //    }
}

void level_graph_layouter::remove(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets)
{
    Q_UNUSED(modules)
    Q_UNUSED(gates)
    Q_UNUSED(nets)

    for (u32 id : modules)
        remove_node_from_maps({hal::node_type::module, id});

    for (u32 id : gates)
        remove_node_from_maps({hal::node_type::gate, id});
}

void level_graph_layouter::add_gate(const u32 gate_id, const int level)
{
    m_node_levels.insert(hal::node{hal::node_type::gate, gate_id}, level);

    if (level == 0)
    {
        m_zero_nodes.append(hal::node{hal::node_type::gate, gate_id});
    }
    else
    {
        if (level > 0)
        {
            if (level > m_positive_nodes.size())
            {
                m_positive_nodes.append(QVector<hal::node>());
                m_positive_nodes.last().append(hal::node{hal::node_type::gate, gate_id});
            }
            else
                m_positive_nodes[level - 1].append(hal::node{hal::node_type::gate, gate_id});
        }
        else    // if (new_level < 0)
        {
            int abs_level = abs(level);

            if (abs_level > m_negative_nodes.size())
            {
                m_negative_nodes.append(QVector<hal::node>());
                m_negative_nodes.last().append(hal::node{hal::node_type::gate, gate_id});
            }
            else
                m_negative_nodes[abs_level - 1].append(hal::node{hal::node_type::gate, gate_id});
        }
    }
}

void level_graph_layouter::recalculate_levels()
{
    // INEFFICIENT BUT EASIER, OPTIMIZE LATER...
    m_node_levels.clear();
    m_zero_nodes.clear();
    m_positive_nodes.clear();
    m_negative_nodes.clear();

    QVector<hal::node> unvisited;

    for (u32 id : m_context->gates())
        unvisited.append(hal::node{hal::node_type::gate, id});

    for (u32 id : m_context->modules())
        unvisited.append(hal::node{hal::node_type::module, id});

    // LEVEL ZERO
    QSet<u32> level_zero_gates;

    for (const u32 id : m_context->nets())
    {
        std::shared_ptr<net> n = g_netlist->get_net_by_id(id);
        assert(n);

        if (n->get_src().get_gate())
            level_zero_gates.insert(n->get_src().get_gate()->get_id());

        for (const endpoint& e : n->get_dsts())
            if (e.get_gate())
                level_zero_gates.remove(e.get_gate()->get_id());
    }

    for (const u32 id : m_context->nets())
    {
        // SEEMS HACKY
        std::shared_ptr<net> n = g_netlist->get_net_by_id(id);
        assert(n);

        if (n->is_unrouted())
            for (const endpoint& e : n->get_dsts())
                if (e.get_gate())
                    level_zero_gates.insert(e.get_gate()->get_id());
    }

    for (const u32 id : level_zero_gates)
    {
        hal::node node;

        if (!m_context->node_for_gate(node, id))
            continue;

        if (!m_zero_nodes.contains(node))
        {
            m_node_levels.insert(node, 0);
            m_zero_nodes.append(node);

            unvisited.removeOne(node);
        }
    }

    // REMAINING LEVELS
    int level = 1;

    while (!unvisited.isEmpty())
    {
        QVector<hal::node> visited;

        for (const hal::node& node : unvisited)
        {
            switch (node.type)
            {
                case hal::node_type::module:
                {
                    std::shared_ptr<module> m = g_netlist->get_module_by_id(node.id);

                    for (const std::shared_ptr<net>& n : m->get_input_nets())
                    {
                        hal::node src_node;

                        if (n->is_unrouted())
                            continue;

                        if (!m_context->node_for_gate(src_node, n->get_src().gate->get_id()))
                            continue;

                        if (m_node_levels.contains(src_node))
                        {
                            visited.append(node);
                            break;
                        }
                    }

                    break;
                }
                case hal::node_type::gate:
                {
                    std::shared_ptr<gate> g = g_netlist->get_gate_by_id(node.id);

                    for (std::shared_ptr<net> n : g->get_fan_in_nets())
                    {
                        hal::node src_node;

                        if (n->is_unrouted())
                            continue;

                        if (!m_context->node_for_gate(src_node, n->get_src().gate->get_id()))
                            continue;

                        if (m_node_levels.contains(src_node))
                        {
                            visited.append(node);
                            break;
                        }
                    }

                    break;
                }
            }
        }

        if (visited.isEmpty())
        {
            // HACK SOLUTIONS TM
            // PROBABLY GOING TO NEED SOME KIND OF GROUP SYSTEM
            for (const hal::node& node : unvisited)
            {
                m_node_levels.insert(node, 0);
                m_zero_nodes.append(node);
            }

            return;
        }
        else
        {
            if (m_positive_nodes.size() < level)
                m_positive_nodes.append(QVector<hal::node>());

            for (const hal::node& node : visited)
            {
                m_node_levels.insert(node, level);
                m_positive_nodes[level - 1].append(node);

                unvisited.removeOne(node);
            }

            ++level;
        }
    }
}
