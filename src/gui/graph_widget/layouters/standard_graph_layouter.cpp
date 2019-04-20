#include "graph_widget/layouters/standard_graph_layouter.h"
#include "graph_widget/graph_scene_manager.h"
#include "graph_widget/graphics_items/global_graphics_net.h"
#include "graph_widget/graphics_items/graphics_gate.h"
#include "graph_widget/graphics_items/separated_graphics_net.h"
#include "graph_widget/graphics_items/standard_graphics_net.h"
#include "gui_globals.h"
#include <QPointF>
#include <QTime>
#include <cmath>
#include <core/log.h>
#include <math.h>
#include <netlist/net.h>
#include <queue>

#include "graph_widget/graphics_items/standard_graphics_gate.h"

int standard_graph_layouter::s_max_level_size = 60;

standard_graph_layouter::standard_graph_layouter()
{
    m_name        = "Standard Layouter";
    m_description = "<p>The standard layouting strategy :DD</p>";
}

void standard_graph_layouter::layout(graph_scene* scene)
{
    m_scene = scene;

    for (auto& g : g_netlist->get_gates())
    {
        graphics_gate* graphics_node = new standard_graphics_gate(g);
        m_gate_map.insert(g, graphics_node);
    }

    m_highest_level = 0;

    compute_gate_levels();
    compress_gate_levels();
    spin_gate_levels();
    place_gates();
    draw_global_nets();
    draw_separated_nets();
    draw_standard_nets();
    //    compute_channel_width();
}

void standard_graph_layouter::reset()
{
    m_gate_map.clear();
    m_net_map.clear();

    m_gate_level.clear();
    m_highest_level = 0;

    m_separated_nets.clear();
    m_gate_layout.clear();

    m_channel_width     = 50;
    m_sub_channel_count = 10;
}

void standard_graph_layouter::handle_netlist_event(netlist_event_handler::event ev, std::shared_ptr<netlist> netlist, u32 associated_data)
{
    Q_UNUSED(ev);
    Q_UNUSED(netlist);
    Q_UNUSED(associated_data);
}
void standard_graph_layouter::handle_gate_event(gate_event_handler::event ev, std::shared_ptr<gate> gate, u32 associated_data)
{
    Q_UNUSED(ev);
    Q_UNUSED(gate);
    Q_UNUSED(associated_data);
}
void standard_graph_layouter::handle_net_event(net_event_handler::event ev, std::shared_ptr<net> net, u32 associated_data)
{
    Q_UNUSED(ev);
    Q_UNUSED(net);
    Q_UNUSED(associated_data);
}
void standard_graph_layouter::handle_module_event(module_event_handler::event ev, std::shared_ptr<module> module, u32 associated_data)
{
    Q_UNUSED(ev);
    Q_UNUSED(module);
    Q_UNUSED(associated_data);
}

void standard_graph_layouter::compute_gate_levels()
{
    QList<std::shared_ptr<gate>> unvisited;

    for (auto& g : g_netlist->get_gates())
        unvisited.append(g);

    std::queue<std::shared_ptr<gate>> in_progress;

    for (auto& net : g_netlist->get_global_input_nets())
    {
        for (endpoint& end : net->get_dsts())
        {
            std::shared_ptr<gate> dst = end.gate;
            int index                 = unvisited.indexOf(dst);
            if (index != -1)
            {
                in_progress.push(dst);
                unvisited.removeAt(index);
                m_gate_level.insert(dst, 0);
            }
        }
    }

    while (!in_progress.empty())
    {
        std::shared_ptr<gate> g = in_progress.front();
        int node_level          = m_gate_level.value(g);

        for (endpoint& end : g->get_successors())
        {
            std::shared_ptr<gate> successor = end.gate;
            int index                       = unvisited.indexOf(successor);
            if (index != -1)
            {
                unvisited.removeAt(index);
                in_progress.push(successor);
                m_gate_level.insert(successor, node_level + 1);

                if (node_level + 1 > m_highest_level)
                    m_highest_level = node_level + 1;
            }
        }
        in_progress.pop();
    }

    for (auto& g : unvisited)
    {
        int lowest_successor_level    = m_highest_level;
        bool has_successor_with_level = false;

        for (endpoint& end : g->get_successors())
        {
            int index = unvisited.indexOf(end.gate);
            if (index == -1)
            {
                int level = m_gate_level.value(end.gate);
                if (level < lowest_successor_level)
                    lowest_successor_level = level;
                has_successor_with_level = true;
            }
        }

        if (!has_successor_with_level)
            m_gate_level.insert(g, 0);
        else
        {
            if (lowest_successor_level <= 0)
                m_gate_level.insert(g, 0);
            else
                m_gate_level.insert(g, lowest_successor_level - 1);
        }
    }

    for (int i = 0; i <= m_highest_level; i++)
        m_gate_layout.push_back(std::list<graphics_gate*>());

    QMap<std::shared_ptr<gate>, int>::const_iterator i = m_gate_level.constBegin();
    while (i != m_gate_level.constEnd())
    {
        //TODO change... maybe
        if (i.value() < 0)
        {
            std::list<std::list<graphics_gate*>>::iterator it = m_gate_layout.begin();
            it->push_back(m_gate_map.value(i.key()));
        }
        else
        {
            std::list<std::list<graphics_gate*>>::iterator it = std::next(m_gate_layout.begin(), i.value());
            it->push_back(m_gate_map.value(i.key()));
        }
        ++i;
    }
}

