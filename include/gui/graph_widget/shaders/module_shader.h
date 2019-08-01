#ifndef MODULE_SHADER_H
#define MODULE_SHADER_H

#include "gui/graph_widget/shaders/graph_shader.h"

class module_shader : public graph_shader
{
public:
    module_shader(const graph_context* const context);

    virtual void add(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets) Q_DECL_OVERRIDE;
    virtual void remove(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets) Q_DECL_OVERRIDE;

    virtual void update() Q_DECL_OVERRIDE;
    virtual void reset() Q_DECL_OVERRIDE;

private:
    static bool s_color_gates;
};

#endif // MODULE_SHADER_H
