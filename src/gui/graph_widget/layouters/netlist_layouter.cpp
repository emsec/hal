#include "graph_widget/layouters/netlist_layouter.h"
#include "graph_widget/graph_scene.h"
#include "graph_widget/graphics_items/global_graphics_net.h"
#include "graph_widget/graphics_items/separated_graphics_net.h"
#include "graph_widget/graphics_items/standard_graphics_net.h"
#include "gui_globals.h"
#include <QQueue>
#include <math.h>

#include "graph_widget/graphics_items/red_pin_gate.h"
#include "graph_widget/graphics_items/standard_graphics_gate.h"

netlist_layouter::netlist_layouter()
{
    m_name        = "Netlist Layouter";
    m_description = "<p><h2>The layouter of the future. Its perfect design optimizes the relation between memory usage and computing speed.<br>"
                    "If you check it out just once, you won't ever want to get back!</h2><br>"
                    "<b><font color=\"red\"><h3>Only 4.99!**</h3></font></b><br>**Limited time while stocks last!</p>";
    init_settings();
}

void netlist_layouter::layout(graph_scene* scene)
{
    m_scene = scene;

    //execute all substeps for the perfect layout experience
    generate_gui_gates();
    compute_gate_levels();

    if (m_squarify_entire_design)
        squarify_entire_design();
    if (m_compress_gate_levels)
        compress_gate_levels();
    if (m_compute_cw_scc)
        compute_cw_and_scc();

    apply_gate_layout();
    draw_nets();
}

void netlist_layouter::reset()
{
    m_gate_to_gui_gate_map.clear();
    m_gate_to_level.clear();
    m_layout.clear();
    m_layout.squeeze();

    init_settings();
}

void netlist_layouter::handle_netlist_event(netlist_event_handler::event ev, std::shared_ptr<netlist> netlist, u32 associated_data)
{
    Q_UNUSED(ev);
    Q_UNUSED(netlist);
    Q_UNUSED(associated_data);
}
void netlist_layouter::handle_gate_event(gate_event_handler::event ev, std::shared_ptr<gate> gate, u32 associated_data)
{
    Q_UNUSED(ev);
    Q_UNUSED(gate);
    Q_UNUSED(associated_data);
}
void netlist_layouter::handle_net_event(net_event_handler::event ev, std::shared_ptr<net> net, u32 associated_data)
{
    Q_UNUSED(ev);
    Q_UNUSED(net);
    Q_UNUSED(associated_data);
}
void netlist_layouter::handle_module_event(module_event_handler::event ev, std::shared_ptr<module> module, u32 associated_data)
{
    Q_UNUSED(ev);
    Q_UNUSED(module);
    Q_UNUSED(associated_data);
}

void netlist_layouter::generate_gui_gates()
{
    for (std::shared_ptr<gate> g : g_netlist->get_gates())
    {
        graphics_gate* new_gate = new standard_graphics_gate(g);
        //graphics_gate* new_gate = new red_pin_gate(g);
        m_max_gate_height = std::max(m_max_gate_height, new_gate->get_height());
        m_max_gate_width  = std::max(m_max_gate_width, new_gate->get_width());
        m_min_gate_height = std::min(m_min_gate_height, new_gate->get_height());
        m_min_gate_width  = std::min(m_min_gate_width, new_gate->get_width());

        m_gate_to_gui_gate_map[g] = new_gate;
    }
}

