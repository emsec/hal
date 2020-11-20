#include "gui/graph_widget/graphics_factory.h"

#include "gui/graph_widget/items/nodes/gates/standard_graphics_gate.h"
#include "gui/graph_widget/items/nodes/modules/standard_graphics_module.h"

namespace hal
{
    namespace GraphicsFactory
    {
        QStringList graphicsModuleTypes()
        {
            return QStringList
            {
                "Standard Module"
            };
        }

        QStringList graphicsGateTypes()
        {
            return QStringList
            {
                "Standard Gate"
            };
        }

        GraphicsModule* createGraphicsModule(Module* m, const int type)
        {
            assert(m);

            switch (type)
            {
            case 0: return new StandardGraphicsModule(m);
            default: assert(false); return nullptr; // UNREACHABLE
            }
        }

        GraphicsGate* createGraphicsGate(Gate* g, const int type)
        {
            assert(g);

            switch (type)
            {
            case 0: return new StandardGraphicsGate(g);
            default: assert(false); return nullptr; // UNREACHABLE
            }
        }
    }
}
