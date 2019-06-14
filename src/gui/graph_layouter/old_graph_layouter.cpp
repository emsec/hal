#include "graph_layouter/old_graph_layouter.h"
#include "graph_layouter/gui_graph_gate.h"
#include "graph_layouter/gui_graph_net.h"
#include "gui_globals.h"

#include "core/log.h"
#include "omp.h"

#include <QDebug>
#include <QPointF>
#include <QTime>
#include <cmath>
#include <math.h>
#include <queue>

old_graph_layouter::~old_graph_layouter()
{
    gui_graph_gate::reset_height_and_width();

    for (std::map<int, gui_graph_gate*>::iterator it = id_gui_gate_lookup.begin(); it != id_gui_gate_lookup.end(); it++)
        delete it->second;
    for (std::map<int, gui_graph_net*>::iterator it = id_gui_graph_net_lookup.begin(); it != id_gui_graph_net_lookup.end(); it++)
        delete it->second;
}

old_graph_layouter::old_graph_layouter(QGraphicsScene* scene, std::shared_ptr<netlist> g)
{
    m_scene = scene;
    m_graph = g;

    log_debug("gui", "Creating gates...");
    //qDebug() << "Creating gates...";
    QTime timer;
    timer.start();
    global_timer.start();

    //####Single Thread
    //    std::set<gate*> tem = m_graph->get_gates();
    //    for (auto v : m_graph->get_gates())
    //    {
    //        gui_graph_gate* gui_gate = new gui_graph_gate(v);
    //        id_gui_gate_lookup.insert(std::make_pair(v->get_id(), gui_gate));
    //    }

    generate_gui_gates();

    connect(&g_graph_relay, &graph_relay::gate_event, this, &old_graph_layouter::handle_gate_event);

    connect(&g_graph_relay, &graph_relay::net_event, this, &old_graph_layouter::handle_net_event);

    connect(&g_graph_relay, &graph_relay::module_event, this, &old_graph_layouter::handle_module_event);
}

void old_graph_layouter::apply_gate_level_placement()
{
    log_debug("gui", "Beginning the Alignment...");
    //qDebug() << "Beginning Alignment...";
    QTime timer;
    timer.start();

    channel_width       = compute_channel_width(sub_channel_width);              //way too big....
    int max_rect_width  = gui_graph_gate::get_max_width() + channel_width;       //name to change
    int max_rect_height = gui_graph_gate::get_max_height() + channel_width_y;    //name to change

    log_debug("gui", "Channel-Width: {}", channel_width);

    for (unsigned int i = 0; i < gate_layout.size(); i++)
    {
        for (unsigned int j = 0; j < gate_layout[i].size(); j++)
        {
            m_scene->addItem(gate_layout[i][j]);
            gate_layout[i][j]->moveBy(i * max_rect_width, j * max_rect_height);
        }
    }

    log_debug("gui", "Finished aligning in: {} ms", timer.elapsed());
    //qDebug() << "Finished Alignment";
}

void old_graph_layouter::layout_graph()
{
    //First step, compute all the level of the gate
    compute_level_of_gates();

    //Second step, check if its necessary to "squarify" the levels, in case its something like 3x2000
    squarify();

    //Third step, put the gates at the computed places on the scene
    apply_gate_level_placement();

    //Fourth step, compute and draw the nets
    draw_nets_with_net_algorithm();

    //Fith step, expand the rect
    expand_scene_rect();

    //####Coloring
    //    for(const auto& it : g_netlist->get_module_manager()->get_modules())
    //        handle_module_update(it.second);

    for (const auto& sm : g_netlist->get_modules())
    {
        for (const auto& gate : sm->get_gates())
        {
            handle_module_event(module_event_handler::event::gate_assigned, sm, gate->get_id());
        }
        for (const auto& net : sm->get_nets())
        {
            handle_module_event(module_event_handler::event::net_assigned, sm, net->get_id());
        }
    }

    Q_EMIT finished_layouting();

    log_debug("gui", "Graph layouted in: {} seconds", global_timer.elapsed() / 1000.0);
    //qDebug() << "Graph layouted in: " << global_timer.elapsed() / 1000.0 << "seconds";
}

void old_graph_layouter::relayout_graph()
{
    reset_state();
    generate_gui_gates();
    layout_graph();
}

