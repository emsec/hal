#include "gui/graph_widget/shaders/graph_shader.h"
#include <assert.h>
namespace hal{
graph_shader::graph_shader(const graph_context* const context) :
    m_context(context)
{
    assert(context);
}

const graph_shader::shading& graph_shader::get_shading()
{
    return m_shading;
}
}
