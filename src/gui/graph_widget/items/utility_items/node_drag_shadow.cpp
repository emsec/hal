#include "graph_widget/items/utility_items/node_drag_shadow.h"

#include "graph_widget/items/nodes/gates/graphics_gate.h"

#include <assert.h>

#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QPainter>
#include <QPaintEvent>
namespace hal{
QPen node_drag_shadow::s_pen;
qreal node_drag_shadow::s_lod;

QColor node_drag_shadow::s_color_pen[3];
QColor node_drag_shadow::s_color_solid[3];
QColor node_drag_shadow::s_color_translucent[3];

void node_drag_shadow::load_settings()
{
    s_color_pen[static_cast<int>(drag_cue::rejected)] = QColor(166, 31, 31, 255);
    s_color_pen[static_cast<int>(drag_cue::movable)] = QColor(48, 172, 79, 255);
    s_color_pen[static_cast<int>(drag_cue::swappable)] = QColor(37, 97, 176, 255);

    s_color_solid[static_cast<int>(drag_cue::rejected)] = QColor(166, 31, 31, 200);
    s_color_solid[static_cast<int>(drag_cue::movable)] = QColor(48, 172, 79, 200);
    s_color_solid[static_cast<int>(drag_cue::swappable)] = QColor(37, 97, 176, 200);

    s_color_translucent[static_cast<int>(drag_cue::rejected)] = QColor(166, 31, 31, 150);
    s_color_translucent[static_cast<int>(drag_cue::movable)] = QColor(48, 172, 79, 150);
    s_color_translucent[static_cast<int>(drag_cue::swappable)] = QColor(37, 97, 176, 150);

    s_pen.setCosmetic(true);
    s_pen.setJoinStyle(Qt::MiterJoin);
}

node_drag_shadow::node_drag_shadow() : QGraphicsObject()
{
    hide();

    setAcceptedMouseButtons(0);
    m_width = 100;
    m_height = 100;
}

void node_drag_shadow::start(const QPointF& posF, const QSizeF& sizeF)
{
    setPos(posF);
    set_width(sizeF.width());
    set_height(sizeF.height());
    setZValue(1);
    show();
}

void node_drag_shadow::stop()
{
    hide();
}

qreal node_drag_shadow::width() const
{
    return m_width;
}

qreal node_drag_shadow::height() const
{
    return m_height;
}

QSizeF node_drag_shadow::size() const
{
    return QSizeF(m_width, m_height);
}

void node_drag_shadow::set_width(const qreal width)
{
    m_width = width;
}

void node_drag_shadow::set_height(const qreal height)
{
    m_height = height;
}

void node_drag_shadow::set_lod(const qreal lod)
{
    s_lod = lod;
}

void node_drag_shadow::set_visual_cue(const drag_cue cue)
{
    m_cue = cue;
    update();
}

void node_drag_shadow::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    int color_index = static_cast<int>(m_cue);
    assert(color_index <= 3);

    s_pen.setColor(s_color_pen[color_index]);
    painter->setPen(s_pen);

    if (s_lod < 0.5)
    {
        painter->fillRect(QRectF(0, 0, m_width, m_height), s_color_solid[color_index]);
    }
    else
    {
        QRectF rect = QRectF(0, 0, m_width, m_height);
        painter->drawRect(rect);
        painter->fillRect(rect, s_color_translucent[color_index]);
    }
}

QRectF node_drag_shadow::boundingRect() const
{
    return QRectF(0, 0, m_width, m_height);
}

QPainterPath node_drag_shadow::shape() const
{
    QPainterPath path;
    path.addRect(QRectF(0, 0, m_width, m_height));
    return path;
}
}
