#include "gui/graph_widget/layouters/orthogonal_graph_layouter.h"

#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_widget/graphics_scene.h"
#include "gui/graph_widget/graphics_gate_factory.h"
#include "gui/graph_widget/graphics_items/separated_graphics_net.h"
#include "gui/graph_widget/graphics_items/unrestricted_graphics_net.h"
#include "gui/gui_globals.h"

#include <qmath.h>

orthogonal_graph_layouter::orthogonal_graph_layouter(graph_context* context) : graph_layouter(context)
{

}

void orthogonal_graph_layouter::layout()
{
    m_scene->delete_all_items();

    if (!m_context)
        return; // SHOULD NEVER BE REACHED

    QVector<graphics_node*> nodes;

    QMap<u32, graphics_gate*> gate_map;
    QMap<u32, graphics_module*> module_map;

    for (const u32& id : m_context->gates())
    {
        std::shared_ptr<gate> g = g_netlist->get_gate_by_id(id);
        graphics_gate* item = graphics_gate_factory::create_graphics_gate(g, 1);

        nodes.append(item);
        gate_map.insert(id, item);
    }

    for (const u32& id : m_context->modules())
    {
        std::shared_ptr<module> m = g_netlist->get_module_by_id(id);
        graphics_module* item = graphics_gate_factory::create_graphics_module(m, 0);

        nodes.append(item);
        module_map.insert(id, item);
    }

    int max_node_width = 0;
    int max_node_height = 0;

    for (const graphics_node* n : nodes)
    {
        if (n->width() > max_node_width)
            max_node_width = n->width();

        if (n->height() > max_node_height)
            max_node_height = n->height();
    }

    int root = sqrt(nodes.size());
    int x = 0;
    int y = 0;

    for (int i = 0; i < nodes.size(); ++i)
    {
        nodes[i]->setPos(x * max_node_width + x * 30, y * max_node_height + y * 30);
        m_scene->addItem(nodes[i]);

        ++x;
        if (x == root)
        {
            x = 0;
            ++y;
        }
    }

//    for (auto n : g_netlist->get_nets())
//    {
//        endpoint src_end = n->get_src();
//        graphics_gate* src_node = map.value(src_end.gate);
//        if (!src_node)
//        {
//            //no src node
//            separated_graphics_net* net_item = new separated_graphics_net(QString::fromStdString(n->get_name()), n);
//            for (endpoint dst_end : n->get_dsts())
//            {
//                graphics_gate* dst_node = map.value(dst_end.gate);
//                if (!dst_node)
//                {
//                    //no dst node
//                    continue;
//                }

//                QPointF dst_position = dst_node->get_mapped_input_pin_position(QString::fromStdString(dst_end.pin_type));
//                net_item->add_input(dst_position);
//            }
//            m_scene->addItem(net_item);
//        }
//        i++;
//    }

//    for (auto& n : g_netlist->get_nets())
//    {
//        if (n->is_unrouted())
//            continue;

//        endpoint src_end        = n->get_src();
//        graphics_gate* src_node = map.value(src_end.gate);
//        if (!src_node)
//        {
//            continue;
//        }

//        QPointF src_position            = src_node->get_output_scene_position(n->get_id(), QString::fromStdString(src_end.pin_type));
//        unrestricted_graphics_net* net_item = new unrestricted_graphics_net(n);
//        net_item->setPos(src_position);

//        for (endpoint& dst_end : n->get_dsts())
//        {
//            graphics_gate* dst_node = map.value(dst_end.gate);
//            if (!dst_node)
//            {
//                continue;
//            }

//            QPointF dst_position = dst_node->get_input_scene_position(n->get_id(), QString::fromStdString(dst_end.pin_type));

//            //            net_item->line_to(QPointF(net_item->current_scene_position().x() + 2, net_item->current_scene_position().y()));
//            //            net_item->line_to(QPointF(net_item->current_scene_position().x(), dst_position.y()));
//            //            net_item->line_to(QPointF(dst_position.x(), net_item->current_scene_position().y()));
//            //            net_item->move_pen_to(src_position);

//            QPointF position = QPointF(src_position.x() + 2, src_position.y());
//            net_item->line_to(position);
//            position = QPointF(position.x(), dst_position.y());
//            net_item->line_to(position);
//            net_item->line_to(QPointF(dst_position.x(), position.y()));
//            net_item->move_pen_to(src_position);
//        }
//        net_item->setZValue(-1);
//        net_item->finalize();
//        m_scene->addItem(net_item);
//        i++;
//    }
}

void orthogonal_graph_layouter::reset()
{
    m_scene->clear();
}

void orthogonal_graph_layouter::add(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets)
{
    Q_UNUSED(modules)
    Q_UNUSED(gates)
    Q_UNUSED(nets)
}

void orthogonal_graph_layouter::remove(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets)
{
    Q_UNUSED(modules)
    Q_UNUSED(gates)
    Q_UNUSED(nets)
}

void orthogonal_graph_layouter::expand(const u32 from_gate, const u32 via_net, const u32 to_gate)
{
    Q_UNUSED(from_gate)
    Q_UNUSED(via_net)
    Q_UNUSED(to_gate)
}

const QString orthogonal_graph_layouter::name() const
{
    return "Orthogonal Layouter";
}

const QString orthogonal_graph_layouter::description() const
{
    return "<p>A fast and simple layouting strategy, nodes are randomly place in a grid and nets are drawn as orthogonal lines</p>";
}