void netlist_layouter::compute_gate_levels()
{
    //helper variable, needed to specify the level for the unvisited gates
    //after the BSF
    int highest_lvl = 0;

    //not every gate can be found with the following BFS, 'unvisited' keeps track of which are left
    QList<std::shared_ptr<gate>> unvisited;
    for (auto& g : g_netlist->get_gates())
        unvisited.append(g);

    //main data structure for the BFS
    QQueue<std::shared_ptr<gate>> in_progress;

    //BFS starts with the global input gates (gates at the left, starts with level 0)
    for (auto& net : g_netlist->get_global_input_nets())
        for (endpoint& point : net->get_dsts())
        {
            std::shared_ptr<gate> global_input_gate = point.gate;
            in_progress.enqueue(global_input_gate);
            unvisited.removeAll(global_input_gate);
            m_gate_to_level[global_input_gate] = 0;
        }

    //after the initialization is done, start with the standard BFS
    while (!in_progress.isEmpty())
    {
        std::shared_ptr<gate> current_gate = in_progress.dequeue();
        int gate_lvl                       = m_gate_to_level[current_gate];

        for (endpoint& point : current_gate->get_successors())    //BUG: THIS IS NOT ORDERED (Maybe)
        {
            std::shared_ptr<gate> successor = point.gate;
            if (unvisited.contains(successor))
            {
                unvisited.removeAll(successor);
                in_progress.enqueue(successor);
                m_gate_to_level[successor] = gate_lvl + 1;

                if (gate_lvl + 1 > highest_lvl)
                    highest_lvl = gate_lvl + 1;
            }
        }
    }

    //at last we compute the level of the gates we could not find with the BFS
    for (std::shared_ptr<gate> g : unvisited)
    {
        int lowest_successor_lvl    = highest_lvl;
        bool has_successor_with_lvl = false;

        for (endpoint& point : g->get_successors())
        {
            if (!unvisited.contains(point.gate))
            {
                int lvl = m_gate_to_level[point.gate];
                if (lvl < lowest_successor_lvl)
                    lowest_successor_lvl = lvl;
                has_successor_with_lvl = true;
            }
        }

        if (!has_successor_with_lvl)
            m_gate_to_level[g] = 0;
        else
            (lowest_successor_lvl <= 0) ? (m_gate_to_level[g] = 0) : (m_gate_to_level[g] = lowest_successor_lvl - 1);
    }

    //Use a map to have sorted keys(in this case the ids) to ensure the same layout result each time,
    //its also better than list because you have instand access to the gates(do not need to be looked up in the netlist)
    QMap<int, std::shared_ptr<gate>> ordered_gates;
    for (auto& gate : g_netlist->get_gates())
        ordered_gates.insert(gate->get_id(), gate);

    m_layout.resize(highest_lvl + 1);
    for (QMap<int, std::shared_ptr<gate>>::const_iterator it = ordered_gates.constBegin(); it != ordered_gates.constEnd(); it++)
        m_layout[m_gate_to_level[it.value()]].append(m_gate_to_gui_gate_map[it.value()]);
}

void netlist_layouter::apply_gate_layout()
{
    int x_ko_offset = m_max_gate_width + m_channel_width_x;
    int y_ko_offset = m_max_gate_height + m_channel_width_y;

    int x = 0;
    for (QVector<graphics_gate*>& vector : m_layout)
    {
        int y = 0;
        for (graphics_gate*& g : vector)
        {
            g->moveBy(x * x_ko_offset, y * y_ko_offset);
            m_scene->addItem(g);
            y++;
        }
        x++;
    }
}

void netlist_layouter::squarify_entire_design()
{
    //TODO: maybe paramize the x- and y-lengths
    int square_number_of_all_gates = sqrt(g_netlist->get_gates().size()) + 1;
    int current_x_to_move          = 0;

    QVector<QVector<graphics_gate*>> new_layout;
    new_layout.resize(square_number_of_all_gates);

    for (int x = 0; x < m_layout.size(); x++)
    {
        for (int y = 0; y < m_layout[x].size(); y++)
        {
            new_layout[current_x_to_move].push_back(m_layout[x][y]);
            m_gate_to_level[m_layout[x][y]->get_gate()] = current_x_to_move;
            if (new_layout[current_x_to_move].size() == square_number_of_all_gates)
                current_x_to_move++;
        }
    }

    //there is no shrink_to_fit function, so do this manually
    while (new_layout[new_layout.size() - 1].size() == 0)
        new_layout.pop_back();

    m_layout = new_layout;
}

void netlist_layouter::compress_gate_levels()
{
    int current_level = 0;
    while (current_level < m_layout.size())
    {
        while (m_layout[current_level].size() - m_compress_gate_levels_value > 0)
        {
            QVector<graphics_gate*> temp = m_layout[current_level].mid(m_layout[current_level].size() - m_compress_gate_levels_value);
            m_layout[current_level].resize(m_layout[current_level].size() - m_compress_gate_levels_value);
            m_layout.insert(current_level, temp);
            current_level++;
        }
        current_level++;
    }
}