void old_graph_layouter::compute_level_of_gates()
{
    /*some vectors / sets that may be needed*/
    /*a set for the already captured? gates*/
    std::set<std::shared_ptr<gate>> visited;
    std::set<gui_graph_gate*> left_after_algorithm;

    log_debug("gui", "Starting gate-Level Algorithm...");
    //qDebug() << "Beginnign gate level algorithm...";;
    QTime timer;
    timer.start();

    for (std::map<int, gui_graph_gate*>::iterator it = id_gui_gate_lookup.begin(); it != id_gui_gate_lookup.end(); it++)
    {
        left_after_algorithm.insert(it->second);
    }
    /*all the gates hat needs to be look into will be added here*/
    std::queue<gui_graph_gate*> in_progress;

    /*vdd, vcc and gnd maybe level -2*/
    /*search for the global input-gates, make them level 0 and fill the queue with initial-values*/
    for (const auto& a : m_graph->get_global_input_nets())
    {
        //shouldnt have sources, just test for dest
        for (const auto& tup : a->get_dsts())
        {
            //test if it is in visited
            auto dst = tup.gate;
            if (visited.find(dst) == visited.end())
            {
                gui_graph_gate* temp_gui_gate = get_gui_gate_from_gate(dst);
                visited.insert(dst);
                in_progress.push(temp_gui_gate);
                left_after_algorithm.erase(temp_gui_gate);
                id_level_lookup.insert(std::make_pair(dst->get_id(), 0));
            }
        }
    }

    /*start with DFS-like-algoritm*/
    while (!in_progress.empty())
    {
        gui_graph_gate* current_gui_gate = in_progress.front();
        int current_gui_gate_level       = get_gate_level(current_gui_gate);

        for (const auto& tup : current_gui_gate->get_ref_gate()->get_successors())
        {
            auto successor = tup.gate;
            if (!(visited.find(successor) != visited.end()))    //test if the successor isnt visited yet, if it is, just do nothing
            {
                visited.insert(successor);
                gui_graph_gate* temp_gui_gate = get_gui_gate_from_gate(successor);
                in_progress.push(temp_gui_gate);
                id_level_lookup.insert(std::make_pair(temp_gui_gate->get_ref_gate()->get_id(), current_gui_gate_level + 1));

                if (current_gui_gate_level + 1 > highest_level)
                    highest_level = current_gui_gate_level + 1;

                left_after_algorithm.erase(temp_gui_gate);
            }
        }

        /*after all is done, pop the current-gate out of in_progress to process the next one*/
        in_progress.pop();
    }

    //go through the left-set for the gates that are not part of the input-graph (such as the gnd/vcc gates), needs to do with a recursive algorithm
    for (gui_graph_gate* n : left_after_algorithm)
    {
        //test if they have predecessors/sucessors that have a level, otherwise give them level zero (take the lowest lvl -1 at successors for the current level)
        int lowest_level_of_successor        = highest_level;
        bool gate_has_a_successor_with_level = false;

        /*search the one with the lowest level*/
        for (const auto& tup : n->get_ref_gate()->get_successors())
        {
            //if it is in the set
            if (visited.find(tup.gate) != visited.end())
            {
                int level = get_gate_level(tup.gate->get_id());
                if (level < lowest_level_of_successor)
                    lowest_level_of_successor = level;
                gate_has_a_successor_with_level = true;
            }
        }
        //give a default-lvl (0), because predeccessors (with a level) are impossible, otherwise repeat necessary steps from above
        if (!gate_has_a_successor_with_level)
        {
            visited.insert(n->get_ref_gate());
            id_level_lookup.insert(std::make_pair(n->get_ref_gate()->get_id(), 0));
        }
        else
        {
            visited.insert(n->get_ref_gate());
            if (lowest_level_of_successor <= 0)
            {
                //log_critical("gui", "lowest_level_of_successor is <= 0.");
                //exit(0);
                id_level_lookup[n->get_ref_gate()->get_id()] = 0;
            }
            else
                id_level_lookup[n->get_ref_gate()->get_id()] = lowest_level_of_successor - 1;
        }
    }
    left_after_algorithm.clear();

    //fill the gates_per_level_array
    gates_per_level.resize(highest_level + 1);
    for (std::map<int, gui_graph_gate*>::iterator it = id_gui_gate_lookup.begin(); it != id_gui_gate_lookup.end(); it++)
    {
        gui_graph_gate* current_gui_gate = it->second;
        int current_gate_level           = get_gate_level(current_gui_gate);
        gates_per_level[current_gate_level]++;
    }

    //#####-Fill the layout-vector-######
    gate_layout.resize(highest_level + 1);
    for (std::map<int, gui_graph_gate*>::iterator it = id_gui_gate_lookup.begin(); it != id_gui_gate_lookup.end(); it++)
    {
        gui_graph_gate* current_gui_gate = it->second;
        int current_gate_level           = get_gate_level(current_gui_gate);
        gate_layout[current_gate_level].push_back(current_gui_gate);
    }

    log_debug("gui", "Computed gate-levels in: {} ms", timer.elapsed());
    //qDebug() << "Finsihed computing gate-levels";
}

