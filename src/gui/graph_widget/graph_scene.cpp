#include "graph_widget/graph_scene.h"
#include "graph_widget/graph_scene_manager.h"
#include "graph_widget/graphics_items/graph_graphics_item.h"
#include "graph_widget/graphics_items/graphics_gate.h"
#include "graph_widget/graphics_items/graphics_net.h"
#include "netlist/gate.h"
#include "netlist/net.h"

graph_scene::graph_scene(graph_scene_manager* manager, QObject* parent) : QGraphicsScene(parent)
{
    m_manager = manager;

    connect(this, &graph_scene::selectionChanged, this, &graph_scene::handle_selection_changed);
}

void graph_scene::addItem(graph_graphics_item* item)
{
    QGraphicsScene::addItem(item);

    switch (item->get_class())
    {
        case item_class::gate:
        {
            graphics_gate* g = static_cast<graphics_gate*>(item);
            m_manager->get_gate_map().insert(g->get_gate()->get_id(), g);
            break;
        }
        case item_class::net:
        {
            graphics_net* n = static_cast<graphics_net*>(item);
            m_manager->get_net_map().insert(n->get_net()->get_id(), n);
            break;
        }
    }
}

void graph_scene::handle_selection_changed()
{
    //    QList<QGraphicsItem*> items = selectedItems();
    //    selected_nets.clear();
    //    selected_gates.clear();

    //    //fill the currently selected items in the 2 lists so that the handling of the items becomes easier
    //    for (int i = 0; i < items.size(); i++)
    //    {
    //        gui_graph_gate* gui_gate = dynamic_cast<gui_graph_gate*>(items.at(i));
    //        if (!gui_gate)
    //        {
    //            gui_graph_net* gui_net = dynamic_cast<gui_graph_net*>(items.at(i));
    //            if (!gui_net)    //it isnt a gate and it isnt a net so ignore that item
    //                continue;

    //            selected_nets.append(gui_net);
    //            continue;
    //        }
    //        selected_gates.append(gui_gate);
    //    }

    //    //because cant handle multiple selection just yet
    //    if (items.size() > 1)
    //        return;

    //    if (items.isEmpty())
    //    {
    //        if (state != no_gate_selected)    //maybe pointless, can be removed(i think)
    //        {
    //            state = no_gate_selected;
    //            if (currently_selected_gate)
    //                currently_selected_gate->enable_input_pin_navigation(false);
    //        }
    //        currently_selected_gate = nullptr;
    //        currently_selected_net  = nullptr;
    //        scene()->update();
    //        return;
    //    }

    //    gui_graph_gate* selected_gui_gate = dynamic_cast<gui_graph_gate*>(items.at(0));
    //    if (!selected_gui_gate)    //its a net, need to set the currently selected net
    //    {
    //        currently_selected_net = dynamic_cast<gui_graph_net*>(items.at(0));
    //        //THE RELAY
    //        QList<u32> gate_ids;
    //        QList<u32> net_ids;
    //        QList<u32> module_ids;
    //        net_ids.append(currently_selected_net->get_ref_net()->get_id());
    //        g_selection_relay.relay_combined_selection(this, gate_ids, net_ids, module_ids);
    //        g_selection_relay.relay_current_net(this, currently_selected_net->get_ref_net()->get_id());
    //        state = net_selected;
    //        return;
    //    }
    //    //past here, the newly selected item is just a gate
    //    //if(state == no_gate_selected), but it seems the state is irrelevant
    //    //if there was a currently selected one, disable it
    //    if (currently_selected_gate)
    //    {
    //        currently_selected_gate->enable_input_pin_navigation(false);
    //        currently_selected_gate->enable_output_pin_navigation(false);
    //    }
    //    state                   = gate_selected;
    //    currently_selected_gate = selected_gui_gate;

    //    //for the RELAY, it is only needed here, not in the navigation functions
    //    QList<u32> gate_ids;
    //    QList<u32> net_ids;
    //    QList<u32> module_ids;
    //    gate_ids.append(currently_selected_gate->get_ref_gate()->get_id());
    //    g_selection_relay.relay_combined_selection(this, gate_ids, net_ids, module_ids);    //this is just for the tree_navigation
    //    g_selection_relay.relay_current_gate(this, currently_selected_gate->get_ref_gate()->get_id());
}
