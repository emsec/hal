#include "gui/selection_history_navigator/selection.h"

namespace hal
{
    Selection::Selection(u32 id, SelectionRelay::item_type type)
    {
        m_type = type;

        if(type == SelectionRelay::item_type::net)
            m_net_ids.push_back(id);
        else if(type == SelectionRelay::item_type::gate)
            m_gate_ids.push_back(id);
    }

    Selection::~Selection(){}

    SelectionRelay::item_type Selection::get_type() const
    {
        return m_type;
    }

    u32 Selection::get_net_id() const
    {
        return *m_net_ids.begin();
    }

    u32 Selection::get_gate_id() const
    {
        return *m_gate_ids.begin();
    }
}
