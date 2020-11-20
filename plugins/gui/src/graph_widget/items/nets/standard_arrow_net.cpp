#include "gui/graph_widget/items/nets/standard_arrow_net.h"

namespace hal
{
    StandardArrowNet::StandardArrowNet(Net* n, const Lines &l)
        : StandardGraphicsNet(n,l), mArrowNet(n)
    {;}

    void StandardArrowNet::setInputPosition(const QPointF& pos)
    {
        mArrowNet.addInput(pos);
        QRectF r = mArrowNet.shape().boundingRect();
        mShape.addPath(mArrowNet.shape());
        mRect = mRect.united(r);
    }

    void StandardArrowNet::setOutputPosition(const QPointF& pos)
    {
        mArrowNet.addOutput(pos);
        QRectF r = mArrowNet.shape().boundingRect();
        mShape.addPath(mArrowNet.shape());
        mRect = mRect.united(r);
    }

    void StandardArrowNet::paint(QPainter* painter,
                                 const QStyleOptionGraphicsItem* option,
                                 QWidget* widget)
    {
        mArrowNet.setHightlight(mHighlight);
        StandardGraphicsNet::paint(painter,option,widget);
        mArrowNet.paint(painter,option,widget);
    }

    void StandardArrowNet::setVisuals(const Visuals &v)
    {
        StandardGraphicsNet::setVisuals(v);
        mArrowNet.setVisuals(v);
    }

}
