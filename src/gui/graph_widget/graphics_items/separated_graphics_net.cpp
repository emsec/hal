#include "graph_widget/graphics_items/separated_graphics_net.h"
#include <QPainter>
#include <QPen>
#include <QStyleOptionGraphicsItem>

qreal separated_graphics_net::s_wire_length;
qreal separated_graphics_net::s_text_offset;

QFont separated_graphics_net::s_font;
qreal separated_graphics_net::s_font_height;

void separated_graphics_net::load_settings()
{
    s_wire_length = 2;
    s_text_offset = 0.2;

    s_font = QFont("Iosevka");
    s_font.setPixelSize(1);
    //QFontMetricsF fm(s_font);
    //s_font_height = fm.height();
    s_font_height = 1;
}

separated_graphics_net::separated_graphics_net(QString text, std::shared_ptr<net> n) : graphics_net(n)
{
    m_text = text;
    QFontMetricsF fm(s_font);
    m_text_width = fm.width(m_text);
    draw_output  = false;
}

void separated_graphics_net::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(widget);

    //    const qreal lod = option->levelOfDetailFromTransform(painter->worldTransform());

    if (s_lod > 0.5)
    {
        s_pen.setColor((option->state & QStyle::State_Selected) ? s_selection_color : m_color);
        painter->setPen(s_pen);
        painter->setFont(s_font);

        if (draw_output)
        {
            painter->drawLine(QPointF(0, 0), QPointF(s_wire_length, 0));
            painter->drawText(QPointF(s_wire_length + s_text_offset, s_font_height / 2), m_text);
        }

        for (QPointF& position : m_input_wires)
        {
            QPointF to(position.x() - s_wire_length, position.y());
            painter->drawLine(position, to);
            qreal horizontal_offset = s_text_offset + m_text_width;
            qreal vertical_offset   = s_font_height / 2;
            painter->drawText(QPointF(to.x() - horizontal_offset, to.y() + vertical_offset), m_text);
        }

        //        s_pen.setColor(Qt::green);
        //        painter->setPen(s_pen);
        //        painter->drawPath(m_shape);
    }
}

void separated_graphics_net::finalize()
{
    m_rect = m_shape.boundingRect();
    m_rect.adjust(-1, -1, 1, 1);
}

void separated_graphics_net::add_output()
{
    if (draw_output)
        return;

    draw_output = true;

    m_shape.moveTo(QPointF(0, -s_stroke_width / 2));
    m_shape.lineTo(QPointF(m_shape.currentPosition().x() + s_wire_length + s_text_offset, m_shape.currentPosition().y()));
    m_shape.lineTo(QPointF(m_shape.currentPosition().x(), m_shape.currentPosition().y() - QFontMetricsF(s_font).height() / 2 + s_stroke_width / 2));
    m_shape.lineTo(QPointF(m_shape.currentPosition().x() + m_text_width, m_shape.currentPosition().y()));
    m_shape.lineTo(QPointF(m_shape.currentPosition().x(), m_shape.currentPosition().y() + QFontMetricsF(s_font).height()));
    m_shape.lineTo(QPointF(m_shape.currentPosition().x() - m_text_width, m_shape.currentPosition().y()));
    m_shape.lineTo(QPointF(m_shape.currentPosition().x(), m_shape.currentPosition().y() - QFontMetricsF(s_font).height() / 2 + s_stroke_width / 2));
    m_shape.lineTo(QPointF(m_shape.currentPosition().x() - s_wire_length - s_text_offset, m_shape.currentPosition().y()));
    m_shape.closeSubpath();
}

void separated_graphics_net::add_input(const QPointF& scene_position)
{
    QPointF mapped_position = mapFromScene(scene_position);
    m_input_wires.append(mapped_position);

    m_shape.moveTo(QPointF(mapped_position.x(), mapped_position.y() - s_stroke_width / 2));
    m_shape.lineTo(QPointF(m_shape.currentPosition().x() - s_wire_length - s_text_offset, m_shape.currentPosition().y()));
    m_shape.lineTo(QPointF(m_shape.currentPosition().x(), m_shape.currentPosition().y() - QFontMetricsF(s_font).height() / 2 + s_stroke_width / 2));
    m_shape.lineTo(QPointF(m_shape.currentPosition().x() - m_text_width, m_shape.currentPosition().y()));
    m_shape.lineTo(QPointF(m_shape.currentPosition().x(), m_shape.currentPosition().y() + QFontMetricsF(s_font).height()));
    m_shape.lineTo(QPointF(m_shape.currentPosition().x() + m_text_width, m_shape.currentPosition().y()));
    m_shape.lineTo(QPointF(m_shape.currentPosition().x(), m_shape.currentPosition().y() - QFontMetricsF(s_font).height() / 2 + s_stroke_width / 2));
    m_shape.lineTo(QPointF(m_shape.currentPosition().x() + s_wire_length + s_text_offset, m_shape.currentPosition().y()));
    m_shape.closeSubpath();
}
