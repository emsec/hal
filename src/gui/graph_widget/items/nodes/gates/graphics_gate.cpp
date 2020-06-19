#include "gui/graph_widget/items/nodes/gates/graphics_gate.h"

#include "netlist/gate.h"

namespace hal
{
    graphics_gate::graphics_gate(const std::shared_ptr<const Gate> g) : graphics_node(hal::item_type::gate, g->get_id(), QString::fromStdString(g->get_name())),
        m_type(QString::fromStdString(g->get_type()->get_name()))
    {
        for (const std::string& input_pin : g->get_input_pins())
            m_input_pins.append(QString::fromStdString(input_pin));

        for (const std::string& output_pin : g->get_output_pins())
            m_output_pins.append(QString::fromStdString(output_pin));
    }
}
