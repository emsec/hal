#include "graph_widget/layouters/minimal_graph_layouter.h"

#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_widget/graphics_scene.h"
#include "gui/graph_widget/items/graphics_gate.h"
#include "gui/graph_widget/items/io_graphics_net.h"
#include "gui/graph_widget/items/minimal_graphics_gate.h"
#include "gui/graph_widget/items/separated_graphics_net.h"
#include "gui/graph_widget/items/standard_graphics_net.h"
#include "gui/graph_widget/graphics_factory.h"
#include "gui_globals.h"

minimal_graph_layouter::minimal_graph_layouter(graph_context* context) : graph_layouter(context)
{

}

void minimal_graph_layouter::layout()
{
    assert(m_context);

    m_scene->delete_all_items();

    QVector<graphics_node*> nodes;

    QMap<u32, graphics_gate*> gate_map;
    QMap<u32, graphics_module*> module_map;

    for (const u32& id : m_context->gates())
    {
        std::shared_ptr<gate> g = g_netlist->get_gate_by_id(id);
        graphics_gate* item = graphics_factory::create_graphics_gate(g, 0);

        nodes.append(item);
        gate_map.insert(id, item);
    }

    for (const u32& id : m_context->modules())
    {
        std::shared_ptr<module> m = g_netlist->get_module_by_id(id);
        graphics_module* item = graphics_factory::create_graphics_module(m, 0);

        nodes.append(item);
        module_map.insert(id, item);
    }

    qreal max_node_width = 0;
    qreal max_node_height = 0;

    for (const graphics_node* n : nodes)
    {
        if (n->width() > max_node_width)
            max_node_width = n->width();

        if (n->height() > max_node_height)
            max_node_height = n->height();
    }

    int root = static_cast<int>(sqrt(nodes.size()));
    int x = 0;
    int y = 0;

    for (int i = 0; i < nodes.size(); ++i)
    {
        nodes[i]->setPos(x * (max_node_width + 60), y * (max_node_height + 60));
        m_scene->add_item(nodes[i]);

        ++x;
        if (x == root)
        {
            x = 0;
            ++y;
        }
    }

    for (const u32 id : m_context->nets())
    {
        std::shared_ptr<net> n = g_netlist->get_net_by_id(id);

        if (!n)
            continue;

        if (n->is_unrouted())
        {
            // HANDLE GLOBAL NETS
            io_graphics_net* net_item = new io_graphics_net(n);

            endpoint src_end = n->get_src();

            if (src_end.get_gate())
            {
                hal::node node;

                if (!m_context->node_for_gate(node, src_end.get_gate()->get_id()))
                    continue;

                graphics_node* node_item = item_for_node(node, gate_map, module_map);

                if (node_item)
                {
                    net_item->setPos(node_item->get_output_scene_position(n->get_id(), QString::fromStdString(src_end.pin_type)));
                    net_item->add_output();
                }
            }

            for (const endpoint& dst_end : n->get_dsts())
            {
                hal::node node;

                if (!m_context->node_for_gate(node, dst_end.get_gate()->get_id()))
                    continue;

                graphics_node* node_item = item_for_node(node, gate_map, module_map);

                if (node_item)
                    net_item->add_input(node_item->get_input_scene_position(n->get_id(), QString::fromStdString(dst_end.pin_type)));
            }

            net_item->finalize();
            m_scene->add_item(net_item);
            continue;
        }

        if (n->get_src().gate)
        {
            if (n->get_src().gate->is_global_gnd_gate() || n->get_src().gate->is_global_vcc_gate())
            {
                // HANDLE SEPARATED NETS
                hal::node node;

                if (!m_context->node_for_gate(node, n->get_src().get_gate()->get_id()))
                    continue;

                separated_graphics_net* net_item = new separated_graphics_net(n, QString::fromStdString(n->get_name()));

                graphics_node* node_item = item_for_node(node, gate_map, module_map);

                if (node_item)
                {
                    net_item->setPos(node_item->get_output_scene_position(n->get_id(), QString::fromStdString(n->get_src().pin_type)));
                    net_item->add_output();
                }

                for (endpoint& dst_end : n->get_dsts())
                {
                    if (!m_context->node_for_gate(node, dst_end.get_gate()->get_id()))
                        continue;

                    node_item = item_for_node(node, gate_map, module_map);

                    if (node_item)
                        net_item->add_input(node_item->get_input_scene_position(n->get_id(), QString::fromStdString(dst_end.pin_type)));
                }

                net_item->finalize();
                m_scene->add_item(net_item);
                continue;
            }
        }

        // HANDLE NORMAL NETS
        graphics_node* src_node = nullptr;

        hal::node node;

        if (!m_context->node_for_gate(node, n->get_src().get_gate()->get_id()))
            continue;

        src_node = item_for_node(node, gate_map, module_map);

        if (!src_node)
            continue;

        const QPointF src_pin_position = src_node->get_output_scene_position(n->get_id(), QString::fromStdString(n->get_src().pin_type));
        standard_graphics_net::lines lines;
        lines.src_x = src_pin_position.x();
        lines.src_y = src_pin_position.y();

        for (const endpoint& dst : n->get_dsts())
        {
            // FIND DST BOX
            graphics_node* dst_node = nullptr;

            if (!m_context->node_for_gate(node, dst.get_gate()->get_id()))
                continue;

            dst_node = item_for_node(node, gate_map, module_map);

            if (!dst_node)
                continue;

            QPointF dst_pin_position = dst_node->get_input_scene_position(n->get_id(), QString::fromStdString(dst.pin_type));

            if (src_pin_position.x() != dst_pin_position.x())
            {
                qreal small_x = src_pin_position.x();
                qreal big_x = dst_pin_position.x();

                if (small_x > big_x)
                {
                    qreal temp = small_x;
                    small_x = big_x;
                    big_x = temp;
                }

                lines.h_lines.append(standard_graphics_net::h_line{small_x, big_x, dst_pin_position.y()});
            }

            if (src_pin_position.y() != dst_pin_position.y())
            {
                qreal small_y = src_pin_position.y();
                qreal big_y = dst_pin_position.y();

                if (small_y > big_y)
                {
                    qreal temp = small_y;
                    small_y = big_y;
                    big_y = temp;
                }

                lines.v_lines.append(standard_graphics_net::v_line{src_pin_position.x(), small_y, big_y});
            }
        }

        standard_graphics_net* net_item = new standard_graphics_net(n, lines);
        net_item->setPos(src_node->get_output_scene_position(n->get_id(), QString::fromStdString(n->get_src().pin_type)));
        m_scene->add_item(net_item);
    }

    m_scene->move_nets_to_background();
}