void standard_graph_layouter::compress_gate_levels()
{
    std::list<std::list<graphics_gate*>>::iterator layout_it = m_gate_layout.begin();

    while (layout_it != m_gate_layout.end())
    {
        int level_size = layout_it->size();

        while (level_size > s_max_level_size)
        {
            std::list<graphics_gate*>::iterator from_it = layout_it->begin();
            std::list<graphics_gate*>::iterator to_it   = std::next(layout_it->begin(), s_max_level_size);
            std::list<graphics_gate*> new_level;
            new_level.splice(new_level.begin(), *layout_it, from_it, to_it);
            m_gate_layout.insert(layout_it, new_level);
            level_size -= s_max_level_size;
        }
        layout_it++;
    }
}

void standard_graph_layouter::spin_gate_levels()
{
    //TODO
}

void standard_graph_layouter::place_gates()
{
    //channel width start
    //Saves the maximum #input/output pins per level
    int input_i  = 0;
    int output_i = 0;
    int output_m = 0;

    //a minimum distance is needed,so it starts with 30 instead of 0
    int max_pins = 30;

    {
        int level = 0;
        for (std::list<graphics_gate*>& list : m_gate_layout)
        {
            output_m = output_i;
            input_i  = 0;
            output_i = 0;

            int y = 0;
            for (graphics_gate*& node : list)
            {
                input_i += node->get_gate()->get_input_pin_types().size();
                output_i += node->get_gate()->get_output_pin_types().size();
                y++;
            }

            //First level cant be used to calculate the width
            if (level == 0)
                continue;

            //Important part here
            if (output_m + input_i > max_pins)
                max_pins = output_m + input_i;

            level++;
        }
    }

    //for starters, take 45% of the calculated width, may be reduced later(Depending on the size of the graph, the bigger the less space is needed)
    max_pins            = 0.25 * max_pins;
    m_sub_channel_count = max_pins;

    m_channel_width = max_pins * 16;    //16 = line_width

    //channel width end

    int max_rect_width  = m_max_gate_width + m_channel_width;
    int max_rect_height = m_max_gate_height + m_channel_width;

    int level = 0;
    for (std::list<graphics_gate*>& list : m_gate_layout)
    {
        int y = 0;
        for (graphics_gate*& node : list)
        {
            node->setX(level * max_rect_width);
            node->setY(y * max_rect_height);
            m_scene->addItem(node);
            y++;
        }
        level++;
    }
}

