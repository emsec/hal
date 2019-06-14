#include "selection_history_navigator/selection.h"

selection::selection(u32 id, selection_relay::single_type type)
{
    m_type = type;

    if(type == selection_relay::single_type::net)
        m_net_ids.push_back(id);
    else if(type == selection_relay::single_type::gate)
        m_gate_ids.push_back(id);
}

selection::~selection(){}

selection_relay::single_type selection::get_type() const
{
    return m_type;
}

u32 selection::get_net_id() const
{
    return *m_net_ids.begin();
}

u32 selection::get_gate_id() const
{
    return *m_gate_ids.begin();
}
