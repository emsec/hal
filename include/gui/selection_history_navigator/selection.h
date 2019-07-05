#ifndef SELECTION_H
#define SELECTION_H

#include "def.h"
#include "selection_relay/selection_relay.h"

#include <vector>

class selection
{

public:
    selection(u32 id, selection_relay::item_type type);
    ~selection();

    selection_relay::item_type get_type() const;

    u32 get_net_id() const;
    u32 get_gate_id() const;
   
private:
    selection_relay::item_type m_type;

    std::vector<u32> m_net_ids;
    std::vector<u32> m_gate_ids;
    std::vector<u32> m_module_ids;

};

#endif    // SELECTION_H
