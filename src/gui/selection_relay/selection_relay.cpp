#include "selection_relay/selection_relay.h"

#include "core/log.h"

#include "netlist/gate.h"
#include "netlist/module.h"
#include "netlist/net.h"

#include "gui_globals.h"

#include <QDebug>

// SET VIA SETTINGS OR TOOLBUTTON
bool selection_relay::s_navigation_skips_enabled = false;

selection_relay::selection_relay(QObject* parent) : QObject(parent), m_current_type(item_type::none), m_focus_type(item_type::none), m_subfocus(subfocus::none)
{
    clear();
}

void selection_relay::clear()
{
    m_selected_gates.clear();
    m_selected_nets.clear();
    m_selected_modules.clear();
    m_subfocus                   = subfocus::none;
    m_subfocus_index             = 0;
    m_focus_id                   = 0;
}

void selection_relay::clear_and_update()
{
    clear();
    Q_EMIT selection_changed(nullptr);
}

void selection_relay::register_sender(void* sender, QString name)
{
    m_sender_register.append(QPair<void*, QString>(sender, name));
}

void selection_relay::remove_sender(void* sender)
{
    for (QPair<void*, QString> pair : m_sender_register)
    {
        if (pair.first == sender)
            m_sender_register.removeOne(pair);
    }
}

void selection_relay::relay_selection_changed(void* sender)
{
    Q_EMIT selection_changed(sender);
}

void selection_relay::relay_subfocus_changed(void* sender)
{
    Q_EMIT subfocus_changed(sender);
}

void selection_relay::navigate_up()
{
    u32 size = 0;

    switch (m_focus_type)
    {
        case item_type::none:
        {
            return;
        }
        case item_type::gate:
        {
            std::shared_ptr<gate> g = g_netlist->get_gate_by_id(m_focus_id);

            if (!g)
                return;

            if (m_subfocus == subfocus::left)
            {
                size = g->get_input_pins().size();

                if (!size)     // CHECK NECESSARY ???
                    return;    // INVALID STATE, FIX OR IGNORE ???

                break;
            }

            if (m_subfocus == subfocus::right)
            {
                size = g->get_output_pins().size();

                if (!size)     // CHECK NECESSARY ???
                    return;    // INVALID STATE, FIX OR IGNORE ???

                break;
            }

            return;
        }
        case item_type::net:
        {
            std::shared_ptr<net> n = g_netlist->get_net_by_id(m_focus_id);

            if (!n)
                return;

            if (m_subfocus == subfocus::right)
            {
                size = n->get_destinations().size();

                if (!size)     // CHECK NECESSARY ???
                    return;    // INVALID STATE, FIX OR IGNORE ???

                break;
            }

            return;
        }
        case item_type::module:
        {
            return;
        }
    }

    if (m_subfocus_index == 0)
        m_subfocus_index = size - 1;
    else
        --m_subfocus_index;

    Q_EMIT subfocus_changed(nullptr);
}

void selection_relay::navigate_down()
{
    u32 size = 0;

    switch (m_focus_type)
    {
        case item_type::none:
        {
            return;
        }
        case item_type::gate:
        {
            std::shared_ptr<gate> g = g_netlist->get_gate_by_id(m_focus_id);

            if (!g)
                return;

            if (m_subfocus == subfocus::left)
            {
                size = g->get_input_pins().size();

                if (!size)     // CHECK NECESSARY ???
                    return;    // INVALID STATE, FIX OR IGNORE ???

                break;
            }

            if (m_subfocus == subfocus::right)
            {
                size = g->get_output_pins().size();

                if (!size)     // CHECK NECESSARY ???
                    return;    // INVALID STATE, FIX OR IGNORE ???

                break;
            }

            return;
        }
        case item_type::net:
        {
            std::shared_ptr<net> n = g_netlist->get_net_by_id(m_focus_id);

            if (!n)
                return;

            if (m_subfocus == subfocus::right)
            {
                size = n->get_destinations().size();

                if (!size)     // CHECK NECESSARY ???
                    return;    // INVALID STATE, FIX OR IGNORE ???

                break;
            }

            return;
        }
        case item_type::module:
        {
            return;
        }
    }

    if (m_subfocus_index == size - 1)
        m_subfocus_index = 0;
    else
        ++m_subfocus_index;

    Q_EMIT subfocus_changed(nullptr);
}

