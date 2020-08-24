#include "gui/graph_widget/graphics_factory.h"

#include "gui/graph_widget/items/nodes/gates/standard_graphics_gate.h"
#include "gui/graph_widget/items/nodes/modules/standard_graphics_module.h"

namespace hal
{
    namespace GraphicsFactory
    {
        QStringList graphics_module_types()
        {
            return QStringList
            {
                "Standard Module"
            };
        }

        QStringList graphics_gate_types()
        {
            return QStringList
            {
                "Standard Gate"
            };
        }

        GraphicsModule* create_graphics_module(Module* m, const int type)
        {
            assert(m);

            switch (type)
            {
            case 0: return new StandardGraphicsModule(m);
            default: assert(false); return nullptr; // UNREACHABLE
            }
        }

        GraphicsGate* create_graphics_gate(Gate* g, const int type)
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