void netlist_layouter::compute_cw_and_scc()
{
    int input_i  = 0;
    int output_i = 0;
    int output_m = 0;

    //a minimum distance is needed,so it starts with 30 instead of 0
    //[TODO: think of the parameters for this function]
    int max_pins = 30;

    for (int i = 1; i < m_layout.size(); i++)
    {
        output_m = output_i;
        input_i  = 0;
        output_i = 0;

        for (int j = 0; j < m_layout[i].size(); j++)
        {
            input_i += m_layout[i][j]->get_gate()->get_input_pin_types().size();
            output_i += m_layout[i][j]->get_gate()->get_output_pin_types().size();
        }

        if (output_m + input_i > max_pins)
            max_pins = output_m + input_i;
    }

    //if the netlist is too big much of the space is wasted, so take a little less
    if (g_netlist->get_gates().size() > 700)
    {
        max_pins            = 0.1 * max_pins;
        m_sub_channel_count = max_pins;
        m_channel_width_x   = max_pins * m_sub_channel_width;
    }

    //for starters, take 25% of the calculated width, may be reduced later(Depending on the size of the graph, the bigger the less space is needed)
    max_pins            = 0.25 * max_pins;
    m_sub_channel_count = max_pins;
    m_channel_width_x   = (max_pins + 2) * m_sub_channel_width;
    m_channel_width_y   = m_channel_width_x * m_x_y_channel_width_ratio;
}

void netlist_layouter::draw_nets()
{
    std::set<std::shared_ptr<gate>> vcc_gates = g_netlist->get_global_vcc_gates();
    std::set<std::shared_ptr<gate>> gnd_gates = g_netlist->get_global_gnd_gates();

    for (int x = 0; x < m_layout.size(); x++)
    {
        for (int y = 0; y < m_layout[x].size(); y++)
        {
            graphics_gate* gui_src_gate = m_layout[x][y];
            int output_pin_count        = gui_src_gate->get_gate()->get_output_pin_types().size();
            for (int i = 0; i < output_pin_count; i++)
            {
                std::shared_ptr<net> current_net = gui_src_gate->get_gate()->get_fan_out_net(gui_src_gate->get_output_pin_type_at_position(i));
                if (current_net)    //evaluate the type of net
                {
                    if (current_net->get_dsts().size() == 0)
                        draw_output_net(current_net);
                    else if (vcc_gates.find(gui_src_gate->get_gate()) != vcc_gates.end())
                        draw_separated_net(current_net, "1");
                    else if (gnd_gates.find(gui_src_gate->get_gate()) != gnd_gates.end())
                        draw_separated_net(current_net, "0");
                    else
                        draw_standard_net(current_net);
                }
            }
        }
    }
    //Draw all global input nets (cant be drawn with the method above)
    for (std::shared_ptr<net> n : g_netlist->get_global_input_nets())
        draw_input_net(n);
}

void netlist_layouter::draw_input_net(std::shared_ptr<net> n)
{
    global_graphics_net* gui_net = new global_graphics_net(n);

    for (endpoint p : n->get_dsts())
        gui_net->add_input(m_gate_to_gui_gate_map[p.gate]->get_input_pin_scene_position(QString::fromStdString(p.pin_type)));

    gui_net->finalize();
    m_scene->addItem(gui_net);
}

void netlist_layouter::draw_output_net(std::shared_ptr<net> n)
{
    global_graphics_net* gui_net = new global_graphics_net(n);
    endpoint src_point           = n->get_src();
    graphics_gate* src_gate      = m_gate_to_gui_gate_map[src_point.gate];
    gui_net->setPos(src_gate->get_output_pin_scene_position(QString::fromStdString(src_point.pin_type)));
    gui_net->add_output();
    gui_net->finalize();
    m_scene->addItem(gui_net);
}

