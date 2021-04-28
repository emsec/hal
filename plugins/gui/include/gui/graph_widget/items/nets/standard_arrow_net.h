#pragma once

#include "gui/graph_widget/items/nets/standard_graphics_net.h"
#include "gui/graph_widget/items/nets/arrow_separated_net.h"

namespace hal {

    /**
     * @ingroup graph-visuals-net
     * @brief A standard net that has parts of a separated net.
     *
     * The GraphicsNet that can be used to display nets in the scene. TODO: Purpose?
     */
    class StandardArrowNet : public StandardGraphicsNet
    {
        ArrowSeparatedNet mArrowNet;
    public:
        /**
         * Constructor.
         *
         * @param n - The underlying net
         * @param l - The lines this GraphicsNet consists of
         */
        StandardArrowNet(Net* n, const Lines& l);
        void setInputPosition(const QPointF &pos);
        void setOutputPosition(const QPointF &pos);
        virtual void setVisuals(const Visuals& v);
        void paint(QPainter* painter,
                   const QStyleOptionGraphicsItem* option,
                   QWidget* widget) override;
    };

}
