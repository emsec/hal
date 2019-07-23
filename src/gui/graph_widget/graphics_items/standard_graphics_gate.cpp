#include "graph_widget/graphics_items/standard_graphics_gate.h"

#include "core/log.h"

#include "netlist/gate.h"

#include "gui/graph_widget/graph_widget_constants.h"
#include "gui/gui_globals.h"

#include <QFont>
#include <QFontMetricsF>
#include <QPainter>
#include <QPen>
#include <QStyle>
#include <QStyleOptionGraphicsItem>

static const qreal baseline = 1;

// MAKE FIXED VALUES CONST
qreal standard_graphics_gate::s_alpha;

QPen standard_graphics_gate::s_pen;

QColor standard_graphics_gate::s_default_main_color;
QColor standard_graphics_gate::s_text_color;

QFont standard_graphics_gate::s_name_font;
QFont standard_graphics_gate::s_type_font;
QFont standard_graphics_gate::s_pin_font;

qreal standard_graphics_gate::s_name_font_height;
qreal standard_graphics_gate::s_type_font_height;

qreal standard_graphics_gate::s_color_bar_height = 30;

qreal standard_graphics_gate::s_pin_inner_horizontal_spacing = 12;
qreal standard_graphics_gate::s_pin_outer_horizontal_spacing = 2.4;

qreal standard_graphics_gate::s_pin_inner_vertical_spacing = 1.2;
qreal standard_graphics_gate::s_pin_outer_vertical_spacing = 0.6;
qreal standard_graphics_gate::s_pin_upper_vertical_spacing = 2;
qreal standard_graphics_gate::s_pin_lower_vertical_spacing = 1.8;

qreal standard_graphics_gate::s_pin_font_height;
qreal standard_graphics_gate::s_pin_font_ascent;
qreal standard_graphics_gate::s_pin_font_descent;
qreal standard_graphics_gate::s_pin_font_baseline;

qreal standard_graphics_gate::s_inner_name_type_spacing = 1.2;
qreal standard_graphics_gate::s_outer_name_type_spacing = 3;

qreal standard_graphics_gate::s_first_pin_y;
qreal standard_graphics_gate::s_pin_y_stride;

void standard_graphics_gate::load_settings()
{
    s_pen.setCosmetic(true);
    s_pen.setJoinStyle(Qt::MiterJoin);

    s_default_main_color = QColor(96, 110, 112);
    //s_default_main_color = QColor(38, 70, 90, 255);
    //s_default_main_color = QColor(191, 211, 202);
    //s_default_main_color = QColor(138, 160, 161);
    s_text_color = QColor(160, 160, 160);

    QFont font = QFont("Iosevka");
    font.setPixelSize(graph_widget_constants::font_size);

    s_name_font = font;
    s_type_font = font;
    s_pin_font = font;

    QFontMetricsF name_fm(s_name_font);
    s_name_font_height = name_fm.height();

    QFontMetricsF type_fm(s_type_font);
    s_type_font_height = type_fm.height();

    QFontMetricsF pin_fm(s_pin_font);
    s_pin_font_height = pin_fm.height();
    s_pin_font_ascent = pin_fm.ascent();
    s_pin_font_descent = pin_fm.descent();
    s_pin_font_baseline = 1;

    s_first_pin_y = s_color_bar_height + s_pin_upper_vertical_spacing + s_pin_font_height;
    s_pin_y_stride = s_pin_font_height + s_pin_inner_vertical_spacing;
}

void standard_graphics_gate::update_alpha()
{
    if (s_lod <= graph_widget_constants::gate_max_lod)
    {
        const qreal difference = graph_widget_constants::gate_max_lod - graph_widget_constants::gate_min_lod;

        s_alpha = 1 - (s_lod - graph_widget_constants::gate_min_lod) / difference;
    }
    else
        s_alpha = 0;
}

standard_graphics_gate::standard_graphics_gate(const std::shared_ptr<const gate> g, const bool adjust_size_to_grid) : graphics_gate(g)
{
    m_color = s_default_main_color;
    format(adjust_size_to_grid);
}

