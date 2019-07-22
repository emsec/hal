#include "graph_widget/graphics_items/minimal_graphics_gate.h"

#include "core/log.h"

#include "netlist/gate.h"

#include <QFont>
#include <QFontMetricsF>
#include <QPainter>
#include <QPen>
#include <QStyle>
#include <QStyleOptionGraphicsItem>

QPen minimal_graphics_gate::s_pen;

QFont minimal_graphics_gate::s_name_font;
QFont minimal_graphics_gate::s_type_font;
QFont minimal_graphics_gate::s_pin_font;

qreal minimal_graphics_gate::s_name_font_height;
qreal minimal_graphics_gate::s_type_font_height;

qreal minimal_graphics_gate::s_pin_inner_horizontal_spacing;
qreal minimal_graphics_gate::s_pin_outer_horizontal_spacing;

qreal minimal_graphics_gate::s_pin_inner_vertical_spacing;
qreal minimal_graphics_gate::s_pin_outer_vertical_spacing;
qreal minimal_graphics_gate::s_pin_upper_vertical_spacing;
qreal minimal_graphics_gate::s_pin_lower_vertical_spacing;

qreal minimal_graphics_gate::s_pin_font_height;
qreal minimal_graphics_gate::s_pin_font_ascent;
qreal minimal_graphics_gate::s_pin_font_descent;
qreal minimal_graphics_gate::s_pin_font_baseline;

qreal minimal_graphics_gate::s_inner_name_type_spacing;
qreal minimal_graphics_gate::s_outer_name_type_spacing;

void minimal_graphics_gate::load_settings()
{
    // ADD STYLESHEET SUPPORT
    s_pin_outer_horizontal_spacing = 1.2;
    s_pin_inner_horizontal_spacing = 12;

    s_pin_inner_vertical_spacing = 1.2;
    s_pin_outer_vertical_spacing = 0.6;
    s_pin_upper_vertical_spacing = 0.6;
    s_pin_lower_vertical_spacing = 1.8;

    s_inner_name_type_spacing = 1.2;
    s_outer_name_type_spacing = 3;

    QFont font = QFont("Iosevka");
    font.setPixelSize(6);
    s_pin_font = font;
    QFontMetricsF pin_fm(s_pin_font);
    s_pin_font_height   = pin_fm.height();
    s_pin_font_ascent   = pin_fm.ascent();
    s_pin_font_descent  = pin_fm.descent();
    s_pin_font_baseline = 1;
    font.setBold(true);
    s_name_font = font;
    s_type_font = font;

    QFontMetricsF name_fm(s_name_font);
    s_name_font_height = name_fm.height();

    QFontMetricsF type_fm(s_type_font);
    s_type_font_height = type_fm.height();

    s_pen.setCosmetic(true);
    //s_pen.setWidth(1);
    s_pen.setJoinStyle(Qt::MiterJoin);
}

minimal_graphics_gate::minimal_graphics_gate(const std::shared_ptr<const gate> g) : graphics_gate(g)
{
    format();
}

QPainterPath minimal_graphics_gate::shape() const
{
    QPainterPath path;
    path.addRect(QRectF(0, 0, m_width, m_height));
    return path;
}

void minimal_graphics_gate::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(widget);

    QColor color = (option->state & QStyle::State_Selected) ? s_selection_color : m_color;
    //    if (option->state & QStyle::State_MouseOver)
    //        color = color.light(150);

    s_pen.setColor(color);

    //    const qreal lod = option->levelOfDetailFromTransform(painter->worldTransform());

    if (s_lod < 0.5)
    {
        painter->setPen(s_pen);
        painter->drawRect(QRect(0, 0, m_width, m_height));
        return;
    }
    else
    {
        painter->setPen(s_pen);
        painter->drawRect(QRectF(0, 0, m_width, m_height));

        painter->setFont(s_name_font);
        painter->drawText(m_name_position, m_name);
        painter->setFont(s_type_font);
        painter->drawText(m_type_position, m_type);

        painter->setFont(s_pin_font);
        int i = 0;
        for (const QString& input_pin : m_input_pins)
        {
            painter->drawText(m_input_pin_positions.at(i), input_pin);
            i++;
        }

        i = 0;
        for (const QString& output_pin : m_output_pins)
        {
            painter->drawText(m_output_pin_positions.at(i), output_pin);
            i++;
        }
        return;
    }

    //    const int lod = option->levelOfDetailFromTransform(painter->worldTransform()) * 10;
}

