#include "gui/graph_widget/items/nodes/modules/graphics_module.h"

#include "netlist/module.h"
#include "netlist/net.h"

bool graphics_module::s_sort_pins = true;
bool graphics_module::s_show_differentiation_number = true;
bool graphics_module::s_show_occurence_number = true;

graphics_module::graphics_module(const std::shared_ptr<const module> m) : graphics_node(hal::item_type::module, m->get_id(), QString::fromStdString(m->get_name()))
{
    for (const std::shared_ptr<net>& n : m->get_input_nets())
    {
        QMap<std::string, int> occurrence_map;

        for (const endpoint& e : n->get_dsts())
        {
            if (e.gate)
                if (m->contains_gate(e.gate, true))
                    occurrence_map.insert(e.pin_type, occurrence_map.value(e.pin_type) + 1);
        }

        QMap<std::string, int>::const_iterator i = occurrence_map.constBegin();

        while (i != occurrence_map.constEnd())
        {
            QString differentiation = "(" + QString::number(n->get_id()) + ")"; // TEMP
            QString occurrence = "(x" + QString::number(i.value()) + ")";
            m_input_pins.append(module_pin{n->get_id(), QString::fromStdString(i.key()), differentiation, occurrence});
            ++i;
        }
    }

    for (const std::shared_ptr<net>& n : m->get_output_nets())
    {
        endpoint e = n->get_src();

        if (e.gate)
            m_output_pins.append(module_pin{n->get_id(), QString::fromStdString(e.pin_type), "", ""});
    }
}