void standard_graphics_gate::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(widget);

    if (s_lod < graph_widget_constants::gate_min_lod)
    {
        painter->fillRect(QRectF(0, 0, m_width, m_height), (option->state & QStyle::State_Selected) ? s_selection_color : m_color);
        return;
    }
    else
    {
//        QLinearGradient gradient(QPointF(0, 0), QPointF(m_width, s_color_bar_height));
//        gradient.setColorAt(0, QColor(28, 85, 0, 255));
//        gradient.setColorAt(1, QColor(0, 0, 0, 200));

        painter->fillRect(QRectF(0, 0, m_width, s_color_bar_height), m_color);
        painter->fillRect(QRectF(0, s_color_bar_height, m_width, m_height - s_color_bar_height), QColor(0, 0, 0, 200));

        s_pen.setColor(s_text_color);
        painter->setPen(s_pen);

        painter->setFont(s_name_font);
        painter->drawText(m_name_position, m_name);
        painter->setFont(s_type_font);
        painter->drawText(m_type_position, m_type);
        painter->setFont(s_pin_font);

        for (int i = 0; i < m_input_pins.size(); ++i)
            painter->drawText(m_input_pin_positions.at(i), m_input_pins.at(i));

        for (int i = 0; i < m_output_pins.size(); ++i)
            painter->drawText(m_output_pin_positions.at(i), m_output_pins.at(i));

        if (g_selection_relay.m_focus_type == selection_relay::item_type::gate)
            if (g_selection_relay.m_focus_id == m_id)
            {
                s_pen.setColor(s_selection_color);
                painter->setPen(s_pen);

                switch (g_selection_relay.m_subfocus)
                {
                case selection_relay::subfocus::none:
                {
                    painter->setFont(s_name_font);
                    painter->drawText(m_name_position, m_name);
                    painter->setFont(s_type_font);
                    painter->drawText(m_type_position, m_type);
                    painter->setFont(s_pin_font);

                    for (int i = 0; i < m_input_pins.size(); ++i)
                        painter->drawText(m_input_pin_positions.at(i), m_input_pins.at(i));

                    for (int i = 0; i < m_output_pins.size(); ++i)
                        painter->drawText(m_output_pin_positions.at(i), m_output_pins.at(i));

                    break;
                }
                case selection_relay::subfocus::left:
                {
                    painter->drawText(m_input_pin_positions.at(g_selection_relay.m_subfocus_index), m_input_pins.at(g_selection_relay.m_subfocus_index));
                    break;
                }
                case selection_relay::subfocus::right:
                {
                    painter->drawText(m_output_pin_positions.at(g_selection_relay.m_subfocus_index), m_output_pins.at(g_selection_relay.m_subfocus_index));
                    break;
                }
                }
            }

        if (s_lod < graph_widget_constants::gate_max_lod)
        {
            QColor fade = m_color;
            fade.setAlphaF(s_alpha);

            //painter->fillRect(QRect(0, 0, m_width, m_height), fade);
            painter->fillRect(QRectF(0, s_color_bar_height, m_width, m_height - s_color_bar_height), fade);
        }

        if (option->state & QStyle::State_Selected)
        {
            s_pen.setColor(s_selection_color);
            s_pen.setCosmetic(true);
            painter->setPen(s_pen);
            s_pen.setCosmetic(false);
            bool original_antialiasing_value = painter->renderHints().testFlag(QPainter::Antialiasing);
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->setBrush(QBrush());
            painter->drawRect(boundingRect());
            //painter->drawRect(boundingRect().marginsAdded(QMarginsF(0.5, 0.5, 0.5, 0.5)));
            painter->setRenderHint(QPainter::Antialiasing, original_antialiasing_value);
        }
    }
}

QPointF standard_graphics_gate::get_input_scene_position(const u32 net_id, const QString& pin_type) const
{
    Q_UNUSED(net_id)

    int index = m_input_pins.indexOf(pin_type);

    if (index == -1)
    {
        log_error("gui graph", "input pin type not found.");
        return mapToScene(QPointF(0, 0));
    }

    qreal y = s_color_bar_height + s_pin_upper_vertical_spacing;
    y += index * (s_pin_font_height + s_pin_inner_vertical_spacing);
    y += s_pin_font_height / 2;

    return mapToScene(QPointF(0, y));
}

