#include "graph_widget/graphics_items/utility_items/gate_navigation_popup.h"

#include "graph_widget/graphics_items/graphics_gate.h"

#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include "QPainter"
#include "QPaintEvent"

gate_navigation_popup::gate_navigation_popup(type t, QGraphicsItem* parent) : QGraphicsObject(parent),
    m_x_animation(new QPropertyAnimation(this, "x", this)),
    m_opacity_animation(new QPropertyAnimation(this, "opacity", this)),
    m_arrow_animation(new QPropertyAnimation(this, "arrow_offset", this)),
    m_animation_group(new QParallelAnimationGroup(this)),
    m_type(t),
    m_graphics_gate(nullptr)
{
    hide();

    setAcceptedMouseButtons(0);
    m_width = 100;
    m_height = 100;

    // DO TYPE STUFF HERE
    m_x_animation->setDuration(700);
    m_x_animation->setEasingCurve(QEasingCurve::OutExpo);
    //m_x_animation->setEasingCurve(QEasingCurve::OutBack);

    m_opacity_animation->setDuration(700);
    m_opacity_animation->setStartValue(0);
    m_opacity_animation->setEndValue(1);
    m_opacity_animation->setEasingCurve(QEasingCurve::Linear);

    m_arrow_animation->setDuration(700);
    m_arrow_animation->setStartValue(0);
    m_arrow_animation->setEndValue(100);
    m_arrow_animation->setLoopCount(-1);
    m_arrow_animation->setEasingCurve(QEasingCurve::Linear);

    m_animation_group->addAnimation(m_x_animation);
    m_animation_group->addAnimation(m_opacity_animation);
    m_animation_group->addAnimation(m_arrow_animation);
}

void gate_navigation_popup::start(const QPointF& final_position)
{
    setOpacity(0);
    show();
    setPos(final_position.x() + 80, final_position.y());
    setZValue(1);
    m_x_animation->setStartValue(final_position.x() + 80);
    m_x_animation->setEndValue(final_position.x());
    m_animation_group->start();
}

void gate_navigation_popup::stop()
{
    hide();
    m_animation_group->stop();
}

qreal gate_navigation_popup::width() const
{
    return m_width;
}

qreal gate_navigation_popup::height() const
{
    return m_height;
}

void gate_navigation_popup::set_width(const qreal width)
{
    m_width = width;
}

void gate_navigation_popup::set_height(const qreal height)
{
    m_height = height;
}

int gate_navigation_popup::arrow_offset() const
{
    return m_arrow_offset;
}

void gate_navigation_popup::set_arrow_offset(const int offset)
{
    m_arrow_offset = offset;
    update(QRectF(0, 0, m_width, m_height)); // CONDITIONAL UPDATE ALL VIEWS INSTEAD ???
}

void gate_navigation_popup::set_graphics_gate(graphics_gate* g)
{
    m_graphics_gate = g;
}

void gate_navigation_popup::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->save();

    painter->fillRect(QRect(0, 0, m_width, m_height), QColor(0, 0, 0, 200));

    QPen pen;
    pen.setCosmetic(true);
    pen.setColor(QColor(240, 173, 0));
    painter->setPen(pen);

    qreal bar_height = 30;
    qreal arrow_width = 10;
    qreal arrow_length = 30;

    QPainterPath path;
    path.lineTo(QPointF(-(arrow_length - arrow_width), (bar_height / 2)));
    path.lineTo(QPointF(0, bar_height));
    path.lineTo(QPointF(-arrow_width, bar_height));
    path.lineTo(QPointF(-arrow_length, (bar_height / 2)));
    path.lineTo(QPointF(-arrow_width, 0));
    path.closeSubpath();

    painter->drawRect(QRectF(0, 0, m_width, m_height));

    painter->setClipRect(QRect(0, 0, m_width, m_height));
    painter->setRenderHint(QPainter::Antialiasing, true);

    painter->drawText(QRectF(10, 40, 50, 50), QString::number(m_arrow_offset));

    qreal value = (40.0 / 100.0) * m_arrow_offset;
    painter->translate(-value, 0);

    for (int x = 0; x < m_width; x += 10)
    {
        painter->fillPath(path, QColor(240, 173, 0));
        painter->translate(20, 0);
    }

    painter->restore();
}

QRectF gate_navigation_popup::boundingRect() const
{
    return QRectF(0, 0, m_width, m_height);
}

QPainterPath gate_navigation_popup::shape() const
{
    QPainterPath path;
    path.addRect(QRectF(0, 0, m_width, m_height));
    return path;
}
