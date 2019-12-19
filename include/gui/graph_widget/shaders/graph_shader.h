#ifndef GRAPH_SHADER_H
#define GRAPH_SHADER_H

#include "def.h"

#include "gui/graph_widget/items/graphics_net.h"
#include "gui/graph_widget/items/graphics_node.h"

#include <QColor>
#include <QMap>
#include <QSet>
#include <QVector>

class graph_context;

class graph_shader
{
public:
    struct shading
    {
        QMap<u32, graphics_node::visuals> module_visuals;
        QMap<u32, graphics_node::visuals> gate_visuals;
        QMap<u32, graphics_net::visuals> net_visuals;
    };

    graph_shader(const graph_context* const context);

    virtual void add(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets) = 0;
    virtual void remove(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets) = 0;

    virtual void update() = 0;
    virtual void reset() = 0;

    const shading& get_shading();

protected:
    const graph_context* const m_context;

    shading m_shading;
};

#endif // GRAPH_SHADER_H
