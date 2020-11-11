#include "gui/graph_widget/shaders/graph_shader.h"
#include <assert.h>

namespace hal
{
    GraphShader::GraphShader(const GraphContext* const context) :
        m_context(context)
    {
        assert(context);
    }

    const GraphShader::Shading& GraphShader::get_shading()
    {
        return m_shading;
    }
}
