#pragma once

#include "gui/graph_widget/items/nets/standard_graphics_net.h"
#include "gui/graph_widget/items/nets/arrow_separated_net.h"

namespace hal {

    class StandardArrowNet : public StandardGraphicsNet
    {
        ArrowSeparatedNet mArrowNet;
    public:
        StandardArrowNet(Net* n, const lines& l);
        void setInputPosition(const QPointF &pos);
        virtual void set_visuals(const visuals& v);
        void paint(QPainter* painter,
                   const QStyleOptionGraphicsItem* option,
                   QWidget* widget) override;
    };

}
