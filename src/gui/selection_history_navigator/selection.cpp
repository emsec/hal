#include "selection_history_navigator/selection.h"

namespace hal
{
    selection::selection(u32 id, SelectionRelay::item_type type)
    {
        m_type = type;

        if(type == SelectionRelay::item_type::net)
            m_net_ids.push_back(id);
        else if(type == SelectionRelay::item_type::gate)
            m_gate_ids.push_back(id);
    }

    selection::~selection(){}

    SelectionRelay::item_type selection::get_type() const
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
}
