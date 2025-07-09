#include "gui/graph_widget/items/nodes/modules/graphics_module.h"

#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"

#include <QDebug>

namespace hal
{
    GraphicsModule::GraphicsModule(Module* m) : GraphicsNode(ItemType::Module, m->get_id())
    {
        setModuleLabel(m, true);

        for (hal::ModulePin* pin : m->get_pins())
        {
            u32 netId       = pin->get_net()->get_id();
            QString pinName = QString::fromStdString(pin->get_name());

            switch (pin->get_direction())
            {
                case PinDirection::input:
                    mInputByNet.insert(netId, mInputPins.size());
                    mInputPins.append(hal::GraphicsModule::ModulePin{pinName, netId});
                    break;
                case PinDirection::output:
                    mOutputByNet.insert(netId, mOutputPins.size());
                    mOutputPins.append(hal::GraphicsModule::ModulePin{pinName, netId});
                    break;
                case PinDirection::inout:
                    mInputByNet.insert(netId, mInputPins.size());
                    mOutputByNet.insert(netId, mOutputPins.size());
                    mInputPins.append(hal::GraphicsModule::ModulePin{pinName, netId});
                    mOutputPins.append(hal::GraphicsModule::ModulePin{pinName, netId});
                    break;
                default:
                    break;
            }
        }
    }

    void GraphicsModule::setModuleLabel(const Module* m, bool init)
    {
        QString textLines[3];
        textLines[0]                              = QString::fromStdString(m->get_name());
        textLines[1]                              = QString::fromStdString(m->get_type());
        textLines[textLines[1].isEmpty() ? 1 : 2] = "Module";
        setNodeText(textLines,init);
    }
}    // namespace hal
