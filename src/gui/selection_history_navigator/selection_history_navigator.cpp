#include "selection_history_navigator/selection_history_navigator.h"

#include "gui_globals.h"

#include <QList>

selection_history_navigator::selection_history_navigator(QObject* parent, unsigned int max_history_size) : QObject(parent)
{
    set_max_history_size(max_history_size);
    m_current_item_iterator = m_gate_id_container.begin();
    g_selection_relay.register_sender(this, "History Navigator");
    connect(&g_selection_relay, &selection_relay::current_gate_update, this, &selection_history_navigator::handle_gate_focused);
}

selection_history_navigator::~selection_history_navigator(){}

void selection_history_navigator::handle_gate_focused(void* sender, u32 id)
{
    Q_UNUSED(sender)
 
    if(m_gate_id_container.empty() || id != *m_current_item_iterator)
    {
        m_current_item_iterator = m_gate_id_container.insert(m_current_item_iterator, id);

        if(m_gate_id_container.size() > m_max_history_size)
            m_gate_id_container.pop_back();
    }
}

void selection_history_navigator::navigate_to_next_gate()
{
    if(!(m_current_item_iterator == m_gate_id_container.begin()))
            relay_navigation(*--m_current_item_iterator);
}

void selection_history_navigator::navigate_to_prev_gate()
{
    if(!(m_current_item_iterator == --m_gate_id_container.end()))
        relay_navigation(*++m_current_item_iterator);
}

void selection_history_navigator::relay_navigation(u32 id)
{
    QList<u32> net_ids;
    QList<u32> gate_ids;
    QList<u32> submod_ids;

    gate_ids.append(id);

    g_selection_relay.relay_current_gate(this, *gate_ids.begin());
    g_selection_relay.relay_combined_selection(this, gate_ids, net_ids, submod_ids);    
}

void selection_history_navigator::set_max_history_size(unsigned int max_size)
{
    m_max_history_size = max_size;
}

unsigned int selection_history_navigator::get_max_history_size()
{
    return m_max_history_size;
}
