#include "gui/graph_widget/items/nets/graphics_net.h"

#include "hal_core/netlist/net.h"

#include <QPen>

namespace hal
{
    qreal GraphicsNet::sLineWidth;
    qreal GraphicsNet::sShapeWidth;

    QPen GraphicsNet::sPen;
    QBrush GraphicsNet::sBrush;

    void GraphicsNet::loadSettings()
    {
        sLineWidth = 1.8;
        sShapeWidth = 5;

        sPen.setWidthF(sLineWidth);
        sPen.setJoinStyle(Qt::MiterJoin);
    }

    GraphicsNet::GraphicsNet(Net* n) : GraphicsItem(ItemType::Net, n->get_id())
    {
    }

    QRectF GraphicsNet::boundingRect() const
    {
        return mRect;
    }

    QPainterPath GraphicsNet::shape() const
    {
        return mShape;
    }

    void GraphicsNet::setVisuals(const GraphicsNet::Visuals& v)
    {
        setVisible(v.mVisible);

        mColor = v.color;
        mPenStyle = v.mPenStyle;
        mFillIcon = v.mFillIcon;
        mFillColor = v.mFillColor;
        mBrushStyle = v.mBrushStyle;
    }
}
