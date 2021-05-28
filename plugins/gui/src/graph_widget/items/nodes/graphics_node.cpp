#include "gui/graph_widget/items/nodes/graphics_node.h"

#include "gui/graph_widget/graph_widget_constants.h"

#include <QApplication>

namespace hal
{
    GraphicsNode::GraphicsNode(const ItemType type, const u32 id, const QString& name)
        : GraphicsItem(type, id)
    {
        mNodeText[0] = name;
        setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsGeometryChanges);
        //setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges | ItemIsFocusable);
        //setAcceptHoverEvents(true);
    }

    QRectF GraphicsNode::boundingRect() const
    {
        return QRectF(0, 0, mWidth, mHeight);
    }

    QPainterPath GraphicsNode::shape() const
    {
        QPainterPath path;
        path.addRect(QRectF(0, 0, mWidth, mHeight));
        return path;
    }

    void GraphicsNode::setVisuals(const GraphicsNode::Visuals& v)
    {
        setVisible(v.mVisible);

        mColor = v.mMainColor;
    }

    qreal GraphicsNode::width() const
    {
        return mWidth;
    }

    qreal GraphicsNode::height() const
    {
        return mHeight;
    }

    QList<u32> GraphicsNode::inputNets() const
    {
        if (mInputByNet.isEmpty()) return QList<u32>();
        int n = mInputByNet.size();
        QVector<u32> retval(n);
        for (auto it=mInputByNet.constBegin(); it!=mInputByNet.constEnd(); ++it)
        {
            Q_ASSERT(it.value() < n);
            retval[it.value()] = it.key();
        }
        return retval.toList();
    }

    QList<u32> GraphicsNode::outputNets() const
    {
        if (mOutputByNet.isEmpty()) return QList<u32>();
        int n = mOutputByNet.size();
        QVector<u32> retval(n);
        for (auto it=mOutputByNet.constBegin(); it!=mOutputByNet.constEnd(); ++it)
        {
            Q_ASSERT(it.value() < n);
            retval[it.value()] = it.key();
        }
        return retval.toList();
    }

    int GraphicsNode::inputByNet(u32 netId) const
    {
        return mInputByNet.value(netId,-1);
    }

    int GraphicsNode::outputByNet(u32 netId) const
    {
        return mOutputByNet.value(netId,-1);
    }

    void GraphicsNode::set_name(const QString &name)
    {
        mNodeText[0] = name;
    }
}
