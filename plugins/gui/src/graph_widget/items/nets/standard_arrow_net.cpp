#include "gui/graph_widget/items/nets/standard_arrow_net.h"

namespace hal
{
    StandardArrowNet::StandardArrowNet(Net* n, const lines& l)
        : StandardGraphicsNet(n,l), mArrowNet(n)
    {;}

    void StandardArrowNet::setInputPosition(const QPointF& pos)
    {
        mArrowNet.add_input(pos);
        QRectF r = mArrowNet.shape().boundingRect();
        m_shape.addPath(mArrowNet.shape());
        m_rect = m_rect.united(r);
    }

    void StandardArrowNet::setOutputPosition(const QPointF& pos)
    {
        mArrowNet.add_output(pos);
        QRectF r = mArrowNet.shape().boundingRect();
        m_shape.addPath(mArrowNet.shape());
        m_rect = m_rect.united(r);
    }

    void StandardArrowNet::paint(QPainter* painter,
                                 const QStyleOptionGraphicsItem* option,
                                 QWidget* widget)
    {
        mArrowNet.setHightlight(m_highlight);
        StandardGraphicsNet::paint(painter,option,widget);
        mArrowNet.paint(painter,option,widget);
    }

    void StandardArrowNet::set_visuals(const visuals& v)
    {
        StandardGraphicsNet::set_visuals(v);
        mArrowNet.set_visuals(v);
    }

}
