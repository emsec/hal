#include "selection_history_navigator/selection_history_navigator.h"

#include "gui_globals.h"

#include <QList>

selection_history_navigator::selection_history_navigator(unsigned int max_history_size, QObject* parent) : QObject(parent)
{
    set_max_history_size(max_history_size);

    m_current_item_iterator = m_selection_container.begin();

    g_selection_relay.register_sender(this, "History Navigator");

    connect(&g_selection_relay, &selection_relay::current_gate_update, this, &selection_history_navigator::handle_gate_focused);
    connect(&g_selection_relay, &selection_relay::current_net_update, this, &selection_history_navigator::handle_net_focused);
}

selection_history_navigator::~selection_history_navigator(){}


void selection_history_navigator::handle_gate_focused(void* sender, u32 id)
{
    if(sender == this)
        return;

    store_selection(id, selection_relay::single_type::gate);
}

void selection_history_navigator::handle_net_focused(void* sender, u32 id)
{
    if(sender == this)
        return;

    store_selection(id, selection_relay::single_type::net);
}

void selection_history_navigator::store_selection(u32 id, selection_relay::single_type type)
{
    m_current_item_iterator = m_selection_container.insert(m_current_item_iterator, selection(id, type));

    if(m_selection_container.size() > m_max_history_size)
            m_selection_container.pop_back();
}

void selection_history_navigator::navigate_to_prev_item()
{
    if(!(m_current_item_iterator == --m_selection_container.end()))
        relay_selection(*++m_current_item_iterator);    
}

void selection_history_navigator::navigate_to_next_item()
{
    if(!(m_current_item_iterator == m_selection_container.begin()))
            relay_selection(*--m_current_item_iterator); 
}

void selection_history_navigator::relay_selection(selection selection)
{
    QList<u32> net_ids;
    QList<u32> gate_ids;
    QList<u32> submod_ids;

    selection_relay::single_type type = selection.get_type();

    if(type == selection_relay::single_type::net)
    {
        net_ids.append(selection.get_net_id());
        g_selection_relay.relay_current_net(this, *net_ids.begin());
    }
    else if(type == selection_relay::single_type::gate)
    {
        gate_ids.append(selection.get_gate_id());
        g_selection_relay.relay_current_gate(this, *gate_ids.begin());
    }

    g_selection_relay.relay_combined_selection(this, gate_ids, net_ids, submod_ids);    
}

void selection_history_navigator::set_max_history_size(unsigned int max_size)
{
    m_max_history_size = max_size;
}

unsigned int selection_history_navigator::get_max_history_size() const
{
    return m_max_history_size;
}
