#include "gui/graph_widget/shaders/graph_shader.h"
#include <assert.h>

namespace hal
{
    GraphShader::GraphShader(const GraphContext* const context) :
        mContext(context)
    {
        assert(context);
    }

    const GraphShader::Shading& GraphShader::getShading()
    {
        return mShading;
    }
}
