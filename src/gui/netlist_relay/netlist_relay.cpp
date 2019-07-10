#include "netlist_relay/netlist_relay.h"

#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/module.h"

#include "gui/module_model/module_item.h"
#include "gui/module_model/module_model.h"

#include "gui/file_manager/file_manager.h" // DEBUG LINE
#include "gui/gui_globals.h" // DEBUG LINE

#include <functional>

#include <QColorDialog> // DEBUG LINE
#include <QInputDialog> // DEBUG LINE
#include <QDebug>

netlist_relay::netlist_relay(QObject* parent) : QObject(parent),
    m_module_model(new module_model(this))
{
    connect(file_manager::get_instance(), &file_manager::file_opened, this, &netlist_relay::debug_handle_file_opened); // DEBUG LINE
    register_callbacks();
}

netlist_relay::~netlist_relay()
{
    netlist_event_handler::unregister_callback("relay");
    net_event_handler::unregister_callback("relay");
    gate_event_handler::unregister_callback("relay");
    module_event_handler::unregister_callback("relay");
}

void netlist_relay::register_callbacks()
{
    netlist_event_handler::register_callback(
                "relay",
                std::function<void(netlist_event_handler::event, std::shared_ptr<netlist>, u32)>
                (std::bind(&netlist_relay::relay_netlist_event, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

    net_event_handler::register_callback(
                "relay",
                std::function<void(net_event_handler::event, std::shared_ptr<net>, u32)>
                (std::bind(&netlist_relay::relay_net_event, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

    gate_event_handler::register_callback(
                "relay",
                std::function<void(gate_event_handler::event, std::shared_ptr<gate>, u32)>
                (std::bind(&netlist_relay::relay_gate_event, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

    module_event_handler::register_callback(
                "relay",
                std::function<void(module_event_handler::event, std::shared_ptr<module>, u32)>
                (std::bind(&netlist_relay::relay_module_event, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

module_item* netlist_relay::get_module_item(const u32 id)
{
    return m_module_items.value(id);
}

module_model* netlist_relay::get_module_model()
{
    return m_module_model;
}

void netlist_relay::debug_change_module_color(module_item* item)
{
    // NOT THREADSAFE

    if (!item)
        return;

    QColor color = QColorDialog::getColor();

    if (!color.isValid())
        return;

    item->set_color(color);
    m_module_model->dataChanged(m_module_model->get_index(item), m_module_model->get_index(item));
}

void netlist_relay::debug_add_selection_to_module(module_item* item)
{
    // NOT THREADSAFE
    // DECIDE HOW TO HANDLE MODULES

    if (!item)
        return;

    std::shared_ptr<module> m = g_netlist->get_module_by_id(item->id());

    if (!m)
        return;

    if (g_selection_relay.m_number_of_selected_gates)
    {
        for (int i = 0; i < g_selection_relay.m_number_of_selected_gates; ++i)
        {
            std::shared_ptr<gate> g = g_netlist->get_gate_by_id(g_selection_relay.m_selected_gates[i]);

            if (g)
                m->assign_gate(g);
        }
    }

//    if (g_selection_relay.m_number_of_selected_nets)
//    {
//        for (int i = 0; i < g_selection_relay.m_number_of_selected_nets; ++i)
//        {
//            std::shared_ptr<net> n = g_netlist->get_net_by_id(g_selection_relay.m_selected_nets[i]);

//            if (n)
//                m->assign_net(n);
//        }
//    }
}

void netlist_relay::debug_add_child_module(module_item* item)
{
    // NOT THREADSAFE

    bool ok;
    QString name = QInputDialog::getText(nullptr, "", "Module Name:", QLineEdit::Normal, "", &ok);

    if (!ok || name.isEmpty())
        return;

    if (!item)
        return;

    std::shared_ptr<module> m = g_netlist->get_module_by_id(item->id());

    if (!m)
        return;

    std::shared_ptr<module> s = g_netlist->create_module(g_netlist->get_unique_module_id(), name.toStdString(), m);
}

void netlist_relay::relay_netlist_event(netlist_event_handler::event ev, std::shared_ptr<netlist> object, u32 associated_data)
{
    if (!object)
        return; // SHOULD NEVER BE REACHED

    //qDebug() << "relay_netlist_event called: event ID =" << ev << "for object at" << object.get();
    //Q_EMIT netlist_event(ev, object, associated_data);

    switch (ev)
    {
    case netlist_event_handler::event::id_changed:
    {
        ///< associated_data = old id
        break;
    }
    case netlist_event_handler::event::input_filename_changed:
    {
        ///< no associated_data
        break;
    }
    case netlist_event_handler::event::design_name_changed:
    {
        ///< no associated_data
        break;
    }
    case netlist_event_handler::event::device_name_changed:
    {
        ///< no associated_data
        break;
    }
    case netlist_event_handler::event::marked_global_vcc:
    {
        ///< associated_data = id of gate
        break;
    }
    case netlist_event_handler::event::marked_global_gnd:
    {
        ///< associated_data = id of gate
        break;
    }
    case netlist_event_handler::event::unmarked_global_vcc:
    {
        ///< associated_data = id of gate
        break;
    }
    case netlist_event_handler::event::unmarked_global_gnd:
    {
        ///< associated_data = id of gate
        break;
    }
    case netlist_event_handler::event::marked_global_input:
    {
        ///< associated_data = id of net
        break;
    }
    case netlist_event_handler::event::marked_global_output:
    {
        ///< associated_data = id of net
        break;
    }
    case netlist_event_handler::event::marked_global_inout:
    {
        ///< associated_data = id of net
        break;
    }
    case netlist_event_handler::event::unmarked_global_input:
    {
        ///< associated_data = id of net
        break;
    }
    case netlist_event_handler::event::unmarked_global_output:
    {
        ///< associated_data = id of net
        break;
    }
    case netlist_event_handler::event::unmarked_global_inout:
    {
        ///< associated_data = id of net
        break;
    }
    }
}

void netlist_relay::relay_module_event(module_event_handler::event ev, std::shared_ptr<module> object, u32 associated_data)
{
    if (!object)
        return; // SHOULD NEVER BE REACHED

    //qDebug() << "relay_module_event called: event ID =" << ev << "for object at" << object.get();
    //Q_EMIT module_event(ev, object, associated_data);

    switch (ev)
    {
    case module_event_handler::event::created:
    {
        //< no associated_data

        module_item* item = new module_item(QString::fromStdString(object->get_name()), object->get_id());
        std::shared_ptr<module> parent_module = object->get_parent_module();
        module_item* parent_item = nullptr;

        if (parent_module)
            parent_item = m_module_items.value(parent_module->get_id());

        m_module_items.insert(object->get_id(), item);
        m_module_model->add_item(item, parent_item);

        g_selection_relay.handle_module_created();

        Q_EMIT module_created(object);
        break;
    }
    case module_event_handler::event::removed:
    {
        //< no associated_data

        g_selection_relay.handle_module_removed(object->get_id());

        Q_EMIT module_removed(object);
        break;
    }
    case module_event_handler::event::name_changed:
    {
        //< no associated_data

        Q_EMIT module_name_changed(object);
        break;
    }
    case module_event_handler::event::parent_changed:
    {
        //< no associated_data

        Q_EMIT module_parent_changed(object);
        break;
    }
    case module_event_handler::event::submodule_added:
    {
        //< associated_data = id of added module

        g_graph_context_manager.handle_module_submodule_added(object, associated_data);

        Q_EMIT module_submodule_added(object, associated_data);
        break;
    }
    case module_event_handler::event::submodule_removed:
    {
        //< associated_data = id of removed module

        g_graph_context_manager.handle_module_submodule_removed(object, associated_data);

        Q_EMIT module_submodule_removed(object, associated_data);
        break;
    }
    case module_event_handler::event::gate_assigned:
    {
        //< associated_data = id of inserted gate

        g_graph_context_manager.handle_module_gate_inserted(object, associated_data);

        Q_EMIT module_gate_assigned(object, associated_data);
        break;
    }
    case module_event_handler::event::gate_removed:
    {
        //< associated_data = id of removed gate

        g_graph_context_manager.handle_module_gate_removed(object, associated_data);

        Q_EMIT module_gate_removed(object, associated_data);
        break;
    }
    }
}

void netlist_relay::relay_gate_event(gate_event_handler::event ev, std::shared_ptr<gate> object, u32 associated_data)
{
    if (!object)
        return; // SHOULD NEVER BE REACHED

    //qDebug() << "relay_gate_event called: event ID =" << ev << "for object at" << object.get();
    //Q_EMIT gate_event(ev, object, associated_data);

    switch (ev)
    {
    case gate_event_handler::event::created:
    {
        //< no associated_data

        g_selection_relay.handle_gate_created();

        break;
    }
    case gate_event_handler::removed:
    {
        //< no associated_data

        g_selection_relay.handle_gate_removed(object->get_id());

        break;
    }
    case gate_event_handler::name_changed:
    {
        //< no associated_data
        break;
    }
    }
}

void netlist_relay::relay_net_event(net_event_handler::event ev, std::shared_ptr<net> object, u32 associated_data)
{
    if (!object)
        return; // SHOULD NEVER BE REACHED

    //qDebug() << "relay_net_event called: event ID =" << ev << "for object at" << object.get();
    //Q_EMIT net_event(ev, object, associated_data);

    switch (ev)
    {
    case net_event_handler::event::created:
    {
        //< no associated_data

        g_selection_relay.handle_net_created();

        break;
    }
    case net_event_handler::event::removed:
    {
        //< no associated_data

        g_selection_relay.handle_net_removed(object->get_id());

        break;
    }
    case net_event_handler::event::name_changed:
    {
        //< no associated_data
        break;
    }
    case net_event_handler::event::src_changed:
    {
        //< no associated_data
        break;
    }
    case net_event_handler::event::dst_added:
    {
        //< associated_data = id of dst gate
        break;
    }
    case net_event_handler::event::dst_removed:
    {
        //< associated_data = id of dst gate
        break;
    }
    }
}

void netlist_relay::debug_handle_file_opened()
{
    std::shared_ptr<module> top_module = g_netlist->get_top_module();
    module_item* item = new module_item(QString::fromStdString(top_module->get_name()), top_module->get_id());

    m_module_items.insert(top_module->get_id(), item);
    m_module_model->add_item(item, nullptr);
}
