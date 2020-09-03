#include "gui/graph_widget/items/nodes/modules/graphics_module.h"

#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"

namespace hal
{
    GraphicsModule::GraphicsModule(Module* m) : GraphicsNode(hal::item_type::module, m->get_id(), QString::fromStdString(m->get_name()))
    {
        for (Net* n : m->get_input_nets())
            m_input_pins.append(module_pin{QString::fromStdString(m->get_input_port_name(n)), n->get_id()});

        for (Net* n : m->get_output_nets())
            m_output_pins.append(module_pin{QString::fromStdString(m->get_output_port_name(n)), n->get_id()});

        if (m_input_pins.size() > 1)
            std::sort(m_input_pins.begin(), m_input_pins.end());

        if (m_output_pins.size() > 1)
            std::sort(m_output_pins.begin(), m_output_pins.end());
    }
}
