#ifndef STANDARD_GRAPH_LAYOUTER_H
#define STANDARD_GRAPH_LAYOUTER_H

#include "gui/graph_widget/layouters/graph_layouter.h"

class standard_graph_layouter final : public graph_layouter
{
public:
    standard_graph_layouter(const graph_context* const context);

    virtual const QString name() const override;
    virtual const QString description() const override;

    virtual void add(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets) override;
    virtual void remove(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets) override;
};

#endif // STANDARD_GRAPH_LAYOUTER_H
