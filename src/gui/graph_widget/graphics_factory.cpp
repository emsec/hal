#include "gui/graph_widget/graphics_factory.h"

#include "gui/graph_widget/items/nodes/gates/standard_graphics_gate.h"
#include "gui/graph_widget/items/nodes/modules/standard_graphics_module.h"

namespace hal
{
    namespace graphics_factory
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

        graphics_module* create_graphics_module(const std::shared_ptr<Module> m, const int type)
        {
            assert(m);

            switch (type)
            {
            case 0: return new standard_graphics_module(m);
            default: assert(false); return nullptr; // UNREACHABLE
            }
        }

        graphics_gate* create_graphics_gate(const std::shared_ptr<const Gate> g, const int type)
        {
            assert(g);

            switch (type)
            {
            case 0: return new standard_graphics_gate(g);
            default: assert(false); return nullptr; // UNREACHABLE
            }
        }
    }
}
