#include "gui/graph_widget/items/nodes/modules/graphics_module.h"

#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"

namespace hal
{
    GraphicsModule::GraphicsModule(Module* m) : GraphicsNode(ItemType::Module, m->get_id(), QString::fromStdString(m->get_name()))
    {
        mNodeText[1]                              = QString::fromStdString(m->get_type());
        mNodeText[mNodeText[1].isEmpty() ? 1 : 2] = "Module";

        for (Net* n : m->get_input_nets())
            mInputPins.append(ModulePin{QString::fromStdString(m->get_input_port_name(n)), n->get_id()});

        for (Net* n : m->get_output_nets())
            mOutputPins.append(ModulePin{QString::fromStdString(m->get_output_port_name(n)), n->get_id()});

        if (mInputPins.size() > 1)
            std::sort(mInputPins.begin(), mInputPins.end());
        if (mOutputPins.size() > 1)
            std::sort(mOutputPins.begin(), mOutputPins.end());
        for (int inp=0; inp<mInputPins.size(); inp++)
            mInputByNet.insert(mInputPins.at(inp).mNetId,inp);

        for (int outp=0; outp<mOutputPins.size(); outp++)
            mOutputByNet.insert(mOutputPins.at(outp).mNetId,outp);
    }
}    // namespace hal
