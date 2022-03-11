#include "gui/graph_widget/items/nodes/gates/graphics_gate.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"

namespace hal
{
    GraphicsGate::GraphicsGate(Gate* g) : GraphicsNode(ItemType::Gate, g->get_id(), QString::fromStdString(g->get_name())), mGate(g), mType(QString::fromStdString(g->get_type()->get_name()))
    {
        const GateType* gt = g->get_type();
        for (const GatePin* input_pin : gt->get_input_pins())
        {
            if (const auto res = g->get_fan_in_net(input_pin); res.is_ok())
            {
                mInputByNet.insert(res.get()->get_id(), mInputPins.size());
            }
            else
            {
                mInputByNet.insert(0, mInputPins.size());
            }
            mInputPins.append(QString::fromStdString(input_pin->get_name()));
        }

        for (const GatePin* output_pin : gt->get_output_pins())
        {
            if (const auto res = g->get_fan_out_net(output_pin); res.is_ok())
            {
                mOutputByNet.insert(res.get()->get_id(), mOutputPins.size());
            }
            else
            {
                mOutputByNet.insert(0, mOutputPins.size());
            }
            mOutputPins.append(QString::fromStdString(output_pin->get_name()));
        }

        mNodeText[1] = QString::fromStdString(gt->get_name());
    }
}    // namespace hal