void old_graph_layouter::squarify()
{
    //qDebug() << "Beginning squarifying...";

    //number of all gates
    int number_of_gates                     = g_netlist->get_gates().size();
    unsigned int square_number_of_all_gates = sqrt(number_of_gates) + 1;

    int current_x_to_move = 0;

    std::vector<std::vector<gui_graph_gate*>> new_gate_layout;
    new_gate_layout.resize(square_number_of_all_gates);

    //go through the original layout
    for (unsigned int x = 0; x < gate_layout.size(); x++)
    {
        for (unsigned int y = 0; y < gate_layout[x].size(); y++)
        {
            gui_graph_gate* gate_to_move = gate_layout[x][y];
            new_gate_layout[current_x_to_move].push_back(gate_to_move);
            if (new_gate_layout[current_x_to_move].size() == square_number_of_all_gates)
                current_x_to_move++;
        }
    }

    //da shrink_to_fit nicht funzt: Lösche alle letzten Level, die gleich 0 sind
    while (new_gate_layout[new_gate_layout.size() - 1].size() == 0)
        new_gate_layout.pop_back();

    gate_layout = new_gate_layout;

    //the level_lookup map needs to be updated because it changed...
    id_level_lookup.erase(id_level_lookup.begin(), id_level_lookup.end());
    for (unsigned int i = 0; i < gate_layout.size(); i++)
    {
        for (unsigned int j = 0; j < gate_layout[i].size(); j++)
        {
            gui_graph_gate* current                            = gate_layout[i][j];
            id_level_lookup[current->get_ref_gate()->get_id()] = i;
        }
    }

    //qDebug() << "Finished squarifying";
}

void old_graph_layouter::generate_gui_gates()
{
    int number_of_gates = m_graph->get_gates().size();
    //this vector is for necessary for accessing the gates in the parallel for-loop (difficult in a set, array is also possible instead of a vector)

    std::vector<gui_graph_gate*> gui_gate_array(number_of_gates, nullptr);

    std::set<u32> ordered_gate_ids;
    for (const auto& gate : m_graph->get_gates())
        ordered_gate_ids.insert(gate->get_id());

    std::vector<std::shared_ptr<gate>> gate_array(number_of_gates, nullptr);
    int cnt = 0;
    for (const auto& id : ordered_gate_ids)
        gate_array[cnt++] = m_graph->get_gate_by_id(id);

#pragma omp parallel for
    for (int i = 0; i < number_of_gates; i++)
    {
        gui_gate_array[i] = new gui_graph_gate(gate_array[i]);
    }

    id_gui_gate_lookup.clear();
    for (int i = 0; i < number_of_gates; i++)
        id_gui_gate_lookup[gui_gate_array[i]->get_ref_gate()->get_id()] = gui_gate_array[i];
}

int old_graph_layouter::get_gate_level(gui_graph_gate* g)
{
    if (g == nullptr)
    {
        log_critical("gui", "parameter 'g' is nullptr.");
        exit(0);
    }
    return this->get_gate_level(g->get_ref_gate()->get_id());
}

int old_graph_layouter::get_gate_level(int id)
{
    std::map<int, int>::iterator found = id_level_lookup.find(id);
    if (found != id_level_lookup.end())
        return found->second;

    log_critical("gui", "invalid gate level for gate id {}", id);
    exit(0);
    return -1;
}

//way too big, (maybe half pins, round up?)
int old_graph_layouter::compute_channel_width(int line_width)
{
    //Saves the maximum #input/output pins per level
    int input_i  = 0;
    int output_i = 0;
    int output_m = 0;

    //a minimum distance is needed,so it starts with 30 instead of 0
    int max_pins = 30;

    for (unsigned int i = 0; i < gate_layout.size(); i++)
    {
        output_m = output_i;
        input_i  = 0;
        output_i = 0;

        for (unsigned int j = 0; j < gate_layout[i].size(); j++)
        {
            input_i += gate_layout[i][j]->get_ref_gate()->get_input_pin_types().size();
            output_i += gate_layout[i][j]->get_ref_gate()->get_output_pin_types().size();
        }

        //First level cant be used to calculate the width
        if (i == 0)
            continue;

        //Important part here
        if (output_m + input_i > max_pins)
            max_pins = output_m + input_i;
    }

    //if the netlist is too big much of the space is wasted, so take a little less
    if (m_graph->get_gates().size() > 700)
    {
        max_pins = 0.1 * max_pins;
        set_sub_channel_count(max_pins);
        channel_width_y = (max_pins * line_width) / 4;    //dont forget the 4 in the gui net class
        return max_pins * line_width;
    }

    //for starters, take 25% of the calculated width, may be reduced later(Depending on the size of the graph, the bigger the less space is needed)
    max_pins = 0.25 * max_pins;
    set_sub_channel_count(max_pins);
    channel_width_y = (max_pins * line_width) / 4;    //dont forget the 4 in the gui net class
    return max_pins * line_width;
}

