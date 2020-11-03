#include "gui/graph_widget/items/nodes/gates/graphics_gate.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"

namespace hal
{
    GraphicsGate::GraphicsGate(Gate* g) : GraphicsNode(hal::item_type::gate, g->get_id(), QString::fromStdString(g->get_name())), m_type(QString::fromStdString(g->get_type()->get_name()))
    {
        for (const std::string& input_pin : g->get_input_pins())
        {
            const Net* n = g->get_fan_in_net(input_pin);
            if (n) mInputByNet.insert(n->get_id(),m_input_pins.size());
            else mInputByNet.insert(0,m_input_pins.size());
            m_input_pins.append(QString::fromStdString(input_pin));
        }

        for (const std::string& output_pin : g->get_output_pins())
        {
            const Net* n = g->get_fan_out_net(output_pin);
            if (n) mOutputByNet.insert(n->get_id(),m_output_pins.size());
            else mOutputByNet.insert(0,m_output_pins.size());
            m_output_pins.append(QString::fromStdString(output_pin));
        }
        mNodeText[1] = QString::fromStdString(g->get_type()->get_name());
    }
}    // namespace hal
