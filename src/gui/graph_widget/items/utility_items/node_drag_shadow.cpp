#include "graph_widget/items/utility_items/node_drag_shadow.h"

#include "graph_widget/items/nodes/gates/graphics_gate.h"

#include <assert.h>

#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QPainter>
#include <QPaintEvent>

namespace hal
{
    QPen NodeDragShadow::s_pen;
    qreal NodeDragShadow::s_lod;

    QColor NodeDragShadow::s_color_pen[3];
    QColor NodeDragShadow::s_color_solid[3];
    QColor NodeDragShadow::s_color_translucent[3];

    void NodeDragShadow::load_settings()
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

    NodeDragShadow::NodeDragShadow() : QGraphicsObject()
    {
        hide();

        setAcceptedMouseButtons(0);
        m_width = 100;
        m_height = 100;
    }

    void NodeDragShadow::start(const QPointF& posF, const QSizeF& sizeF)
    {
        setPos(posF);
        set_width(sizeF.width());
        set_height(sizeF.height());
        setZValue(1);
        show();
    }

    void NodeDragShadow::stop()
    {
        hide();
    }

    qreal NodeDragShadow::width() const
    {
        return m_width;
    }

    qreal NodeDragShadow::height() const
    {
        return m_height;
    }

    QSizeF NodeDragShadow::size() const
    {
        return QSizeF(m_width, m_height);
    }

    void NodeDragShadow::set_width(const qreal width)
    {
        m_width = width;
    }

    void NodeDragShadow::set_height(const qreal height)
    {
        m_height = height;
    }

    void NodeDragShadow::set_lod(const qreal lod)
    {
        s_lod = lod;
    }

    void NodeDragShadow::set_visual_cue(const drag_cue cue)
    {
        m_cue = cue;
        update();
    }

    void NodeDragShadow::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
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

    QRectF NodeDragShadow::boundingRect() const
    {
        return QRectF(0, 0, m_width, m_height);
    }

    QPainterPath NodeDragShadow::shape() const
    {
        QPainterPath path;
        path.addRect(QRectF(0, 0, m_width, m_height));
        return path;
    }
}