void selection_relay::navigate_left()
{
    switch (m_focus_type)
    {
        case item_type::none:
        {
            return;
        }
        case item_type::gate:
        {
            std::shared_ptr<gate> g = g_netlist->get_gate_by_id(m_focus_id);

            if (!g)
                return;

            if (g->get_input_pins().size())    // CHECK HERE OR IN PRIVATE METHODS ?
            {
                if (m_subfocus == subfocus::left)
                    follow_gate_input_pin(g, m_subfocus_index);
                else
                {
                    if (s_navigation_skips_enabled && g->get_input_pins().size() == 1)
                        follow_gate_input_pin(g, 0);
                    else
                        subfocus_left();
                }
            }

            return;
        }
        case item_type::net:
        {
            std::shared_ptr<net> n = g_netlist->get_net_by_id(m_focus_id);

            if (!n)
                return;

            if (m_subfocus == subfocus::left)
                follow_net_to_source(n);
            else
            {
                if (s_navigation_skips_enabled && n->get_destinations().size() == 1)
                    follow_net_to_source(n);
                else
                    subfocus_left();
            }

            return;
        }
        case item_type::module:
        {
            return;
        }
    }
}

void selection_relay::navigate_right()
{
    switch (m_focus_type)
    {
        case item_type::none:
        {
            return;
        }
        case item_type::gate:
        {
            std::shared_ptr<gate> g = g_netlist->get_gate_by_id(m_focus_id);

            if (!g)
                return;

            if (m_subfocus == subfocus::right)
                follow_gate_output_pin(g, m_subfocus_index);
            else
            {
                if (s_navigation_skips_enabled && g->get_output_pins().size() == 1)
                    follow_gate_output_pin(g, 0);
                else
                    subfocus_right();
            }

            return;
        }
        case item_type::net:
        {
            std::shared_ptr<net> n = g_netlist->get_net_by_id(m_focus_id);

            if (!n)
                return;

            if (m_subfocus == subfocus::right)
            {
                follow_net_to_destination(n, m_subfocus_index);
                return;
            }

            if (s_navigation_skips_enabled && n->get_destinations().size() == 1)
                follow_net_to_destination(n, 0);
            else
                subfocus_right();

            return;
        }
        case item_type::module:
        {
            return;
        }
    }
}

void selection_relay::handle_module_removed(const u32 id)
{
    auto it = m_selected_modules.find(id);
    if (it != m_selected_modules.end())
    {
        m_selected_modules.erase(it);
        Q_EMIT selection_changed(nullptr);
    }
}

void selection_relay::handle_gate_removed(const u32 id)
{
    auto it = m_selected_gates.find(id);
    if (it != m_selected_gates.end())
    {
        m_selected_gates.erase(it);
        Q_EMIT selection_changed(nullptr);
    }
}

void selection_relay::handle_net_removed(const u32 id)
{
    auto it = m_selected_nets.find(id);
    if (it != m_selected_nets.end())
    {
        m_selected_nets.erase(it);
        Q_EMIT selection_changed(nullptr);
    }
}

