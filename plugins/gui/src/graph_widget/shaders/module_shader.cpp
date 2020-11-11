#include "gui/graph_widget/shaders/module_shader.h"

#include "hal_core/netlist/module.h"

#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/gui_globals.h"

namespace hal
{
    ModuleShader::ModuleShader(const GraphContext* const context) : GraphShader(context),
        mColorGates(true) // INITIALIZE WITH DEFAULT VALUE FROM SETTINGS
    {
    }

    void ModuleShader::add(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets)
    {
        Q_UNUSED(modules)
        Q_UNUSED(gates)
        Q_UNUSED(nets)
    }

    void ModuleShader::remove(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets)
    {
        Q_UNUSED(modules)
        Q_UNUSED(gates)
        Q_UNUSED(nets)
    }

    void ModuleShader::update()
    {
        mShading.mOduleVisuals.clear();
        mShading.mGateVisuals.clear();
        mShading.mNetVisuals.clear();

        for (const u32& id : mContext->modules())
        {
            GraphicsNode::Visuals v;
            v.mMainColor = gNetlistRelay->getModuleColor(id);
            v.mVisible = true;
            mShading.mOduleVisuals.insert(id, v);
        }

        if (mColorGates)
        {
            for (const u32& id : mContext->gates())
            {
                Gate* g = gNetlist->get_gate_by_id(id);
                assert(g);

                Module* m = g->get_module();
                assert(m);

                GraphicsNode::Visuals v;
                v.mMainColor = gNetlistRelay->getModuleColor(m->get_id());
                v.mVisible = true;
                mShading.mGateVisuals.insert(id, v);
            }
        }

        // DEBUG CODE
        for (const u32& id : mContext->nets())
        {
            GraphicsNet::Visuals v{true, QColor(200, 200, 200), Qt::SolidLine, true, QColor(100, 100, 100), Qt::SolidPattern};
            mShading.mNetVisuals.insert(id, v);
        }
    }
}
