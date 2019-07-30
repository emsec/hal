#ifndef DEBUG_LAYOUTER_H
#define DEBUG_LAYOUTER_H

#include "gui/graph_widget/layouters/graph_layouter.h"

class debug_layouter final : public graph_layouter
{
public:
    debug_layouter(const graph_context* const context);

    virtual const QString name() const override;
    virtual const QString description() const override;

    virtual void layout() override;

    virtual void add(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets) override;
    virtual void remove(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets) override;

    virtual void expand(const u32 from_gate, const u32 via_net, const u32 to_gate) override;
};

#endif // DEBUG_LAYOUTER_H
