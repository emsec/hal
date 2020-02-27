#ifndef MODULE_SHADER_H
#define MODULE_SHADER_H

#include "gui/graph_widget/shaders/graph_shader.h"

class module_shader final : public graph_shader
{
public:
    module_shader(const graph_context* const context);

    virtual void add(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets) override;
    virtual void remove(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets) override;
    virtual void update() override;

private:
    bool m_color_gates;
};

#endif // MODULE_SHADER_H
