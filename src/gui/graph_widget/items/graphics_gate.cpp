#include "gui/graph_widget/items/graphics_gate.h"

#include "netlist/gate.h"

#include "gui/graph_widget/graph_widget_constants.h"

graphics_gate::graphics_gate(const std::shared_ptr<const gate> g) : graphics_node(hal::item_type::gate, g->get_id(), QString::fromStdString(g->get_name())),
    m_type(QString::fromStdString(g->get_type()->get_name()))
{
    assert(g);

    for (const std::string& input_pin : g->get_input_pins())
        m_input_pins.append(QString::fromStdString(input_pin));

    for (const std::string& output_pin : g->get_output_pins())
        m_output_pins.append(QString::fromStdString(output_pin));
}