void old_graph_layouter::expand_scene_rect()
{
    QRectF bounding_rect = m_scene->itemsBoundingRect();
    m_scene->setSceneRect(bounding_rect.x() - 200, bounding_rect.y()-200, bounding_rect.width() + 400, bounding_rect.height() + 400);
}

void old_graph_layouter::reset_state()
{
    gui_graph_gate::reset_height_and_width();

    for (std::map<int, gui_graph_gate*>::iterator it = id_gui_gate_lookup.begin(); it != id_gui_gate_lookup.end(); it++)
        delete it->second;
    for (std::map<int, gui_graph_net*>::iterator it = id_gui_graph_net_lookup.begin(); it != id_gui_graph_net_lookup.end(); it++)
        delete it->second;

    id_gui_gate_lookup.clear();
    id_gui_graph_net_lookup.clear();

    gate_layout.clear();
    id_level_lookup.clear();

    gates_per_level.clear();

    m_scene->clear();
    m_scene->update();

    highest_level = 0;
}

gui_graph_gate* old_graph_layouter::get_gui_gate_from_gate(std::shared_ptr<gate> gate)
{
    std::map<int, gui_graph_gate*>::iterator found = id_gui_gate_lookup.find(gate->get_id());
    if (found != id_gui_gate_lookup.end())
        return found->second;
    else
        return nullptr;
}

gui_graph_gate* old_graph_layouter::get_gui_gate_from_id(int id)
{
    std::map<int, gui_graph_gate*>::iterator found = id_gui_gate_lookup.find(id);
    if (found != id_gui_gate_lookup.end())
        return found->second;
    else
        return nullptr;
}

gui_graph_net* old_graph_layouter::get_gui_graph_net_from_id(int id)
{
    std::map<int, gui_graph_net*>::iterator found = id_gui_graph_net_lookup.find(id);
    if (found != id_gui_graph_net_lookup.end())
        return found->second;
    else
        return nullptr;
}

int old_graph_layouter::get_channal_width()
{
    return channel_width;
}

int old_graph_layouter::get_net_sub_channels_count()
{
    return net_sub_channel_count;
}

int old_graph_layouter::get_net_sub_channel_width()
{
    return sub_channel_width;
}

void old_graph_layouter::set_channel_width(int new_channel_width)
{
    channel_width = new_channel_width;
}

void old_graph_layouter::set_sub_channel_count(int new_count)
{
    net_sub_channel_count = new_count;
}

void old_graph_layouter::set_sub_channel_width(int new_width)
{
    sub_channel_width = new_width;
}

void old_graph_layouter::handle_gate_event(gate_event_handler::event ev, std::shared_ptr<gate> gate, u32 associated_data)
{
    Q_UNUSED(associated_data)
    if (ev == gate_event_handler::event::name_changed)
    {
        id_gui_gate_lookup[gate->get_id()]->update_name();
        m_scene->update();
    }
}

void old_graph_layouter::handle_net_event(net_event_handler::event ev, std::shared_ptr<net> net, u32 associated_data)
{
    Q_UNUSED(ev)
    Q_UNUSED(net)
    Q_UNUSED(associated_data)
}

void old_graph_layouter::handle_module_event(module_event_handler::event ev, std::shared_ptr<module> module, u32 associated_data)
{
    // UGLY: top module does not get a special color
    // should be: current top module, not global top module
    if (module == g_netlist->get_top_module()) return;

    if (ev == module_event_handler::event::gate_removed)
    {
        id_gui_gate_lookup[associated_data]->remove_module_color(module->get_id());
        m_scene->update();
    }
    else if (ev == module_event_handler::event::gate_assigned)
    {
        id_gui_gate_lookup[associated_data]->add_new_module_color(QColor::fromHsv((module->get_id() * 25) % 360, 255, 255), module->get_id());
        m_scene->update();
    }
    else if (ev == module_event_handler::event::net_assigned)
    {
        id_gui_graph_net_lookup[associated_data]->add_new_module_color(QColor::fromHsv((module->get_id() * 25) % 360, 255, 255), module->get_id());
        m_scene->update();
    }
    else if (ev == module_event_handler::event::net_removed)
    {
        id_gui_graph_net_lookup[associated_data]->remove_module_color(module->get_id());
        m_scene->update();
    }
}

