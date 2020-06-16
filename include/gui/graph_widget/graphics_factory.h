#pragma once

#include "gui/graph_widget/items/nodes/gates/graphics_gate.h"
#include "gui/graph_widget/items/nodes/modules/graphics_module.h"

#include <QStringList>

#include <cassert>
#include <memory>

namespace graphics_factory
{
QStringList graphics_module_types();
QStringList graphics_gate_types();

graphics_module* create_graphics_module(const std::shared_ptr<Module> m, const int type);
graphics_gate* create_graphics_gate(const std::shared_ptr<const Gate> g, const int type);
}
