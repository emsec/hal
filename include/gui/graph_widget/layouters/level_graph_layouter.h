#ifndef LEVEL_GRAPH_LAYOUTER_H
#define LEVEL_GRAPH_LAYOUTER_H

#include "gui/graph_widget/layouters/graph_layouter.h"

#include <QVector>

class level_graph_layouter final : public graph_layouter
{
public:
    level_graph_layouter(const graph_context* const context);

    virtual const QString name() const override;
    virtual const QString description() const override;

    virtual void add(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets) override;
    virtual void remove(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets) override;

private:
    void add_gate(const u32 gate_id, const int level);
    void recalculate_levels();

    QMap<hal::node, int> m_node_levels;

    // COMBINE LEVEL VECTORS ???
    QVector<hal::node> m_zero_nodes;

    QVector<QVector<hal::node>> m_positive_nodes;
    QVector<QVector<hal::node>> m_negative_nodes;
};

#endif // LEVEL_GRAPH_LAYOUTER_H
