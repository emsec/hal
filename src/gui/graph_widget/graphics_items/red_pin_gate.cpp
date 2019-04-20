#include "graph_widget/graphics_items/red_pin_gate.h"
#include "core/log.h"
#include <QDebug>
#include <QPainter>

red_pin_gate::red_pin_gate(std::shared_ptr<gate> g) : graphics_gate(g)
{
    format();
}

void red_pin_gate::load_settings()
{
    s_pin_font      = QFont("Iosevka", 1);
    s_name_font     = QFont("Iosevka", 1, QFont::Bold);
    s_default_color = QColor(Qt::white);
    s_pen           = QPen(s_default_color);
    s_pen.setCosmetic(true);
    s_pen.setWidthF(1.5);
    s_pen.setJoinStyle(Qt::MiterJoin);

    s_drawn_pin_height   = 1;
    s_overall_pin_height = 2.0 * s_drawn_pin_height;
    s_overall_pin_width  = 1.5;

    s_pin_name_rect_stretching = 0.5;

    s_name_rect_stretching = 1;
}

void red_pin_gate::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setPen(s_pen);
    painter->setFont(s_pin_font);

    //painter->fillRect(QRectF(0,0,m_width, m_height), Qt::red);

    if (isSelected())
    {
        painter->fillRect(QRectF(s_overall_pin_width, 0, m_width - 2 * s_overall_pin_width, m_height), Qt::red);
    }
    if (s_lod < 1.5)
    {
        painter->drawRect(0, 0, m_width, m_height);
        return;
    }

    //painter->drawRect(QRectF(0,0, m_width, m_height));
    painter->drawRect(m_drawn_rect);
    //painter->drawText(QRectF(0,0, m_width, m_height), Qt::AlignCenter, m_name);
    painter->drawText(m_name_rect, Qt::AlignCenter, m_name);
    //qDebug() << "Name Rect: " << m_name_rect;

    for (QPointF pin_point : m_input_pin_positions)
        painter->fillRect(QRectF(pin_point.x(), pin_point.y(), s_overall_pin_width, s_drawn_pin_height), Qt::red);

    for (QPointF pin_point : m_output_pin_positions)
        painter->fillRect(QRectF(pin_point.x(), pin_point.y(), s_overall_pin_width, s_drawn_pin_height), Qt::red);

    for (int i = 0; i < m_input_pins.size(); i++)
    {
        painter->drawRect(m_input_pin_name_rects.at(i));
        painter->drawText(m_input_pin_name_rects.at(i), Qt::AlignCenter, m_input_pins.at(i));
    }
    for (int i = 0; i < m_output_pins.size(); i++)
    {
        painter->drawRect(m_output_pin_name_rects.at(i));
        painter->drawText(m_output_pin_name_rects.at(i), Qt::AlignCenter, m_output_pins.at(i));
    }
}

QPointF red_pin_gate::get_input_pin_scene_position(QString type)
{
    int index = m_input_pins.indexOf(type);
    if (index == -1)
    {
        log_error("gui graph", "type not found");
        return mapToScene(QPointF(0, 0));
    }

    //s_drawn_pin_height = offset
    return mapToScene(QPointF(0, index * s_overall_pin_height + s_drawn_pin_height));
}

QPointF red_pin_gate::get_output_pin_scene_position(QString type)
{
    int index = m_output_pins.indexOf(type);
    if (index == -1)
    {
        log_error("gui graph", "type not found");
        return mapToScene(QPointF(0, 0));
    }
    return mapToScene(QPointF(m_width, index * s_overall_pin_height + s_drawn_pin_height));
}

void red_pin_gate::format()
{
    QFontMetricsF pin_mf(s_pin_font);
    QFontMetricsF name_mf(s_name_font);

    m_input_pin_name_width  = 0;
    m_output_pin_name_width = 0;

    for (auto& pin_name : m_input_pins)
        m_input_pin_name_width = std::max(m_input_pin_name_width, pin_mf.width(pin_name));
    for (auto& pin_name : m_output_pins)
        m_output_pin_name_width = std::max(m_output_pin_name_width, pin_mf.width(pin_name));

    //qDebug() << "Name: " << name_mf.width(m_name) << "Input-Pin Width: " << m_input_pin_name_width;
    m_width = 2 * s_overall_pin_width + m_input_pin_name_width + name_mf.width(m_name) + m_output_pin_name_width + 2 * s_pin_name_rect_stretching + s_name_rect_stretching;

    qreal pin_height = std::max(m_input_pins.size(), m_output_pins.size()) * s_overall_pin_height;
    //m_height = pin_height > name_mf.height() ? pin_height : name_mf.height();
    m_height = std::max(pin_height, name_mf.height());

    m_drawn_rect = QRectF(s_overall_pin_width, 0, m_input_pin_name_width + name_mf.width(m_name) + m_output_pin_name_width + 2 * s_pin_name_rect_stretching + s_name_rect_stretching, m_height);
    m_name_rect  = QRectF(s_overall_pin_width + m_input_pin_name_width + s_pin_name_rect_stretching, 0, name_mf.width(m_name) + s_name_rect_stretching, m_height);

    //Calculate Pin-Positions
    qreal offset = 0.25 * s_overall_pin_height;
    for (int i = 0; i < m_input_pins.size(); i++)
    {
        m_input_pin_positions.append(QPointF(0, i * s_overall_pin_height + offset));
        m_input_pin_name_rects.append(QRectF(s_overall_pin_width, i * s_overall_pin_height, m_input_pin_name_width + s_pin_name_rect_stretching, s_overall_pin_height));
        //qDebug() << "Point: " << QPointF(0, i*s_overall_pin_height + offset);
    }

    qreal x               = s_overall_pin_width + m_input_pin_name_width + name_mf.width(m_name) + m_output_pin_name_width + 2 * s_pin_name_rect_stretching + s_name_rect_stretching;
    qreal x_pin_name_rect = x - m_output_pin_name_width - s_pin_name_rect_stretching;
    for (int i = 0; i < m_output_pins.size(); i++)
    {
        m_output_pin_positions.append(QPointF(x, i * s_overall_pin_height + offset));
        m_output_pin_name_rects.append(QRectF(x_pin_name_rect, i * s_overall_pin_height, m_output_pin_name_width + s_pin_name_rect_stretching, s_overall_pin_height));
    }

    //    qDebug() << "Overall Size: " << QRectF(0,0, m_width, m_height);
    //    qDebug() << "Drawn Outer Recht: " << m_drawn_rect;
    //    qDebug() << "Input Name Width: " << m_input_pin_name_width;
    //    qDebug() << "Output Name Width: " << m_output_pin_name_width;
}

QFont red_pin_gate::s_name_font;
QFont red_pin_gate::s_pin_font;
QColor red_pin_gate::s_default_color;
QPen red_pin_gate::s_pen;
qreal red_pin_gate::s_overall_pin_width;
qreal red_pin_gate::s_overall_pin_height;
qreal red_pin_gate::s_pin_name_rect_stretching;
qreal red_pin_gate::s_name_rect_stretching;
qreal red_pin_gate::s_drawn_pin_height;
