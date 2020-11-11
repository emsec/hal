#include "gui/graph_widget/items/nodes/gates/graphics_gate.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"

namespace hal
{
    GraphicsGate::GraphicsGate(Gate* g) : GraphicsNode(ItemType::Gate, g->get_id(), QString::fromStdString(g->get_name())), mType(QString::fromStdString(g->get_type()->get_name()))
    {
        for (const std::string& input_pin : g->get_input_pins())
        {
            const Net* n = g->get_fan_in_net(input_pin);
            if (n) mInputByNet.insert(n->get_id(),mInputPins.size());
            else mInputByNet.insert(0,mInputPins.size());
            mInputPins.append(QString::fromStdString(input_pin));
        }

        for (const std::string& output_pin : g->get_output_pins())
        {
            const Net* n = g->get_fan_out_net(output_pin);
            if (n) mOutputByNet.insert(n->get_id(),mOutputPins.size());
            else mOutputByNet.insert(0,mOutputPins.size());
            mOutputPins.append(QString::fromStdString(output_pin));
        }
        mNodeText[1] = QString::fromStdString(g->get_type()->get_name());
    }
}    // namespace hal