void netlist_layouter::draw_standard_net(std::shared_ptr<net> n)
{
    standard_graphics_net* gui_net = new standard_graphics_net(n);
    graphics_gate* gui_src_gate    = m_gate_to_gui_gate_map[n->get_src().gate];

    QPointF src_pin_position = gui_src_gate->get_output_pin_scene_position(QString::fromStdString(n->get_src().pin_type));
    gui_net->setPos(src_pin_position);

    for (endpoint p : n->get_dsts())
    {
        graphics_gate* gui_dst_gate = m_gate_to_gui_gate_map[p.gate];
        QPointF dst_pin_position    = gui_dst_gate->get_input_pin_scene_position(QString::fromStdString(p.pin_type));

        if (dst_pin_position.x() > src_pin_position.x())    //standard from left to right
        {
            //distace from the pin to the start of the chanel
            qreal right = gui_src_gate->pos().x() + m_max_gate_width - src_pin_position.x();

            if (dst_pin_position.x() - src_pin_position.x() > m_max_gate_width + m_channel_width_x)    //check for dodging
            {
                qreal down = gui_src_gate->pos().y() + m_max_gate_height - src_pin_position.y();

                gui_net->line_to(QPointF(src_pin_position.x() + right + m_current_sub_channel * m_sub_channel_width, src_pin_position.y()));
                gui_net->line_to(QPointF(gui_net->current_scene_position().x(),
                                         gui_net->current_scene_position().y() + down
                                             + (m_current_sub_channel * m_sub_channel_width
                                                * m_x_y_channel_width_ratio)));    //instead of compressing, use half the sub_channel_count, but with the same distance between the subchannel
                gui_net->line_to(QPointF(dst_pin_position.x() - ((m_current_sub_channel + 1) * m_sub_channel_width),
                                         gui_net->current_scene_position().y()));    //+1 because the most right channel is for the "right-to-left" nets
                gui_net->line_to(QPointF(gui_net->current_scene_position().x(), dst_pin_position.y()));
                gui_net->line_to(QPointF(dst_pin_position.x(), dst_pin_position.y()));
            }
            else
            {
                gui_net->line_to(QPointF(src_pin_position.x() + right + m_current_sub_channel * m_sub_channel_width, src_pin_position.y()));
                gui_net->line_to(QPointF(gui_net->current_scene_position().x(), dst_pin_position.y()));
                gui_net->line_to(QPointF(dst_pin_position.x(), dst_pin_position.y()));
            }
        }
        else    //right to left
        {
            qreal down = gui_src_gate->pos().y() + m_max_gate_height - src_pin_position.y();

            gui_net->line_to(QPointF(src_pin_position.x() + m_sub_channel_width, src_pin_position.y()));
            gui_net->line_to(QPointF(gui_net->current_scene_position().x(), src_pin_position.y() + down + (m_current_sub_channel * m_sub_channel_width * m_x_y_channel_width_ratio)));
            gui_net->line_to(QPointF(dst_pin_position.x() - (m_sub_channel_width * 2), gui_net->current_scene_position().y()));
            gui_net->line_to(QPointF(gui_net->current_scene_position().x(), dst_pin_position.y()));
            gui_net->line_to(QPointF(dst_pin_position.x(), dst_pin_position.y()));
        }
        gui_net->move_pen_to(src_pin_position);
    }

    gui_net->finalize();
    m_scene->addItem(gui_net);

    if (m_current_sub_channel == m_sub_channel_count)
        m_current_sub_channel = 0;
    m_current_sub_channel++;
}

void netlist_layouter::draw_separated_net(std::shared_ptr<net> n, QString text)
{
    separated_graphics_net* separated_net = new separated_graphics_net(text, n);
    QPointF src_pin_position              = m_gate_to_gui_gate_map[n->get_src().gate]->get_output_pin_scene_position(QString::fromStdString(n->get_src().pin_type));
    separated_net->setPos(src_pin_position);
    separated_net->add_output();
    for (endpoint p : n->get_dsts())
    {
        graphics_gate* gui_dst_gate = m_gate_to_gui_gate_map[p.gate];
        QPointF dst_pin_position    = gui_dst_gate->get_input_pin_scene_position(QString::fromStdString(p.pin_type));
        separated_net->add_input(dst_pin_position);
    }
    separated_net->finalize();
    m_scene->addItem(separated_net);
}

void netlist_layouter::init_settings()
{
    m_min_gate_height = std::numeric_limits<qreal>::max();
    m_min_gate_width  = std::numeric_limits<qreal>::max();

    m_max_gate_height = 0;
    m_max_gate_width  = 0;

    m_squarify_entire_design     = false;
    m_compress_gate_levels       = false;
    m_compute_cw_scc             = true;
    m_compress_gate_levels_value = 50;
    m_sub_channel_count          = 3;    //this value needs to be at least 2 (better 3 or more)!!!
    m_sub_channel_width          = 2;    //this cant be computed, is a fixed value (also setable throug the settings?)
    m_channel_width_x            = (m_sub_channel_count + 2) * m_sub_channel_width;
    m_x_y_channel_width_ratio    = 0.5;    //for now half, can be set through the settings
    m_channel_width_y            = m_channel_width_x * m_x_y_channel_width_ratio;

    m_current_sub_channel = 1;
    //TODO: Other settings: if you want the default computation, set the variables for
    //compute_cw_and_scc
}
