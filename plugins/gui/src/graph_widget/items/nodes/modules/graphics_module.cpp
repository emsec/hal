#include "gui/graph_widget/items/nodes/modules/graphics_module.h"

#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"

#include <QDebug>

namespace hal
{
    GraphicsModule::GraphicsModule(Module* m) : GraphicsNode(ItemType::Module, m->get_id(), QString::fromStdString(m->get_name()))
    {
        mNodeText[1]                              = QString::fromStdString(m->get_type());
        mNodeText[mNodeText[1].isEmpty() ? 1 : 2] = "Module";

        for (hal::ModulePin* pin : m->get_pins())
        {
            u32 netId       = pin->get_net()->get_id();
            QString pinName = QString::fromStdString(pin->get_name());

            switch (pin->get_direction())
            {
                case PinDirection::input:
                    mInputPins.append(hal::GraphicsModule::ModulePin{pinName, netId});
                    mInputByNet.insert(netId, mInputPins.size());
                    break;
                case PinDirection::output:
                    mOutputPins.append(hal::GraphicsModule::ModulePin{pinName, netId});
                    mOutputByNet.insert(netId, mInputPins.size());
                    break;
                case PinDirection::inout:
                    mInputPins.append(hal::GraphicsModule::ModulePin{pinName, netId});
                    mOutputPins.append(hal::GraphicsModule::ModulePin{pinName, netId});
                    mInputByNet.insert(netId, mInputPins.size());
                    mOutputByNet.insert(netId, mInputPins.size());
                    break;
                default:
                    break;
            }
        }
    }
}    // namespace hal