void old_graph_layouter::draw_nets_with_net_algorithm()
{
    //qDebug() << "Computing nets...";
    //##########Version 2####################### [Calculate the nets column by columm, so you know the channel each net has]
    auto left = m_graph->get_nets();    //you miss some nets if you just iterate through the input-gate pin nets (like the global input nets), so sleave them in here

    int sub_channel = 1;    //the 0 sub_channel is for global out/vcc nets
    std::vector<gui_graph_net*> gui_graph_nets;
    for (unsigned int i = 0; i < gate_layout.size(); i++)
    {
        for (unsigned int j = 0; j < gate_layout[i].size(); j++)
        {
            gui_graph_gate* gui_gate = gate_layout[i][j];
            //Go through all the output pins of the current gate, calculate the net and remove it from the set, then go through the net-set
            unsigned int output_pin_count = gui_gate->output_pin_count();
            if (output_pin_count == 0)
                continue;

            //main loop for calculating the nets (go through all the pins for each gate get the net)
            for (unsigned int current_pin = 0; current_pin < output_pin_count; current_pin++)
            {
                auto current_gate = gui_gate->get_ref_gate();
                auto current_net  = current_gate->get_fan_out_net(gui_gate->get_output_pin_type_at_position(current_pin));
                if (!current_net)
                    continue;
                gui_graph_net* l_net = new gui_graph_net(current_net, m_scene, this);
                left.erase(current_net);
                gui_graph_nets.push_back(l_net);
                m_scene->addItem(l_net);
                l_net->calculate_net(sub_channel);
                m_scene->removeItem(l_net);
                sub_channel++;
                //just to be sure:
                if (sub_channel == net_sub_channel_count)
                    sub_channel = 1;
            }
        }
        sub_channel = 1;
    }
    //draw all the leftover nets, such as global input nets
    for (const auto& net : left)
    {
        gui_graph_net* gui_net = new gui_graph_net(net, m_scene, this);
        m_scene->addItem(gui_net);
        gui_net->calculate_net(sub_channel);
        gui_graph_nets.push_back(gui_net);
    }
    //adds all items to the scene and inserts them in the map so you can look them up later
    for (const auto& gui_net : gui_graph_nets)
    {
        m_scene->addItem(gui_net);
        id_gui_graph_net_lookup.insert(std::make_pair(gui_net->get_ref_net()->get_id(), gui_net));
    }
    //###########End of Version 2#################
    //qDebug() << "Finished computing nets";
}

/*draws simple lines from the source to the destinations)*/
void old_graph_layouter::draw_nets_as_straight_lines()
{
    QPen pen(Qt::white);
    pen.setWidth(30);

    for (const auto& net : m_graph->get_nets())
    {
        /*get the dst gate and the dst set*/
        auto gate_src = net->get_src().gate;
        auto dst_set  = net->get_dsts();
        /*test if the it is a input-net, and if it is, just ignore it*/
        if (gate_src == nullptr)
            continue;
        if (dst_set.size() < 1)
            continue;

        //here starts the important part
        gui_graph_gate* src_gui_gate = get_gui_gate_from_gate(gate_src);
        QPointF offset_pin_cood      = src_gui_gate->get_unmapped_output_pin_coordinates(QString::fromStdString(net->get_src().pin_type));
        QPointF src_pin_cood         = src_gui_gate->scenePos() + offset_pin_cood;    //important

        /*go through all the destinations*/
        for (auto tup : dst_set)
        {
            gui_graph_gate* gui_dst_gate = get_gui_gate_from_gate(tup.gate);
            QPointF dst_pin_offset       = gui_dst_gate->get_unmapped_input_pin_coordinates(QString::fromStdString(tup.pin_type));
            QPointF dst_pin_cood         = gui_dst_gate->scenePos() + dst_pin_offset;
            //log_debug("gui", "DESTINATION: " << dst_pin_cood;
            m_scene->addLine(QLineF(src_pin_cood, dst_pin_cood), pen);
        }
    }
}
