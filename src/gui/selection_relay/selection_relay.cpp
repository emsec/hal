#include "selection_relay/selection_relay.h"

#include "core/log.h"

#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/module.h"

#include "gui_globals.h"

#include <QDebug>

// SET VIA SETTINGS OR TOOLBUTTON
bool selection_relay::s_navigation_skips_enabled = false;

selection_relay::selection_relay(QObject* parent) : QObject(parent),
    m_selected_gates(nullptr),
    m_selected_nets(nullptr),
    m_selected_modules(nullptr),
    m_size_of_selected_gates(0),
    m_size_of_selected_nets(0),
    m_size_of_selected_modules(0),
    m_number_of_selected_gates(0),
    m_number_of_selected_nets(0),
    m_number_of_selected_modules(0),
    m_current_type(item_type::none),
    m_focus_type(item_type::none),
    m_subfocus(subfocus::none)
{
}

void selection_relay::init()
{
    // CONSTRUCTION + DESTRUCTION OR
    // INIT + RESET ?
    m_number_of_selected_gates = 0;
    m_number_of_selected_nets = 0;
    m_number_of_selected_modules = 0;

    m_size_of_selected_gates = g_netlist->get_gates().size();
    m_size_of_selected_nets = g_netlist->get_nets().size();
    m_size_of_selected_modules = g_netlist->get_modules().size();

    m_selected_gates = new u32[m_size_of_selected_gates];
    m_selected_nets = new u32[m_size_of_selected_nets];
    m_selected_modules = new u32[m_size_of_selected_modules];
}

void selection_relay::reset()
{
    // CONSTRUCTION + DESTRUCTION OR
    // INIT + RESET ?

    delete[] m_selected_gates;
    delete[] m_selected_nets;
    delete[] m_selected_modules;

    m_selected_gates = nullptr;
    m_selected_nets = nullptr;
    m_selected_modules = nullptr;

    m_size_of_selected_gates = 0;
    m_size_of_selected_nets = 0;
    m_size_of_selected_modules = 0;

    m_number_of_selected_gates = 0;
    m_number_of_selected_nets = 0;
    m_number_of_selected_modules = 0;
}

void swap(u32* a, u32* b)
{
    u32 tmp = *a;
    *a = *b;
    *b = tmp;
}

void quicksort_recursion(u32* begin, u32* end)
{
    u32* ptr;
    u32* split;

    if (end - begin <= 1)
        return;

    ptr = begin;
    split = begin + 1;

    while (++ptr <= end)
    {
        if (*ptr < *begin)
        {
            swap(ptr, split);
            ++split;
        }
    }

    swap(begin, split - 1);
    quicksort_recursion(begin, split - 1);
    quicksort_recursion(split, end);
}

void quicksort(u32* begin, u32* end)
{
    int difference = end - begin;

    if (difference < 1)
        return;

    if (difference == 1)
    {
        if (*begin > *end)
            swap(begin, end);

        return;
    }

    quicksort_recursion(begin, end);
}

