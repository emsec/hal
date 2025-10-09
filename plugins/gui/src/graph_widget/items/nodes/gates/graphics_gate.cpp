#include "gui/graph_widget/items/nodes/gates/graphics_gate.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include <QHash>

namespace hal
{
    GraphicsGate::GraphicsGate(Gate* g)
        : GraphicsNode(ItemType::Gate, g->get_id()),
          mGate(g),
          mType(QString::fromStdString(g->get_type()->get_name())),
          mShapeType(StandardShape),
          mPathWidth(0),
          mMaxInputPinWidth(0),
          mMaxOutputPinWidth(0)
    {
        QString textLines[3];
        textLines[0] = QString::fromStdString(g->get_name());
        const GateType* gt = g->get_type();

        for (GateTypeProperty& gtprop : gt->get_property_list())
        {
            switch(gtprop) {
                case GateTypeProperty::c_inverter:
                    mShapeType = InverterShape;
                    break;
                case hal::GateTypeProperty::c_and:
                    mShapeType = AndShape;
                    break;
                case hal::GateTypeProperty::c_nand:
                    mShapeType = NandShape;
                    break;
                case hal::GateTypeProperty::c_or:
                    mShapeType = OrShape;
                    break;
                case hal::GateTypeProperty::c_nor:
                    mShapeType = NorShape;
                    break;
                case hal::GateTypeProperty::c_xor:
                    mShapeType = XorShape;
                    break;
                case hal::GateTypeProperty::c_xnor:
                    mShapeType = NxorShape;
                    break;
                default:
                    break;
            }
        }

        if (mShapeType != StandardShape)
            mPathWidth = 10;

        for (const GatePin* input_pin : gt->get_input_pins())
        {
            u32 netId = 0;
            int index = mInputPinStruct.size();
            const Net* n = g->get_fan_in_net(input_pin);
            if (n)
            {
                netId = n->get_id();
                mInputByNet.insert(netId, index);
            }
            else
            {
                mInputByNet.insert(0, index);
            }
            mInputPinStruct.append({QString::fromStdString(input_pin->get_name()), 0., netId, index, true});
        }

        for (const GatePin* output_pin : gt->get_output_pins())
        {
            u32 netId = 0;
            int index = mOutputPinStruct.size();
            const Net* n = g->get_fan_out_net(output_pin);
            if (n)
            {
                netId = n->get_id();
                mOutputByNet.insert(netId, index);
            }
            else
            {
                mOutputByNet.insert(0, index);
            }
            mOutputPinStruct.append({QString::fromStdString(output_pin->get_name()), 0., netId, index, false});
        }

        textLines[1] = QString::fromStdString(gt->get_name());
        setNodeText(textLines,true);
    }

    QList<QLineF> GraphicsGate::connectors(u32 netId, bool isInput) const
    {
        if (!netId) return  QList<QLineF>();
        int inx = isInput ? mInputByNet.value(netId,-1) : mOutputByNet.value(netId,-1);
        if (inx < 0) return QList<QLineF>();
        return isInput ? mInputPinStruct.at(inx).connectors : mOutputPinStruct.at(inx).connectors;
    }
}    // namespace hal
