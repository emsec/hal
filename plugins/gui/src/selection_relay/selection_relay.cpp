#include "gui/selection_relay/selection_relay.h"

#include "gui/gui_globals.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/utilities/log.h"

namespace hal
{
    // SET VIA SETTINGS OR TOOLBUTTON
    bool SelectionRelay::s_navigation_skips_enabled = false;

    SelectionRelay::SelectionRelay(QObject* parent) : QObject(parent),
        m_current_type(ItemType::None), m_focus_type(ItemType::None), m_subfocus(Subfocus::None)
    {
        clear();
    }

    void SelectionRelay::clear()
    {
        mModulesSuppressedByFilter.clear();
        mGatesSuppressedByFilter.clear();
        mNetsSuppressedByFilter.clear();
        m_selected_gates.clear();
        m_selected_nets.clear();
        m_selected_modules.clear();
        m_subfocus       = Subfocus::None;
        m_subfocus_index = 0;
        m_focus_id       = 0;
    }

    void SelectionRelay::clear_and_update()
    {
        clear();
        Q_EMIT selection_changed(nullptr);
    }

    void SelectionRelay::register_sender(void* sender, QString name)
    {
        m_sender_register.append(QPair<void*, QString>(sender, name));
    }

    void SelectionRelay::remove_sender(void* sender)
    {
        for (QPair<void*, QString> pair : m_sender_register)
        {
            if (pair.first == sender)
                m_sender_register.removeOne(pair);
        }
    }

    void SelectionRelay::relay_selection_changed(void* sender)
    {
        Q_EMIT selection_changed(sender);
    }

    void SelectionRelay::relay_subfocus_changed(void* sender)
    {
        Q_EMIT subfocus_changed(sender);
    }

    // TODO deduplicate navigate_up and navigate_down
    void SelectionRelay::navigate_up()
    {
        u32 size = 0;

        switch (m_focus_type)
        {
        case ItemType::None: {
                return;
            }
        case ItemType::Gate: {
                Gate* g = g_netlist->get_gate_by_id(m_focus_id);

                if (!g)
                    return;

                if (m_subfocus == Subfocus::Left)
                {
                    size = g->get_input_pins().size();

                    if (!size)     // CHECK NECESSARY ???
                        return;    // INVALID STATE, FIX OR IGNORE ???

                    break;
                }

                if (m_subfocus == Subfocus::Right)
                {
                    size = g->get_output_pins().size();

                    if (!size)     // CHECK NECESSARY ???
                        return;    // INVALID STATE, FIX OR IGNORE ???

                    break;
                }

                return;
            }
        case ItemType::Net: {
                Net* n = g_netlist->get_net_by_id(m_focus_id);

                if (!n)
                    return;

                if (m_subfocus == Subfocus::Right)
                {
                    size = n->get_destinations().size();

                    if (!size)     // CHECK NECESSARY ???
                        return;    // INVALID STATE, FIX OR IGNORE ???

                    break;
                }

                return;
            }
        case ItemType::Module: {
                Module* m = g_netlist->get_module_by_id(m_focus_id);

                if (!m)
                    return;

                if (m_subfocus == Subfocus::Left)
                {
                    size = m->get_input_nets().size();

                    if (!size)     // CHECK NECESSARY ???
                        return;    // INVALID STATE, FIX OR IGNORE ???

                    break;
                }

                if (m_subfocus == Subfocus::Right)
                {
                    size = m->get_output_nets().size();

                    if (!size)     // CHECK NECESSARY ???
                        return;    // INVALID STATE, FIX OR IGNORE ???

                    break;
                }

                return;
            }
        }

        if (m_subfocus_index == 0)
            m_subfocus_index = size - 1;
        else
            --m_subfocus_index;

        Q_EMIT subfocus_changed(nullptr);
    }

    void SelectionRelay::navigate_down()
    {
        u32 size = 0;

        switch (m_focus_type)
        {
        case ItemType::None: {
                return;
            }
        case ItemType::Gate: {
                Gate* g = g_netlist->get_gate_by_id(m_focus_id);

                if (!g)
                    return;

                if (m_subfocus == Subfocus::Left)
                {
                    size = g->get_input_pins().size();

                    if (!size)     // CHECK NECESSARY ???
                        return;    // INVALID STATE, FIX OR IGNORE ???

                    break;
                }

                if (m_subfocus == Subfocus::Right)
                {
                    size = g->get_output_pins().size();

                    if (!size)     // CHECK NECESSARY ???
                        return;    // INVALID STATE, FIX OR IGNORE ???

                    break;
                }

                return;
            }
        case ItemType::Net: {
                Net* n = g_netlist->get_net_by_id(m_focus_id);

                if (!n)
                    return;

                if (m_subfocus == Subfocus::Right)
                {
                    size = n->get_destinations().size();

                    if (!size)     // CHECK NECESSARY ???
                        return;    // INVALID STATE, FIX OR IGNORE ???

                    break;
                }

                return;
            }
        case ItemType::Module: {
                Module* m = g_netlist->get_module_by_id(m_focus_id);

                if (!m)
                    return;

                if (m_subfocus == Subfocus::Left)
                {
                    size = m->get_input_nets().size();

                    if (!size)     // CHECK NECESSARY ???
                        return;    // INVALID STATE, FIX OR IGNORE ???

                    break;
                }

                if (m_subfocus == Subfocus::Right)
                {
                    size = m->get_output_nets().size();

                    if (!size)     // CHECK NECESSARY ???
                        return;    // INVALID STATE, FIX OR IGNORE ???

                    break;
                }

                return;
            }
        }

        if (m_subfocus_index == size - 1)
            m_subfocus_index = 0;
        else
            ++m_subfocus_index;

        Q_EMIT subfocus_changed(nullptr);
    }