void standard_graph_layouter::draw_global_nets()
{
    QList<std::shared_ptr<net>> handeled;

    for (auto& n : g_netlist->get_global_inout_nets())
    {
        endpoint src_end        = n->get_src();
        graphics_gate* src_node = m_gate_map.value(src_end.gate);
        if (!src_node)
        {
            //no src node
            //error
            continue;
        }
        QPointF src_position          = src_node->get_input_pin_scene_position(QString::fromStdString(src_end.pin_type));
        global_graphics_net* net_item = new global_graphics_net(n);
        net_item->setPos(src_position);
        net_item->add_output();

        for (endpoint& dst_end : n->get_dsts())
        {
            graphics_gate* dst_node = m_gate_map.value(dst_end.gate);
            if (!dst_node)
            {
                //no dst node
                //error
                continue;
            }

            QPointF dst_position = dst_node->get_input_pin_scene_position(QString::fromStdString(dst_end.pin_type));
            net_item->add_input(dst_position);
        }
        net_item->finalize();
        m_scene->addItem(net_item);
        handeled.append(n);
    }

    for (auto& n : g_netlist->get_global_input_nets())
    {
        if (handeled.contains(n))
            continue;

        global_graphics_net* net_item = new global_graphics_net(n);
        net_item->setPos(QPointF(0, 0));

        for (endpoint& dst_end : n->get_dsts())
        {
            graphics_gate* dst_node = m_gate_map.value(dst_end.gate);
            if (!dst_node)
            {
                //no dst node
                //error
                continue;
            }

            QPointF dst_position = dst_node->get_input_pin_scene_position(QString::fromStdString(dst_end.pin_type));
            net_item->add_input(dst_position);
        }
        net_item->finalize();
        m_scene->addItem(net_item);
        handeled.append(n);
    }

    for (auto& n : g_netlist->get_global_output_nets())
    {
        if (handeled.contains(n))
            continue;

        endpoint src_end        = n->get_src();
        graphics_gate* src_node = m_gate_map.value(src_end.gate);
        if (!src_node)
        {
            //no src node
            //error
            continue;
        }
        QPointF src_position          = src_node->get_output_pin_scene_position(QString::fromStdString(src_end.pin_type));
        global_graphics_net* net_item = new global_graphics_net(n);
        net_item->setPos(src_position);
        net_item->add_output();
        net_item->finalize();
        m_scene->addItem(net_item);
        handeled.append(n);
    }
}

void standard_graph_layouter::draw_separated_nets()
{
    std::set<std::shared_ptr<gate>> set     = g_netlist->get_global_gnd_gates();
    std::set<std::shared_ptr<gate>> vcc_set = g_netlist->get_global_vcc_gates();

    set.insert(vcc_set.begin(), vcc_set.end());

    for (auto& g : set)
    {
        for (auto& n : g->get_fan_out_nets())
        {
            graphics_gate* src_node = m_gate_map.value(g);

            if (!src_node)
            {
                //no src node
                //error
                continue;
            }

            QPointF src_position             = src_node->get_output_pin_scene_position(QString::fromStdString(n->get_src().pin_type));
            separated_graphics_net* net_item = new separated_graphics_net(QString::fromStdString(n->get_name()), n);
            net_item->setPos(src_position);
            net_item->add_output();

            for (endpoint& dst_end : n->get_dsts())
            {
                graphics_gate* dst_node = m_gate_map.value(dst_end.gate);
                if (!dst_node)
                {
                    //no dst node
                    //error
                    continue;
                }

                QPointF dst_position = dst_node->get_input_pin_scene_position(QString::fromStdString(dst_end.pin_type));
                net_item->add_input(dst_position);
            }
            net_item->finalize();
            m_scene->addItem(net_item);
            m_separated_nets.push_back(n);
        }
    }
}

void standard_graph_layouter::compute_channel_width()
{
}

//void distance_graph_layouter::slap_nodes_on_scene()
//{
//    int level = 0;
//    for (std::list<graphics_gate*> list : m_node_layout)
//    {
//        int y = 0;
//        for (graphics_gate* node : list)
//        {
//            node->setX(level * 200);
//            node->setY(y * 100);
//            graph_scene_manager::get_instance().get_scene().addItem(node);
//            y++;
//        }
//        level++;
//    }
//}

