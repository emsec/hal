#include "gui/selection_history_navigator/selection_history_navigator.h"

#include "gui/gui_globals.h"

#include <QList>

namespace hal
{
    SelectionHistoryNavigator::SelectionHistoryNavigator(unsigned int max_history_size, QObject* parent) : QObject(parent)
    {
        set_max_history_size(max_history_size);

        m_current_item_iterator = m_selection_container.begin();

        g_selection_relay->register_sender(this, "History Navigator");

        connect(g_selection_relay, &SelectionRelay::selection_changed, this, &SelectionHistoryNavigator::handle_selection_changed);
    }

    SelectionHistoryNavigator::~SelectionHistoryNavigator()
    {
    }

    void SelectionHistoryNavigator::handle_selection_changed(void* sender)
    {
        if (sender == this)
            return;

        if (!g_selection_relay->m_selected_gates.isEmpty())
        {
            store_selection(*g_selection_relay->m_selected_gates.begin(), SelectionRelay::item_type::gate);
        }
        else if (!g_selection_relay->m_selected_nets.isEmpty())
        {
            store_selection(*g_selection_relay->m_selected_nets.begin(), SelectionRelay::item_type::net);
        }
    }

    void SelectionHistoryNavigator::store_selection(u32 id, SelectionRelay::item_type type)
    {
        m_current_item_iterator = m_selection_container.insert(m_current_item_iterator, Selection(id, type));

        if (m_selection_container.size() > m_max_history_size)
            m_selection_container.pop_back();
    }

    void SelectionHistoryNavigator::navigate_to_prev_item()
    {
        if (!(m_current_item_iterator == --m_selection_container.end()))
            relay_selection(*++m_current_item_iterator);
    }

    void SelectionHistoryNavigator::navigate_to_next_item()
    {
        if (!(m_current_item_iterator == m_selection_container.begin()))
            relay_selection(*--m_current_item_iterator);
    }

    void SelectionHistoryNavigator::relay_selection(Selection selection)
    {
        g_selection_relay->clear();

        SelectionRelay::item_type type = selection.get_type();

        if (type == SelectionRelay::item_type::net)
        {
            g_selection_relay->m_selected_nets.insert(selection.get_net_id());
        }
        else if (type == SelectionRelay::item_type::gate)
        {
            g_selection_relay->m_selected_gates.insert(selection.get_gate_id());
        }

        Q_EMIT g_selection_relay->selection_changed(this);
    }

    void SelectionHistoryNavigator::set_max_history_size(unsigned int max_size)
    {
        m_max_history_size = max_size;
    }

    unsigned int SelectionHistoryNavigator::get_max_history_size() const
    {
        return m_max_history_size;
    }
}