QPointF minimal_graphics_gate::get_input_scene_position(const u32 net_id, const QString& pin_type) const
{
    Q_UNUSED(net_id)

    int index = m_input_pins.indexOf(pin_type);

    if (index == -1)
    {
        log_error("gui graph", "type not found.");
        return mapToScene(QPointF(0, 0));
    }

    qreal y = s_pin_upper_vertical_spacing;

    if (index > 0)
        y += index * s_pin_inner_vertical_spacing + index * s_pin_font_height;

    y += s_pin_font_height / 2;

    return mapToScene(QPointF(0, y));
}

QPointF minimal_graphics_gate::get_output_scene_position(const u32 net_id, const QString& pin_type) const
{
    Q_UNUSED(net_id)

    int index = m_output_pins.indexOf(pin_type);

    if (index == -1)
    {
        log_error("gui graph", "type not found.");
        return mapToScene(QPointF(0, 0));
    }

    qreal y = s_pin_upper_vertical_spacing;

    if (index > 0)
        y += index * s_pin_inner_vertical_spacing + index * s_pin_font_height;

    y += s_pin_font_height / 2;

    return mapToScene(QPointF(m_width, y));
}

void minimal_graphics_gate::set_visuals(const graphics_node::visuals& v)
{

}

void minimal_graphics_gate::format()
{
    QFontMetricsF name_fm(s_name_font);
    qreal name_width = name_fm.width(m_name);

    QFontMetricsF type_fm(s_type_font);
    qreal type_width = type_fm.width(m_type);

    QFontMetricsF pin_fm(s_pin_font);

    int max_input_pin_width = 0;
    for (QString input_pin : m_input_pins)
    {
        qreal width = pin_fm.width(input_pin);
        if (width > max_input_pin_width)
            max_input_pin_width = width;
    }

    int max_output_pin_width = 0;
    for (QString output_pin : m_output_pins)
    {
        qreal width = pin_fm.width(output_pin);
        if (width > max_output_pin_width)
            max_output_pin_width = width;
    }

    qreal outer_spacing          = m_input_pins.size() ? s_pin_upper_vertical_spacing + s_pin_lower_vertical_spacing : 0;
    qreal total_input_pin_height = m_input_pins.size() * s_pin_font_height + (m_input_pins.size() - 1) * s_pin_inner_vertical_spacing + outer_spacing;

    outer_spacing                 = m_output_pins.size() ? s_pin_upper_vertical_spacing + s_pin_lower_vertical_spacing : 0;
    qreal total_output_pin_height = m_output_pins.size() * s_pin_font_height + (m_output_pins.size() - 1) * s_pin_inner_vertical_spacing + outer_spacing;

    qreal max_pin_height = std::max(total_input_pin_height, total_output_pin_height);

    //qreal default_height = name_fm.height() + type_fm.height() + s_inner_name_type_spacing + 2 * s_outer_name_type_spacing;
    qreal default_height = s_name_font_height + s_type_font_height + s_inner_name_type_spacing + 2 * s_outer_name_type_spacing;

    m_width  = std::max(max_input_pin_width, max_output_pin_width) * 2 + s_pin_inner_horizontal_spacing * 2 + s_pin_outer_horizontal_spacing * 2 + std::max(name_width, type_width);
    m_height = std::max(max_pin_height, default_height);

    m_name_position.setX(m_width / 2.0 - name_width / 2.0);
    m_name_position.setY(m_height / 2.0 - s_inner_name_type_spacing / 2);

    m_type_position.setX(m_width / 2.0 - type_width / 2.0);
    //m_type_position.setY(m_name_position.y() + type_fm.height() + s_inner_name_type_spacing / 2);
    m_type_position.setY(m_name_position.y() + s_type_font_height + s_inner_name_type_spacing / 2);

    qreal y = s_pin_upper_vertical_spacing + s_pin_font_height;

    for (int i = 0; i < m_input_pins.size(); i++)
    {
        m_input_pin_positions.append(QPointF(s_pin_outer_horizontal_spacing, y));
        y += (s_pin_font_height + s_pin_inner_vertical_spacing);
    }

    y = s_pin_upper_vertical_spacing + s_pin_font_height;

    for (QString output_pin : m_output_pins)
    {
        qreal x = m_width - (pin_fm.size(0, output_pin).rwidth() + s_pin_outer_horizontal_spacing);
        m_output_pin_positions.append(QPointF(x, y));
        y += (s_pin_font_height + s_pin_inner_vertical_spacing);
    }
}
