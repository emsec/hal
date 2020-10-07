#include "gui/graph_widget/items/nodes/graphics_node.h"

#include "gui/graph_widget/graph_widget_constants.h"

#include <QApplication>

namespace hal
{
    GraphicsNode::GraphicsNode(const hal::item_type type, const u32 id, const QString& name) : GraphicsItem(type, id),
        m_name(name)
    {
        setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsGeometryChanges);
        //setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges | ItemIsFocusable);
        //setAcceptHoverEvents(true);
    }

    QRectF GraphicsNode::boundingRect() const
    {
        return QRectF(0, 0, m_width, m_height);
    }

    QPainterPath GraphicsNode::shape() const
    {
        QPainterPath path;
        path.addRect(QRectF(0, 0, m_width, m_height));
        return path;
    }

    void GraphicsNode::set_visuals(const GraphicsNode::visuals& v)
    {
        setVisible(v.visible);

        m_color = v.main_color;
    }

    qreal GraphicsNode::width() const
    {
        return m_width;
    }

    qreal GraphicsNode::height() const
    {
        return m_height;
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

    void GraphicsNode::set_name(QString name)
    {
        m_name = name;
    }

    QVariant GraphicsNode::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
    {
        // NOT FUNCTIONAL, NEEDS TO USE LAYOUT DATA
        // superseded by new drag and drop function
        // if (change == ItemPositionChange)
        // {
        //     if(QApplication::mouseButtons() == Qt::LeftButton)
        //     {
        //         QPointF new_position = value.toPointF();
        //         int adjusted_x = qRound(new_position.x() / graph_widget_constants::grid_size) * graph_widget_constants::grid_size;
        //         int adjusted_y = qRound(new_position.y() / graph_widget_constants::grid_size) * graph_widget_constants::grid_size;
        //         return QPoint(adjusted_x, adjusted_y);
        //     }
        //     else
        //         return value;
        // }
        // else
            return QGraphicsItem::itemChange(change, value);
    }
}
