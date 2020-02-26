#include "gui/graph_widget/items/nodes/graphics_node.h"

#include "gui/graph_widget/graph_widget_constants.h"

#include <QApplication>

graphics_node::graphics_node(const hal::item_type type, const u32 id, const QString& name) : graphics_item(type, id),
    m_name(name)
{
    setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsGeometryChanges);
    //setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges | ItemIsFocusable);
    //setAcceptHoverEvents(true);
}

QRectF graphics_node::boundingRect() const
{
    return QRectF(0, 0, m_width, m_height);
}

QPainterPath graphics_node::shape() const
{
    QPainterPath path;
    path.addRect(QRectF(0, 0, m_width, m_height));
    return path;
}

void graphics_node::set_visuals(const graphics_node::visuals& v)
{
    setVisible(v.visible);

    m_color = v.main_color;
}

qreal graphics_node::width() const
{
    return m_width;
}

qreal graphics_node::height() const
{
    return m_height;
}

void graphics_node::set_name(QString name)
{
    m_name = name;
}

QVariant graphics_node::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
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