QPointF standard_graphics_gate::get_output_scene_position(const u32 net_id, const QString& pin_type) const
{
    Q_UNUSED(net_id)

    int index = m_output_pins.indexOf(pin_type);

    if (index == -1)
    {
        log_error("gui graph", "output pin type not found.");
        return mapToScene(QPointF(0, 0));
    }

    qreal y = s_color_bar_height + s_pin_upper_vertical_spacing;
    y += index * (s_pin_font_height + s_pin_inner_vertical_spacing);
    y += s_pin_font_height / 2;

    return mapToScene(QPointF(m_width, y));
}

void standard_graphics_gate::set_visuals(const graphics_node::visuals& v)
{
    m_color = v.main_color;
}

void standard_graphics_gate::format(const bool& adjust_size_to_grid)
{
    QFontMetricsF name_fm(s_name_font);
    qreal name_width = name_fm.width(m_name);

    QFontMetricsF type_fm(s_type_font);
    qreal type_width = type_fm.width(m_type);

    QFontMetricsF pin_fm(s_pin_font);
    qreal max_pin_width = 0;

    for (const QString& input_pin : m_input_pins)
    {
        qreal width = pin_fm.width(input_pin);
        if (width > max_pin_width)
            max_pin_width = width;
    }

    for (const QString& output_pin : m_output_pins)
    {
        qreal width = pin_fm.width(output_pin);
        if (width > max_pin_width)
            max_pin_width = width;
    }

    qreal total_input_pin_height = 0;

    if (!m_input_pins.isEmpty())
    {
        total_input_pin_height = m_input_pins.size() * s_pin_font_height +
                                (m_input_pins.size() - 1) * s_pin_inner_vertical_spacing +
                                 s_pin_upper_vertical_spacing + s_pin_lower_vertical_spacing;
    }

    qreal total_output_pin_height = 0;

    if (!m_output_pins.isEmpty())
    {
        total_output_pin_height = m_output_pins.size() * s_pin_font_height +
                                 (m_output_pins.size() - 1) * s_pin_inner_vertical_spacing +
                                  s_pin_upper_vertical_spacing + s_pin_lower_vertical_spacing;
    }

    qreal max_pin_height = std::max(total_input_pin_height, total_output_pin_height);
    qreal min_body_height = s_name_font_height + s_type_font_height + s_inner_name_type_spacing + 2 * s_outer_name_type_spacing;

    m_width = max_pin_width * 2 + s_pin_inner_horizontal_spacing * 2 + s_pin_outer_horizontal_spacing * 2 + std::max(name_width, type_width);
    m_height = std::max(max_pin_height, min_body_height) + s_color_bar_height;

    if (adjust_size_to_grid)
    {
        if (m_width / graph_widget_constants::grid_size)
        {
            int floored = m_width;
            int quotient = floored / graph_widget_constants::grid_size;
            m_width = (quotient + 1) * graph_widget_constants::grid_size;
        }

        if (m_height / graph_widget_constants::grid_size)
        {
            int floored = m_height;
            int quotient = floored / graph_widget_constants::grid_size;
            m_height = (quotient + 1) * graph_widget_constants::grid_size;
        }
    }

    m_name_position.setX(m_width / 2 - name_width / 2);
    m_name_position.setY(std::max(m_height / 2 - s_name_font_height / 2 - s_inner_name_type_spacing / 2, s_color_bar_height + s_outer_name_type_spacing + s_name_font_height));

    m_type_position.setX(m_width / 2 - type_width / 2);
    m_type_position.setY(m_name_position.y() + s_type_font_height + s_inner_name_type_spacing / 2);

    qreal y = s_color_bar_height + s_pin_upper_vertical_spacing + s_pin_font_ascent + baseline;

    // QVECTOR PROBABLY UNNECESSARY HERE, VALUES CAN BE CALCULATED INSIDE PAINT METHOD
    for (int i = 0; i < m_input_pins.size(); i++)
    {
        m_input_pin_positions.append(QPointF(s_pin_outer_horizontal_spacing, y));
        y += (s_pin_font_height + s_pin_inner_vertical_spacing);
    }

    y = s_color_bar_height + s_pin_upper_vertical_spacing + s_pin_font_ascent + baseline;

    for (const QString& output_pin : m_output_pins)
    {
        qreal x = m_width - (pin_fm.size(0, output_pin).rwidth() + s_pin_outer_horizontal_spacing);
        m_output_pin_positions.append(QPointF(x, y));
        y += (s_pin_font_height + s_pin_inner_vertical_spacing);
    }
}
