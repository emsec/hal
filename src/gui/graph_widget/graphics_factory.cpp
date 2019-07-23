#include "gui/graph_widget/graphics_factory.h"

#include "gui/graph_widget/items/standard_graphics_gate.h"
#include "gui/graph_widget/items/io_graphics_net.h"
#include "gui/graph_widget/items/separated_graphics_net.h"
#include "gui/graph_widget/items/minimal_graphics_gate.h"
#include "gui/graph_widget/items/standard_graphics_module.h"

namespace graphics_factory
{

QStringList graphics_module_types()
{
    return QStringList
    {
        "Standard Module",
        "Minimal Module"
    };
}

QStringList graphics_gate_types()
{
    return QStringList
    {
        "Standard Gate",
        "Minimal Gate"
    };
}

graphics_module* create_graphics_module(const std::shared_ptr<const module> m, const int type)
{
    assert(m);

    switch (type)
    {
    case 0: return new standard_graphics_module(m);
    default: return nullptr;
    }
}

graphics_gate* create_graphics_gate(const std::shared_ptr<const gate> g, const int type)
{
    assert(g);

    switch (type)
    {
    case 0: return new standard_graphics_gate(g);
    case 1: return new minimal_graphics_gate(g);
    default: return nullptr;
    }
}
}
