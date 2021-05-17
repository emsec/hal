#include "gui/graph_widget/layouters/physical_graph_layouter.h"

#include "gui/gui_globals.h"

#include <limits>
#include <set>

namespace hal
{
    PhysicalGraphLayouter::PhysicalGraphLayouter(const GraphContext* const context)
        : GraphLayouter(context), mMinXDistance(std::numeric_limits<i32>::max()), mMinYDistance(std::numeric_limits<i32>::max())
    {
    }

    QString PhysicalGraphLayouter::name() const
    {
        return "Physical Layouter";
    }

    QString PhysicalGraphLayouter::mDescription() const
    {
        return "<p>PLACEHOLDER</p>";
    }

    void PhysicalGraphLayouter::add(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets, PlacementHint placement)
    {
        Q_UNUSED(modules)
        Q_UNUSED(gates)
        Q_UNUSED(nets)
        // TODO is it OK to ignore the placement hint in this layouter?
        Q_UNUSED(placement)

        // DOES THE CORE GUARANTEE UNIQUE LOCATION DATA ???
        // COLLAPSE / ALIGN

        std::set<i32> x_coordinates;
        std::set<i32> y_coordinates;

        for (u32 id : gates)
        {
            Gate* g = gNetlist->get_gate_by_id(id);
            assert(g);

            if (g->has_location())
            {
                x_coordinates.insert(g->get_location_x());
                y_coordinates.insert(g->get_location_y());
            }
        }

        for (i32 x1 : x_coordinates)
            for (i32 x2 : x_coordinates)
                if (x1 != x2)
                {
                    i32 x_distance = std::abs(x1 - x2);

                    if (x_distance < mMinXDistance)
                        mMinXDistance = x_distance;
                }

        for (i32 y1 : y_coordinates)
            for (i32 y2 : y_coordinates)
                if (y1 != y2)
                {
                    i32 y_distance = std::abs(y1 - y2);

                    if (y_distance < mMinYDistance)
                        mMinYDistance = y_distance;
                }
    }

    void PhysicalGraphLayouter::remove(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets)
    {
        Q_UNUSED(modules)
        Q_UNUSED(gates)
        Q_UNUSED(nets)

        for (u32 id : modules)
            removeNodeFromMaps(Node(id, Node::Module));

        for (u32 id : gates)
            removeNodeFromMaps(Node(id, Node::Gate));
    }
}    // namespace hal
