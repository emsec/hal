#pragma once

#include "gui/graph_widget/layouters/graph_layouter.h"

class standard_graph_layouter final : public graph_layouter
{
public:
    standard_graph_layouter(const graph_context* const context);

    QString name() const override;
    QString description() const override;

    void add(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets, hal::placement_hint placement) override;
    void remove(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets) override;

private:
    void add_compact(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& nets);
    void add_vertical(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& nets, bool left, const hal::node& preferred_origin);
};
