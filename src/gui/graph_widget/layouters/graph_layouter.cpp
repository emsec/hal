#include "gui/graph_widget/layouters/graph_layouter.h"

#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_widget/graphics_factory.h"
#include "gui/graph_widget/graphics_scene.h"
#include "gui/graph_widget/items/io_graphics_net.h"
#include "gui/graph_widget/items/graphics_gate.h"
#include "gui/graph_widget/items/graphics_item.h"
#include "gui/graph_widget/items/graphics_net.h"
#include "gui/graph_widget/items/separated_graphics_net.h"
#include "gui/graph_widget/items/standard_graphics_net.h"
#include "gui/gui_globals.h"

graph_layouter::graph_layouter(const graph_context* const context) :
    m_scene(new graphics_scene()),
    m_context(context)
{

}

graph_layouter::~graph_layouter()
{
    delete m_scene;
}

graphics_scene* graph_layouter::scene() const
{
    return m_scene;
}
