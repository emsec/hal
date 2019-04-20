#include "graph_widget/layouters/minimal_graph_layouter.h"
#include "graph_widget/graph_scene_manager.h"
#include "graph_widget/graphics_items/graphics_gate.h"
#include "graph_widget/graphics_items/separated_graphics_net.h"
#include "graph_widget/graphics_items/standard_graphics_net.h"
#include "gui_globals.h"

#include "graph_widget/graphics_items/standard_graphics_gate.h"

minimal_graph_layouter::minimal_graph_layouter()
{
    m_name        = "Minimal Layouter";
    m_description = "<p>A fast and simple layouting strategy, nodes are randomly place in a grid and nets are drawn as straight lines</p>";
}

void minimal_graph_layouter::layout(graph_scene* scene)
{
    m_scene = scene;

    int x;
    int y;
    int i = 0;
    QList<graphics_gate*> nodes;
    QMap<std::shared_ptr<gate>, graphics_gate*> map;

    for (auto& g : g_netlist->get_gates())
    {
        graphics_gate* node_item = new standard_graphics_gate(g);
        nodes.append(node_item);
        map.insert(g, node_item);
        x = i * 200 % 20000;
        y = i / 100 * 100;
        node_item->setPos(QPointF(x, y));
        m_scene->addItem(node_item);
        i++;
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
    //}

    for (auto& n : g_netlist->get_nets())
    {
        if (n->is_unrouted())
            continue;

        standard_graphics_net* net_item = new standard_graphics_net(n);
        endpoint src_end                = n->get_src();
        graphics_gate* src_node         = map.value(src_end.gate);
        if (!src_node)
        {
            //no src node TODO handle
            continue;
        }

        QPointF src_position = src_node->get_output_pin_scene_position(QString::fromStdString(src_end.pin_type));
        net_item->setPos(src_position);

        for (endpoint& dst_end : n->get_dsts())
        {
            graphics_gate* dst_node = map.value(dst_end.gate);
            if (!dst_node)
            {
                //no dst node TODO handle
                continue;
            }

            QPointF dst_position = dst_node->get_input_pin_scene_position(QString::fromStdString(dst_end.pin_type));
            net_item->line_to(dst_position);
            net_item->move_pen_to(src_position);
        }
        net_item->setZValue(-1);
        net_item->finalize();
        m_scene->addItem(net_item);
        i++;
    }
}

void minimal_graph_layouter::reset()
{
}

void minimal_graph_layouter::handle_netlist_event(netlist_event_handler::event ev, std::shared_ptr<netlist> netlist, u32 associated_data)
{
    Q_UNUSED(ev);
    Q_UNUSED(netlist);
    Q_UNUSED(associated_data);
}
void minimal_graph_layouter::handle_gate_event(gate_event_handler::event ev, std::shared_ptr<gate> gate, u32 associated_data)
{
    Q_UNUSED(ev);
    Q_UNUSED(gate);
    Q_UNUSED(associated_data);
}
void minimal_graph_layouter::handle_net_event(net_event_handler::event ev, std::shared_ptr<net> net, u32 associated_data)
{
    Q_UNUSED(ev);
    Q_UNUSED(net);
    Q_UNUSED(associated_data);
}
void minimal_graph_layouter::handle_module_event(module_event_handler::event ev, std::shared_ptr<module> module, u32 associated_data)
{
    Q_UNUSED(ev);
    Q_UNUSED(module);
    Q_UNUSED(associated_data);
}