    // TODO nothing is using this method - do we need it?
    void SelectionRelay::navigate_left()
    {
        switch (m_focus_type)
        {
        case ItemType::None: {
                return;
            }
        case ItemType::Gate: {
                Gate* g = g_netlist->get_gate_by_id(m_focus_id);

                if (!g)
                    return;

                if (g->get_input_pins().size())    // CHECK HERE OR IN PRIVATE METHODS ?
                {
                    if (m_subfocus == Subfocus::Left)
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
        case ItemType::Net: {
                Net* n = g_netlist->get_net_by_id(m_focus_id);

                if (!n)
                    return;

                if (m_subfocus == Subfocus::Left)
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
        case ItemType::Module: {
                Module* m = g_netlist->get_module_by_id(m_focus_id);

                if (!m)
                    return;

                if (m->get_input_nets().size())    // CHECK HERE OR IN PRIVATE METHODS ?
                {
                    if (m_subfocus == Subfocus::Left)
                        follow_module_input_pin(m, m_subfocus_index);
                    else
                    {
                        if (s_navigation_skips_enabled && m->get_input_nets().size() == 1)
                            follow_module_input_pin(m, 0);
                        else
                            subfocus_left();
                    }
                }

                return;
            }
        }
    }

    // TODO nothing is using this method - do we need it?
    void SelectionRelay::navigate_right()
    {
        switch (m_focus_type)
        {
        case ItemType::None: {
                return;
            }
        case ItemType::Gate: {
                Gate* g = g_netlist->get_gate_by_id(m_focus_id);

                if (!g)
                    return;

                if (m_subfocus == Subfocus::Right)
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
        case ItemType::Net: {
                Net* n = g_netlist->get_net_by_id(m_focus_id);

                if (!n)
                    return;

                if (m_subfocus == Subfocus::Right)
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
        case ItemType::Module: {
                Module* m = g_netlist->get_module_by_id(m_focus_id);

                if (!m)
                    return;

                if (m_subfocus == Subfocus::Right)
                    follow_module_output_pin(m, m_subfocus_index);
                else
                {
                    if (s_navigation_skips_enabled && m->get_output_nets().size() == 1)
                        follow_module_output_pin(m, 0);
                    else
                        subfocus_right();
                }

                return;
            }
        }
    }

    void SelectionRelay::suppressedByFilter(const QList<u32>& modIds, const QList<u32>& gatIds, const QList<u32>& netIds)
    {
        mModulesSuppressedByFilter = modIds.toSet();
        mGatesSuppressedByFilter   = gatIds.toSet();
        mNetsSuppressedByFilter    = netIds.toSet();
        Q_EMIT selection_changed(nullptr);
    }

    bool SelectionRelay::isModuleSelected(u32 id) const
    {
        return m_selected_modules.contains(id) && !mModulesSuppressedByFilter.contains(id);
    }

    bool SelectionRelay::isGateSelected(u32 id) const
    {
        return m_selected_gates.contains(id) && !mGatesSuppressedByFilter.contains(id);
    }

    bool SelectionRelay::isNetSelected(u32 id) const
    {
        return m_selected_nets.contains(id) && !mNetsSuppressedByFilter.contains(id);
    }

    void SelectionRelay::handle_module_removed(const u32 id)
    {
        auto it = m_selected_modules.find(id);
        if (it != m_selected_modules.end())
        {
            m_selected_modules.erase(it);
            Q_EMIT selection_changed(nullptr);
        }
    }

    void SelectionRelay::handle_gate_removed(const u32 id)
    {
        auto it = m_selected_gates.find(id);
        if (it != m_selected_gates.end())
        {
            m_selected_gates.erase(it);
            Q_EMIT selection_changed(nullptr);
        }
    }

    void SelectionRelay::handle_net_removed(const u32 id)
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
    void SelectionRelay::follow_gate_input_pin(Gate* g, u32 input_pin_index)
    {
        std::string pin_type = *std::next(g->get_input_pins().begin(), input_pin_index);
        Net* n               = g->get_fan_in_net(pin_type);

        if (!n)
            return;    // ADD SOUND OR SOMETHING, ALTERNATIVELY ADD BOOL RETURN VALUE TO METHOD ???

        clear();

        m_selected_nets.insert(n->get_id());

        m_focus_type = ItemType::Net;
        m_focus_id   = n->get_id();

        if (n->get_destinations().size() == 1)
        {
            if (s_navigation_skips_enabled)
                m_subfocus = Subfocus::None;
            else
                m_subfocus = Subfocus::Right;

            m_subfocus_index = 0;
        }
        else
        {
            int i = 0;
            for (auto e : n->get_destinations())
            {
                if (e->get_gate() == g && e->get_pin() == pin_type)
                    break;

                ++i;
            }

            m_subfocus       = Subfocus::Right;
            m_subfocus_index = i;
        }

        Q_EMIT selection_changed(nullptr);
    }

    void SelectionRelay::follow_gate_output_pin(Gate* g, u32 output_pin_index)
    {
        std::string pin_type = *std::next(g->get_output_pins().begin(), output_pin_index);
        auto n               = g->get_fan_out_net(pin_type);

        if (!n)
            return;    // ADD SOUND OR SOMETHING, ALTERNATIVELY ADD BOOL RETURN VALUE TO METHOD ???

        clear();

        m_selected_nets.insert(n->get_id());

        m_focus_type = ItemType::Net;
        m_focus_id   = n->get_id();

        if (s_navigation_skips_enabled)
            m_subfocus = Subfocus::None;
        else
            m_subfocus = Subfocus::Left;

        m_subfocus_index = 0;

        Q_EMIT selection_changed(nullptr);
    }

    void SelectionRelay::follow_module_input_pin(Module* m, u32 input_pin_index)
    {
        Q_UNUSED(m)
        Q_UNUSED(input_pin_index)
        // TODO implement
    }

    void SelectionRelay::follow_module_output_pin(Module* m, u32 output_pin_index)
    {
        Q_UNUSED(m)
        Q_UNUSED(output_pin_index)
        // TODO implement
    }

    void SelectionRelay::follow_net_to_source(Net* n)
    {
        if(n->get_sources().empty())
            return;

        auto e = n->get_sources().at(0);
        auto g = e->get_gate();

        if (!g)
            return;

        clear();

        m_selected_gates.insert(g->get_id());

        m_focus_type = ItemType::Gate;
        m_focus_id   = g->get_id();

        if (s_navigation_skips_enabled && g->get_output_pins().size() == 1)
        {
            m_subfocus       = Subfocus::Left;    // NONE OR LEFT ???
            m_subfocus_index = 0;
        }
        else
        {
            int i = 0;
            for (const std::string& pin_type : g->get_output_pins())
            {
                if (pin_type == e->get_pin())
                    break;

                ++i;
            }

            m_subfocus       = Subfocus::Right;
            m_subfocus_index = i;
        }

        Q_EMIT selection_changed(nullptr);
    }

    void SelectionRelay::follow_net_to_destination(Net* n, u32 dst_index)
    {
        auto e  = n->get_destinations().at(dst_index);
        Gate* g = e->get_gate();

        if (!g)
            return;

        clear();

        m_selected_gates.insert(g->get_id());

        m_focus_type = ItemType::Gate;
        m_focus_id   = g->get_id();

        if (s_navigation_skips_enabled && g->get_input_pins().size() == 1)
        {
            m_subfocus       = Subfocus::Right;    // NONE OR RIGHT ???
            m_subfocus_index = 0;
        }
        else
        {
            int i = 0;
            for (const std::string& pin_type : g->get_input_pins())
            {
                if (pin_type == e->get_pin())
                    break;

                ++i;
            }

            m_subfocus       = Subfocus::Left;
            m_subfocus_index = i;
        }

        Q_EMIT selection_changed(nullptr);
    }

    void SelectionRelay::subfocus_none()
    {
        m_subfocus       = Subfocus::None;
        m_subfocus_index = 0;    // TECHNICALLY REDUNDANT, KEEP FOR COMPLETENESS ???

        Q_EMIT subfocus_changed(nullptr);
    }

    void SelectionRelay::subfocus_left()
    {
        m_subfocus       = Subfocus::Left;
        m_subfocus_index = 0;

        Q_EMIT subfocus_changed(nullptr);
    }

    void SelectionRelay::subfocus_right()
    {
        m_subfocus       = Subfocus::Right;
        m_subfocus_index = 0;

        Q_EMIT subfocus_changed(nullptr);
    }
}    // namespace hal