// GET CORE GUARANTEES
// UNCERTAIN ABOUT UNROUTED (GLOBAL) NETS, DECIDE
void selection_relay::follow_gate_input_pin(std::shared_ptr<gate> g, u32 input_pin_index)
{
    std::string pin_type = *std::next(g->get_input_pins().begin(), input_pin_index);
    std::shared_ptr<net> n = g->get_fan_in_net(pin_type);

    if (!n)
        return;    // ADD SOUND OR SOMETHING, ALTERNATIVELY ADD BOOL RETURN VALUE TO METHOD ???

    clear();

    m_selected_nets.insert(n->get_id());

    m_focus_type = item_type::net;
    m_focus_id   = n->get_id();

    if (n->get_destinations().size() == 1)
    {
        if (s_navigation_skips_enabled)
            m_subfocus = subfocus::none;
        else
            m_subfocus = subfocus::right;

        m_subfocus_index = 0;
    }
    else
    {
        int i = 0;
        for (endpoint& e : n->get_destinations())
        {
            if (e.get_gate() == g && e.get_pin() == pin_type)
                break;

            ++i;
        }

        m_subfocus       = subfocus::right;
        m_subfocus_index = i;
    }

    Q_EMIT selection_changed(nullptr);
}

void selection_relay::follow_gate_output_pin(std::shared_ptr<gate> g, u32 output_pin_index)
{
    std::string pin_type = *std::next(g->get_output_pins().begin(), output_pin_index);
    std::shared_ptr<net> n = g->get_fan_out_net(pin_type);

    if (!n)
        return;    // ADD SOUND OR SOMETHING, ALTERNATIVELY ADD BOOL RETURN VALUE TO METHOD ???

    clear();

    m_selected_nets.insert(n->get_id());

    m_focus_type = item_type::net;
    m_focus_id   = n->get_id();

    if (s_navigation_skips_enabled)
        m_subfocus = subfocus::none;
    else
        m_subfocus = subfocus::left;

    m_subfocus_index = 0;

    Q_EMIT selection_changed(nullptr);
}

void selection_relay::follow_net_to_source(std::shared_ptr<net> n)
{
    endpoint e              = n->get_source();
    std::shared_ptr<gate> g = e.get_gate();

    if (!g)
        return;

    clear();

    m_selected_gates.insert(g->get_id());

    m_focus_type = item_type::gate;
    m_focus_id   = g->get_id();

    if (s_navigation_skips_enabled && g->get_output_pins().size() == 1)
    {
        m_subfocus       = subfocus::left;    // NONE OR LEFT ???
        m_subfocus_index = 0;
    }
    else
    {
        int i = 0;
        for (const std::string& pin_type: g->get_output_pins())
        {
            if (pin_type == e.get_pin())
                break;

            ++i;
        }

        m_subfocus       = subfocus::right;
        m_subfocus_index = i;
    }

    Q_EMIT selection_changed(nullptr);
}

void selection_relay::follow_net_to_destination(std::shared_ptr<net> n, u32 dst_index)
{
    endpoint e              = n->get_destinations().at(dst_index);
    std::shared_ptr<gate> g = e.get_gate();

    if (!g)
        return;

    clear();

    m_selected_gates.insert(g->get_id());

    m_focus_type = item_type::gate;
    m_focus_id   = g->get_id();

    if (s_navigation_skips_enabled && g->get_input_pins().size() == 1)
    {
        m_subfocus       = subfocus::right;    // NONE OR RIGHT ???
        m_subfocus_index = 0;
    }
    else
    {
        int i = 0;
        for (const std::string& pin_type: g->get_input_pins())
        {
            if (pin_type == e.get_pin())
                break;

            ++i;
        }

        m_subfocus       = subfocus::left;
        m_subfocus_index = i;
    }

    Q_EMIT selection_changed(nullptr);
}

void selection_relay::subfocus_none()
{
    m_subfocus       = subfocus::none;
    m_subfocus_index = 0;    // TECHNICALLY REDUNDANT, KEEP FOR COMPLETENESS ???

    Q_EMIT subfocus_changed(nullptr);
}

void selection_relay::subfocus_left()
{
    m_subfocus       = subfocus::left;
    m_subfocus_index = 0;

    Q_EMIT subfocus_changed(nullptr);
}

void selection_relay::subfocus_right()
{
    m_subfocus       = subfocus::right;
    m_subfocus_index = 0;

    Q_EMIT subfocus_changed(nullptr);
}
