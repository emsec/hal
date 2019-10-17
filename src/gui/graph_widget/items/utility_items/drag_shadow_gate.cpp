#include "graph_widget/items/utility_items/drag_shadow_gate.h"

#include "graph_widget/items/graphics_gate.h"

#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include "QPainter"
#include "QPaintEvent"

drag_shadow_gate::drag_shadow_gate() : QGraphicsObject()
{
    hide();

    setAcceptedMouseButtons(0);
    m_width = 100;
    m_height = 100;
}

void drag_shadow_gate::start(const QPointF& posF, const QSizeF& sizeF)
{
    setPos(posF);
    set_width(sizeF.width());
    set_height(sizeF.height());
    setZValue(1);
    show();
}

void drag_shadow_gate::stop()
{
    hide();
}

qreal drag_shadow_gate::width() const
{
    return m_width;
}

qreal drag_shadow_gate::height() const
{
    return m_height;
}

void drag_shadow_gate::set_width(const qreal width)
{
    m_width = width;
}

void drag_shadow_gate::set_height(const qreal height)
{
    m_height = height;
}

void drag_shadow_gate::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->save();

    painter->fillRect(QRectF(0, 0, m_width, m_height), QColor(0, 255, 0, 200));
}

QRectF drag_shadow_gate::boundingRect() const
{
    return QRectF(0, 0, m_width, m_height);
}

QPainterPath drag_shadow_gate::shape() const
{
    QPainterPath path;
    path.addRect(QRectF(0, 0, m_width, m_height));
    return path;
}