void minimal_graph_layouter::add(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets)
{
    Q_UNUSED(modules)
    Q_UNUSED(gates)
    Q_UNUSED(nets)
}

void minimal_graph_layouter::remove(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets)
{
    Q_UNUSED(modules)
    Q_UNUSED(gates)
    Q_UNUSED(nets)
}

void minimal_graph_layouter::expand(const u32 from_gate, const u32 via_net, const u32 to_gate)
{
    Q_UNUSED(from_gate)
    Q_UNUSED(via_net)
    Q_UNUSED(to_gate)
}

const QString minimal_graph_layouter::name() const
{
    return "Minimal Layouter";
}

const QString minimal_graph_layouter::description() const
{
    return "<p>PLACEHOLDER</p>";
}

graphics_node* minimal_graph_layouter::item_for_node(hal::node& node, const QMap<u32, graphics_gate*>& gate_map, const QMap<u32, graphics_module*>& module_map)
{
    graphics_node* item = nullptr;

    switch (node.type)
    {
    case hal::node_type::gate:
    {
        //assert(gate_map.contains(node.id));

        if (gate_map.contains(node.id))
            item = gate_map.value(node.id);

        return item;
    }
    case hal::node_type::module:
    {
        //assert(module_map.contains(node.id));

        if (module_map.contains(node.id))
            item = module_map.value(node.id);

        return item;
    }
    }
}