void selection_relay::sort()
{
    //if (m_number_of_selected_gates > 1)
    quicksort(m_selected_gates, m_selected_gates + m_number_of_selected_gates);

    //if (m_number_of_selected_nets > 1)
    quicksort(m_selected_nets, m_selected_nets + m_number_of_selected_nets);

    //if (m_number_of_selected_submodules > 1)
    quicksort(m_selected_modules, m_selected_modules + m_number_of_selected_modules);

//    QString string = "Gates : ";
//    for (u32 i = 0; i < m_number_of_selected_gates; i++)
//    {
//        string += QString::number(m_selected_gates[i]);
//        string += ", ";
//    }

//    qDebug() << string;
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

void selection_relay::relay_gate_selection(void* sender, QList<u32>& gate_ids, Mode mode)
{
    Q_EMIT gate_selection_update(sender, gate_ids, mode);
}

void selection_relay::relay_net_selection(void* sender, QList<u32>& net_ids, Mode mode)
{
    Q_EMIT net_selection_update(sender, net_ids, mode);
}

void selection_relay::relay_module_selection(void* sender, QList<u32>& module_ids, Mode mode)
{
    Q_EMIT module_selection_update(sender, module_ids, mode);
}

void selection_relay::relay_combined_selection(void* sender, QList<u32>& gate_ids, QList<u32>& net_ids, QList<u32>& module_ids, Mode mode)
{
    QString name = "unknown";
    for (QPair<void*, QString> pair : m_sender_register)
    {
        if (pair.first == sender)
            name = pair.second;
    }
    QString string = name + " selection: ";
    switch (mode)
    {
        case Mode::override:
            string += "Override ";
            break;
        case Mode::add:
            string += "Add ";
            break;
        case Mode::remove:
            string += "Remove ";
            break;
    }

    int num_gates = gate_ids.length();

    if (num_gates)
    {
        string += QString::number(num_gates) + " gates (";

        QList<u32>::const_iterator iterator;
        iterator = gate_ids.begin();
        int i    = num_gates - 1;
        while (i)
        {
            string += QString::number(*iterator) + ", ";
            iterator++;
            i--;
        }
        string += QString::number(*iterator) + "), ";
    }
    else
        string += "0 gates, ";

    int num_nets = net_ids.length();

    if (num_nets)
    {
        string += QString::number(num_nets) + " nets (";

        QList<u32>::const_iterator iterator;
        iterator = net_ids.begin();
        int i    = num_nets - 1;
        while (i)
        {
            string += QString::number(*iterator) + ", ";
            iterator++;
            i--;
        }
        string += QString::number(*iterator) + "), ";
    }
    else
        string += "0 nets, ";

    int num_modules = module_ids.length();

    if (num_modules)
    {
        string += QString::number(num_modules) + " modules (";

        QList<u32>::const_iterator iterator;
        iterator = module_ids.begin();
        int i    = num_modules - 1;
        while (i)
        {
            string += QString::number(*iterator) + ", ";
            iterator++;
            i--;
        }
        string += QString::number(*iterator) + ")";
    }
    else
        string += "0 modules";

    log_info("user", string.toStdString());

    Q_EMIT combined_selection_update(sender, gate_ids, net_ids, module_ids, mode);
}

void selection_relay::relay_current_gate(void* sender, u32 gate_id)
{
    Q_EMIT current_gate_update(sender, gate_id);
}

void selection_relay::relay_current_net(void* sender, u32 net_id)
{
    Q_EMIT current_net_update(sender, net_id);
}

void selection_relay::relay_current_module(void* sender, u32 module_id)
{
    Q_EMIT current_module_update(sender, module_id);
}

void selection_relay::relay_current_cleared(void* sender)
{
    Q_EMIT current_cleared_update(sender);
}

void selection_relay::relay_current_deleted(void* sender)
{
    Q_EMIT current_deleted_update(sender);
}

void selection_relay::relay_jump_gate(void* sender, u32 gate_id)
{
    Q_EMIT jump_gate_update(sender, gate_id);
}

void selection_relay::relay_jump_net(void* sender, u32 net_id)
{
    Q_EMIT jump_net_update(sender, net_id);
}

void selection_relay::relay_jump_module(void* sender, u32 module_id)
{
    Q_EMIT jump_module_update(sender, module_id);
}

void selection_relay::relay_jump_selection(void* sender)
{
    Q_EMIT jump_selection_update(sender);
}

void selection_relay::relay_gate_highlight(void* sender, QList<u32>& ids, Mode mode, u32 channel)
{
    Q_EMIT gate_highlight_update(sender, ids, mode, channel);
}

void selection_relay::relay_net_highlight(void* sender, QList<u32>& ids, Mode mode, u32 channel)
{
    Q_EMIT net_highlight_update(sender, ids, mode, channel);
}

void selection_relay::relay_module_highlight(void* sender, QList<u32>& ids, Mode mode, u32 channel)
{
    Q_EMIT module_highlight_update(sender, ids, mode, channel);
}

void selection_relay::relay_combined_highlight(void* sender, QList<u32>& gate_ids, QList<u32>& net_ids, QList<u32>& module_ids, Mode mode, u32 channel)
{
    Q_EMIT combined_highlight_update(sender, gate_ids, net_ids, module_ids, mode, channel);
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
            size = g->get_input_pin_types().size();

            if (!size) // CHECK NECESSARY ???
                return; // INVALID STATE, FIX OR IGNORE ???

            break;
        }

        if (m_subfocus == subfocus::right)
        {
            size = g->get_output_pin_types().size();

            if (!size) // CHECK NECESSARY ???
                return; // INVALID STATE, FIX OR IGNORE ???

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
            size = n->get_dsts().size();

            if (!size) // CHECK NECESSARY ???
                return; // INVALID STATE, FIX OR IGNORE ???

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
            size = g->get_input_pin_types().size();

            if (!size) // CHECK NECESSARY ???
                return; // INVALID STATE, FIX OR IGNORE ???

            break;
        }

        if (m_subfocus == subfocus::right)
        {
            size = g->get_output_pin_types().size();

            if (!size) // CHECK NECESSARY ???
                return; // INVALID STATE, FIX OR IGNORE ???

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
            size = n->get_dsts().size();

            if (!size) // CHECK NECESSARY ???
                return; // INVALID STATE, FIX OR IGNORE ???

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

        if (g->get_input_pin_types().size()) // CHECK HERE OR IN PRIVATE METHODS ?
        {
            if (m_subfocus == subfocus::left)
                follow_gate_input_pin(g, m_subfocus_index);
            else
            {
                if (s_navigation_skips_enabled && g->get_input_pin_types().size() == 1)
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
            follow_net_to_src(n);
        else
        {
            if (s_navigation_skips_enabled && n->get_dsts().size() == 1)
                follow_net_to_src(n);
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
            if (s_navigation_skips_enabled && g->get_output_pin_types().size() == 1)
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
            follow_net_to_dst(n, m_subfocus_index);
            return;
        }

        if (s_navigation_skips_enabled && n->get_dsts().size() == 1)
            follow_net_to_dst(n, 0);
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

void selection_relay::handle_module_created()
{
    m_size_of_selected_modules++;
    u32* new_array = new u32[m_size_of_selected_modules];
    memcpy(new_array, m_selected_modules, m_number_of_selected_modules * sizeof(u32));
    delete[] m_selected_modules;
    m_selected_modules = new_array;
}

void selection_relay::handle_module_removed(const u32 id)
{
    m_size_of_selected_modules--;
    u32* new_array = new u32[m_size_of_selected_modules];
    bool was_selected = false;

    for (u32 i = 0; i < m_number_of_selected_modules; i++)
    {
        if (was_selected)
            new_array[i - 1] = m_selected_modules[i];
        else
        {
            if (m_selected_modules[i] != id)
                new_array[i] = m_selected_modules[i];
            else
                was_selected = true;
        }
    }

    delete[] m_selected_modules;
    m_selected_modules = new_array;

    if (was_selected)
        Q_EMIT selection_changed(nullptr);
}

void selection_relay::handle_gate_created()
{
    m_size_of_selected_gates++;
    u32* new_array = new u32[m_size_of_selected_gates];
    memcpy(new_array, m_selected_gates, m_number_of_selected_gates * sizeof(u32));
    delete[] m_selected_gates;
    m_selected_gates = new_array;
}

void selection_relay::handle_gate_removed(const u32 id)
{
    m_size_of_selected_gates--;
    u32* new_array = new u32[m_size_of_selected_gates];
    bool was_selected = false;

    for (u32 i = 0; i < m_number_of_selected_gates; i++)
    {
        if (was_selected)
            new_array[i - 1] = m_selected_gates[i];
        else
        {
            if (m_selected_gates[i] != id)
                new_array[i] = m_selected_gates[i];
            else
                was_selected = true;
        }
    }

    delete[] m_selected_gates;
    m_selected_gates = new_array;

    if (was_selected)
        Q_EMIT selection_changed(nullptr);
}

void selection_relay::handle_net_created()
{
    m_size_of_selected_nets++;
    u32* new_array = new u32[m_size_of_selected_nets];
    memcpy(new_array, m_selected_nets, m_number_of_selected_nets * sizeof(u32));
    delete[] m_selected_nets;
    m_selected_nets = new_array;
}

void selection_relay::handle_net_removed(const u32 id)
{
    m_size_of_selected_nets--;
    u32* new_array = new u32[m_size_of_selected_nets];
    bool was_selected = false;

    for (u32 i = 0; i < m_number_of_selected_nets; i++)
    {
        if (was_selected)
            new_array[i - 1] = m_selected_nets[i];
        else
        {
            if (m_selected_nets[i] != id)
                new_array[i] = m_selected_nets[i];
            else
                was_selected = true;
        }
    }

    delete[] m_selected_nets;
    m_selected_nets = new_array;

    if (was_selected)
        Q_EMIT selection_changed(nullptr);
}

// GET CORE GUARANTEES
// UNCERTAIN ABOUT UNROUTED (GLOBAL) NETS, DECIDE
void selection_relay::follow_gate_input_pin(std::shared_ptr<gate> g, u32 input_pin_index)
{
    std::string pin_type = *std::next(g->get_input_pin_types().begin(), input_pin_index);
    std::shared_ptr<net> n = g->get_fan_in_net(pin_type);

    if (!n)
        return; // ADD SOUND OR SOMETHING, ALTERNATIVELY ADD BOOL RETURN VALUE TO METHOD ???

    m_number_of_selected_gates = 0;
    m_number_of_selected_nets = 1;
    m_number_of_selected_modules = 0;

    m_selected_nets[0] = n->get_id();

    m_focus_type = item_type::net;
    m_focus_id = n->get_id();

    if (n->get_dsts().size() == 1)
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
        for (endpoint& e : n->get_dsts())
        {
            if (e.get_gate() == g && e.get_pin_type() == pin_type)
                break;

            ++i;
        }

        m_subfocus = subfocus::right;
        m_subfocus_index = i;
    }

    Q_EMIT selection_changed(nullptr);
}

void selection_relay::follow_gate_output_pin(std::shared_ptr<gate> g, u32 output_pin_index)
{
    std::string pin_type = *std::next(g->get_output_pin_types().begin(), output_pin_index);
    std::shared_ptr<net> n = g->get_fan_out_net(pin_type);

    if (!n)
        return; // ADD SOUND OR SOMETHING, ALTERNATIVELY ADD BOOL RETURN VALUE TO METHOD ???

    m_number_of_selected_gates = 0;
    m_number_of_selected_nets = 1;
    m_number_of_selected_modules = 0;

    m_selected_nets[0] = n->get_id();

    m_focus_type = item_type::net;
    m_focus_id = n->get_id();

    if (s_navigation_skips_enabled)
        m_subfocus = subfocus::none;
    else
        m_subfocus = subfocus::left;

    m_subfocus_index = 0;

    Q_EMIT selection_changed(nullptr);
}

void selection_relay::follow_net_to_src(std::shared_ptr<net> n)
{
    endpoint e = n->get_src();
    std::shared_ptr<gate> g = e.get_gate();

    if (!g)
        return;

    m_number_of_selected_gates = 1;
    m_number_of_selected_nets = 0;
    m_number_of_selected_modules = 0;

    m_selected_gates[0] = g->get_id();

    m_focus_type = item_type::gate;
    m_focus_id = g->get_id();

    if (s_navigation_skips_enabled && g->get_output_pin_types().size() == 1)
    {
        m_subfocus = subfocus::left; // NONE OR LEFT ???
        m_subfocus_index = 0;
    }
    else
    {
        int i = 0;
        for (const std::string& pin_type: g->get_output_pin_types())
        {
            if (pin_type == e.get_pin_type())
                break;

            ++i;
        }

        m_subfocus = subfocus::right;
        m_subfocus_index = i;
    }

    Q_EMIT selection_changed(nullptr);
}

void selection_relay::follow_net_to_dst(std::shared_ptr<net> n, u32 dst_index)
{
    endpoint e = n->get_dsts().at(dst_index);
    std::shared_ptr<gate> g = e.get_gate();

    if (!g)
        return;

    m_number_of_selected_gates = 1;
    m_number_of_selected_nets = 0;
    m_number_of_selected_modules = 0;

    m_selected_gates[0] = g->get_id();

    m_focus_type = item_type::gate;
    m_focus_id = g->get_id();

    if (s_navigation_skips_enabled && g->get_input_pin_types().size() == 1)
    {
        m_subfocus = subfocus::right; // NONE OR RIGHT ???
        m_subfocus_index = 0;
    }
    else
    {
        int i = 0;
        for (const std::string& pin_type: g->get_input_pin_types())
        {
            if (pin_type == e.get_pin_type())
                break;

            ++i;
        }

        m_subfocus = subfocus::left;
        m_subfocus_index = i;
    }

    Q_EMIT selection_changed(nullptr);
}

void selection_relay::subfocus_none()
{
    m_subfocus = subfocus::none;
    m_subfocus_index = 0; // TECHNICALLY REDUNDANT, KEEP FOR COMPLETENESS ???

    Q_EMIT subfocus_changed(nullptr);
}

void selection_relay::subfocus_left()
{
    m_subfocus = subfocus::left;
    m_subfocus_index = 0;

    Q_EMIT subfocus_changed(nullptr);
}

void selection_relay::subfocus_right()
{
    m_subfocus = subfocus::right;
    m_subfocus_index = 0;

    Q_EMIT subfocus_changed(nullptr);
}
