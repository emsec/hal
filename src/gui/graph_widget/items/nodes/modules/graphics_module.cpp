#include "gui/graph_widget/items/nodes/modules/graphics_module.h"

#include "netlist/module.h"
#include "netlist/net.h"

graphics_module::graphics_module(const std::shared_ptr<Module> m) : graphics_node(hal::item_type::module, m->get_id(), QString::fromStdString(m->get_name()))
{
    for (const std::shared_ptr<Net>& n : m->get_input_nets())
        m_input_pins.append(module_pin{QString::fromStdString(m->get_input_port_name(n)), n->get_id()});

    for (const std::shared_ptr<Net>& n : m->get_output_nets())
        m_output_pins.append(module_pin{QString::fromStdString(m->get_output_port_name(n)), n->get_id()});
}