void standard_graph_layouter::draw_standard_nets()
{
    std::set<std::shared_ptr<net>> left = g_netlist->get_nets();

    int sub_channel = 0;

    for (std::list<graphics_gate*>& list : m_gate_layout)
    {
        for (graphics_gate*& node : list)
        {
            graphics_gate* graphics_node = node;

            unsigned int output_pin_count = graphics_node->get_gate()->get_output_pin_types().size();
            if (output_pin_count == 0)
                continue;

            for (unsigned int current_pin = 0; current_pin < output_pin_count; current_pin++)
            {
                std::shared_ptr<gate> current_graph_node = graphics_node->get_gate();
                std::string pin                          = *std::next(current_graph_node->get_output_pin_types().begin(), current_pin);
                std::shared_ptr<net> current_net         = current_graph_node->get_fan_out_net(pin);
                if (!current_net)
                    continue;

                if (std::find(m_separated_nets.begin(), m_separated_nets.end(), current_net) != m_separated_nets.end())
                    continue;

                left.erase(current_net);
                calculate_net(current_net, sub_channel);
                sub_channel++;

                if (sub_channel == m_sub_channel_count)
                    sub_channel = 0;
            }
        }
        sub_channel = 0;
    }
}

void standard_graph_layouter::calculate_net(std::shared_ptr<net> n, int sub_channel)
{
    graphics_gate* src_gui_node;
    QPointF src_pin;
    std::vector<QPointF> dst_pins;
    int line_width = 1;

    std::shared_ptr<gate> src_node = n->get_src().gate;
    src_gui_node                   = m_gate_map.value(src_node);
    src_pin                        = src_gui_node->get_output_pin_scene_position(QString::fromStdString(n->get_src().pin_type));

    for (auto& tup : n->get_dsts())
    {
        graphics_gate* temp_dst = m_gate_map.value(tup.gate);
        QPointF temp_dst_pin    = temp_dst->get_input_pin_scene_position(QString::fromStdString(tup.pin_type));
        dst_pins.push_back(temp_dst_pin);
    }

    int line = sub_channel;

    standard_graphics_net* graphics_net = new standard_graphics_net(n);
    graphics_net->setPos(src_pin.x(), src_pin.y());
    graphics_net->line_to(QPointF(src_pin.x() + 5, src_pin.y()));

    for (QPointF& dst_point : dst_pins)
    {
        if (src_pin.x() < dst_point.x())
        {
            if (dst_point.x() > (m_max_gate_width + m_channel_width))    //bigger than channel_width, then "dodge"
            {
                int down = src_gui_node->pos().y() + m_max_gate_height - src_pin.y();

                int to_right = src_gui_node->pos().x() + m_max_gate_width - src_pin.x();

                graphics_net->line_to(QPointF(to_right + 30 + line * line_width, graphics_net->current_scene_position().y()));
                graphics_net->line_to(
                    QPointF(graphics_net->current_scene_position().x(), graphics_net->current_scene_position().y() + down + line * line_width + line_width));    //+line_width because line could be 0
                graphics_net->line_to(
                    QPointF(dst_point.x() - (line + 1) * line_width, graphics_net->current_scene_position().y()));    //dst_points ist kein vielfaches von line_width...., iwas muss angepasst werden
                graphics_net->line_to(QPointF(graphics_net->current_scene_position().x(), dst_point.y()));
                graphics_net->line_to(QPointF(dst_point.x(), dst_point.y()));
            }
            else
            {
                //ordinary left to right net if dodging is not needed
                //channel 0 is the most left, channel 1 the second most etc ###[Version 2]###
                int to_right = src_gui_node->pos().x() + m_max_gate_width - src_pin.x();

                graphics_net->line_to(QPointF(to_right + 30 + line * line_width, graphics_net->current_scene_position().y()));
                graphics_net->line_to(QPointF(graphics_net->current_scene_position().x(), dst_point.y()));
                graphics_net->line_to(QPointF(dst_point.x(), graphics_net->current_scene_position().y()));
            }
        }
        else
        {
            int down = src_gui_node->pos().y() + m_max_gate_height - src_pin.y();

            graphics_net->line_to(QPointF(20 + line_width * line, 0));
            graphics_net->line_to(QPointF(graphics_net->current_scene_position().x(), down + 20 + 5 * line));    //Hier die lines und die größte node beachten beim runtergehen
            graphics_net->line_to(QPointF(dst_point.x() - line_width * (line + 1), graphics_net->current_scene_position().y()));
            graphics_net->line_to(QPointF(graphics_net->current_scene_position().x(), dst_point.y()));
            graphics_net->line_to(QPointF(dst_point.x(), dst_point.y()));
        }

        graphics_net->move_pen_to(QPointF(src_pin.x() + 5, src_pin.y()));
    }

    graphics_net->finalize();
    m_scene->addItem(graphics_net);
}
