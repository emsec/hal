#include "selection_history_navigator/selection_history_navigator.h"

#include "gui_globals.h"

#include <QList>

namespace hal
{
    selection_history_navigator::selection_history_navigator(unsigned int max_history_size, QObject* parent) : QObject(parent)
    {
        set_max_history_size(max_history_size);

        m_current_item_iterator = m_selection_container.begin();

        g_selection_relay.register_sender(this, "History Navigator");

        connect(&g_selection_relay, &selection_relay::selection_changed, this, &selection_history_navigator::handle_selection_changed);
    }

    selection_history_navigator::~selection_history_navigator()
    {
    }

    void selection_history_navigator::handle_selection_changed(void* sender)
    {
        if (sender == this)
            return;

        if (!g_selection_relay.m_selected_gates.isEmpty())
        {
            store_selection(*g_selection_relay.m_selected_gates.begin(), selection_relay::item_type::gate);
        }
        else if (!g_selection_relay.m_selected_nets.isEmpty())
        {
            store_selection(*g_selection_relay.m_selected_nets.begin(), selection_relay::item_type::net);
        }
    }

    void selection_history_navigator::store_selection(u32 id, selection_relay::item_type type)
    {
        m_current_item_iterator = m_selection_container.insert(m_current_item_iterator, selection(id, type));

        if (m_selection_container.size() > m_max_history_size)
            m_selection_container.pop_back();
    }

    void selection_history_navigator::navigate_to_prev_item()
    {
        if (!(m_current_item_iterator == --m_selection_container.end()))
            relay_selection(*++m_current_item_iterator);
    }

    void selection_history_navigator::navigate_to_next_item()
    {
        if (!(m_current_item_iterator == m_selection_container.begin()))
            relay_selection(*--m_current_item_iterator);
    }

    void selection_history_navigator::relay_selection(selection selection)
    {
        g_selection_relay.clear();

        selection_relay::item_type type = selection.get_type();

        if (type == selection_relay::item_type::net)
        {
            g_selection_relay.m_selected_nets.insert(selection.get_net_id());
        }
        else if (type == selection_relay::item_type::gate)
        {
            g_selection_relay.m_selected_gates.insert(selection.get_gate_id());
        }

        Q_EMIT g_selection_relay.selection_changed(this);
    }

    void selection_history_navigator::set_max_history_size(unsigned int max_size)
    {
        m_max_history_size = max_size;
    }

    unsigned int selection_history_navigator::get_max_history_size() const
    {
        return m_max_history_size;
    }
}
