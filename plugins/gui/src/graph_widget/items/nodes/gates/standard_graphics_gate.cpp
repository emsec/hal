#include "gui/graph_widget/items/nodes/gates/standard_graphics_gate.h"

#include "hal_core/netlist/gate.h"

#include "gui/graph_widget/graph_widget_constants.h"
#include "gui/gui_globals.h"

#include <QFont>
#include <QFontMetricsF>
#include <QPainter>
#include <QPen>
#include <QStyle>
#include <QStyleOptionGraphicsItem>

namespace hal{
static const qreal baseline = 1;

qreal StandardGraphicsGate::s_alpha;

QPen StandardGraphicsGate::s_pen;

QColor StandardGraphicsGate::s_text_color;

QFont StandardGraphicsGate::sTextFont[2];
QFont StandardGraphicsGate::s_pin_font;

qreal StandardGraphicsGate::sTextFontHeight[2];

qreal StandardGraphicsGate::s_color_bar_height = 30;

qreal StandardGraphicsGate::s_pin_inner_horizontal_spacing = 12;
qreal StandardGraphicsGate::s_pin_outer_horizontal_spacing = 2.4;

qreal StandardGraphicsGate::s_pin_inner_vertical_spacing = 1.2;
qreal StandardGraphicsGate::s_pin_outer_vertical_spacing = 0.6;
qreal StandardGraphicsGate::s_pin_upper_vertical_spacing = 2;
qreal StandardGraphicsGate::s_pin_lower_vertical_spacing = 1.8;

qreal StandardGraphicsGate::s_pin_font_height;
qreal StandardGraphicsGate::s_pin_font_ascent;
qreal StandardGraphicsGate::s_pin_font_descent;
qreal StandardGraphicsGate::s_pin_font_baseline;

qreal StandardGraphicsGate::s_inner_name_type_spacing = 1.2;
qreal StandardGraphicsGate::s_outer_name_type_spacing = 3;

qreal StandardGraphicsGate::s_first_pin_y;
qreal StandardGraphicsGate::s_pin_y_stride;

const int StandardGraphicsGate::sIconPadding = 3;
const QSize StandardGraphicsGate::sIconSize(s_color_bar_height - 2 * sIconPadding,
                                              s_color_bar_height - 2 * sIconPadding);
QPixmap* StandardGraphicsGate::sIconInstance = nullptr;

const QPixmap& StandardGraphicsGate::iconPixmap()
{
    if (!sIconInstance) sIconInstance
            = new QPixmap(QPixmap::fromImage(QImage(":/icons/sel_gate").scaled(sIconSize)));
    return *sIconInstance;
}


void StandardGraphicsGate::load_settings()
{
    s_pen.setCosmetic(true);
    s_pen.setJoinStyle(Qt::MiterJoin);

    s_text_color = QColor(160, 160, 160);

    QFont font = QFont("Iosevka");
    font.setPixelSize(graph_widget_constants::font_size);

    for (int iline=0; iline<2; iline++)
    {
        sTextFont[iline] = font;
        QFontMetricsF fmf(font);
        sTextFontHeight[iline] = fmf.height();
    }

    s_pin_font = font;
    QFontMetricsF pin_fm(s_pin_font);
    s_pin_font_height = pin_fm.height();
    s_pin_font_ascent = pin_fm.ascent();
    s_pin_font_descent = pin_fm.descent();
    s_pin_font_baseline = 1;

    s_first_pin_y = s_color_bar_height + s_pin_upper_vertical_spacing + s_pin_font_height;
    s_pin_y_stride = s_pin_font_height + s_pin_inner_vertical_spacing;
}

void StandardGraphicsGate::update_alpha()
{
    if (s_lod <= graph_widget_constants::gate_max_lod)
        s_alpha = 1 - (s_lod - graph_widget_constants::gate_min_lod) / (graph_widget_constants::gate_max_lod - graph_widget_constants::gate_min_lod);
    else
        s_alpha = 0;
}

StandardGraphicsGate::StandardGraphicsGate(Gate* g, const bool adjust_size_to_grid) : GraphicsGate(g)
{
    format(adjust_size_to_grid);
}

void StandardGraphicsGate::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(widget);

    if (s_lod < graph_widget_constants::gate_min_lod)
    {
        painter->fillRect(QRectF(0, 0, m_width, m_height), penColor(option->state));
        return;
    }
    else
    {
        painter->fillRect(QRectF(0, 0, m_width, s_color_bar_height), m_color);
        painter->fillRect(QRectF(0, s_color_bar_height, m_width, m_height - s_color_bar_height), QColor(0, 0, 0, 200));
//        QRectF iconRect(sIconPadding,sIconPadding,sIconSize.width(),sIconSize.height());
//        painter->fillRect(iconRect,Qt::black);
//        painter->drawPixmap(QPoint(sIconPadding,sIconPadding), iconPixmap());

        s_pen.setColor(penColor(option->state,s_text_color));
        painter->setPen(s_pen);

        for (int iline=0; iline<2; iline++)
        {
            painter->setFont(sTextFont[iline]);
            painter->drawText(mTextPosition[iline], mNodeText[iline]);
        }

        bool gateHasFocus =
                g_selection_relay->m_focus_type == SelectionRelay::ItemType::Gate
                && g_selection_relay->m_focus_id == m_id;
        int subFocusIndex = static_cast<int>(g_selection_relay->m_subfocus_index);

        painter->setFont(s_pin_font);
        s_pen.setColor(s_text_color);
        painter->setPen(s_pen);

        QPointF text_pos(s_pin_outer_horizontal_spacing, s_color_bar_height + s_pin_upper_vertical_spacing + s_pin_font_ascent + baseline);

        for (int i = 0; i < m_input_pins.size(); ++i)
        {
            if (gateHasFocus)
                if (g_selection_relay->m_subfocus == SelectionRelay::Subfocus::Left
                        && i == subFocusIndex)
                    s_pen.setColor(selectionColor());
                else
                    s_pen.setColor(s_text_color);
            else
                s_pen.setColor(penColor(option->state,s_text_color));
            painter->setPen(s_pen);
            painter->drawText(text_pos, m_input_pins.at(i));
            text_pos.setY(text_pos.y() + s_pin_font_height + s_pin_inner_vertical_spacing);
        }

        for (int i = 0; i < m_output_pins.size(); ++i)
        {
            if (gateHasFocus)
                if (g_selection_relay->m_subfocus == SelectionRelay::Subfocus::Right
                        && i == subFocusIndex)
                    s_pen.setColor(selectionColor());
                else
                    s_pen.setColor(s_text_color);
            else
               s_pen.setColor(penColor(option->state,s_text_color));
            painter->setPen(s_pen);
            painter->drawText(m_output_pin_positions.at(i), m_output_pins.at(i));
        }

        if (s_lod < graph_widget_constants::gate_max_lod)
        {
            QColor fade = m_color;
            fade.setAlphaF(s_alpha);
            painter->fillRect(QRectF(0, s_color_bar_height, m_width, m_height - s_color_bar_height), fade);
        }

        if (option->state & QStyle::State_Selected)
        {
            s_pen.setColor(selectionColor());
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
        else
        {
            QColor gcol = groupingColor();
            if (gcol.isValid())
            {
                QPen grPen(gcol);
                painter->setPen(grPen);
                painter->drawRect(boundingRect());
            }
        }
    }
}

QPointF StandardGraphicsGate::get_input_scene_position(const u32 net_id, const QString& pin_type) const
{
    Q_UNUSED(net_id)

    int index = m_input_pins.indexOf(pin_type);
    assert(index != -1);

    return endpointPositionByIndex(index,true);
}

QPointF StandardGraphicsGate::get_output_scene_position(const u32 net_id, const QString& pin_type) const
{
    Q_UNUSED(net_id)

    int index = m_output_pins.indexOf(pin_type);
    assert(index != -1);

    return endpointPositionByIndex(index,false);
}

float StandardGraphicsGate::yEndpointDistance() const
{
    return (s_pin_font_height + s_pin_inner_vertical_spacing);
}

float StandardGraphicsGate::yTopPinDistance() const
{
    return (s_color_bar_height + s_pin_upper_vertical_spacing + s_pin_font_height / 2);
}

QPointF StandardGraphicsGate::endpointPositionByIndex(int index, bool isInput) const
{
    qreal y = yTopPinDistance() + index * yEndpointDistance();
    return mapToScene(QPointF(isInput ? 0 : m_width, y));
}

void StandardGraphicsGate::format(const bool& adjust_size_to_grid)
{
    qreal textWidth[2];
    for (int iline=0; iline<2; iline++)
    {
        QFontMetricsF fmf(sTextFont[iline]);
        textWidth[iline] = fmf.width(mNodeText[iline]);
    }

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
        total_input_pin_height = m_input_pins.size() * s_pin_font_height +
                                (m_input_pins.size() - 1) * s_pin_inner_vertical_spacing +
                                 s_pin_upper_vertical_spacing + s_pin_lower_vertical_spacing;

    qreal total_output_pin_height = 0;

    if (!m_output_pins.isEmpty())
        total_output_pin_height = m_output_pins.size() * s_pin_font_height +
                                 (m_output_pins.size() - 1) * s_pin_inner_vertical_spacing +
                                  s_pin_upper_vertical_spacing + s_pin_lower_vertical_spacing;

    qreal max_pin_height = std::max(total_input_pin_height, total_output_pin_height);
    qreal min_body_height = s_inner_name_type_spacing + 2 * s_outer_name_type_spacing;

    qreal maxTextWidth = 0;
    for (int iline=0; iline<2; iline++)
    {
        min_body_height += sTextFontHeight[iline];
        if (maxTextWidth  < textWidth[iline]) maxTextWidth = textWidth[iline];
    }


    m_width = max_pin_width * 2 + s_pin_inner_horizontal_spacing * 2 + s_pin_outer_horizontal_spacing * 2 + maxTextWidth;
    m_height = std::max(max_pin_height, min_body_height) + s_color_bar_height;

    if (adjust_size_to_grid)
    {
        int floored_width = static_cast<int>(m_width);
        int quotient = floored_width / graph_widget_constants::grid_size;

        if (m_width > quotient * graph_widget_constants::grid_size)
            m_width = (quotient + 1) * graph_widget_constants::grid_size;

        int floored_height = static_cast<int>(m_height);
        quotient = floored_height / graph_widget_constants::grid_size;

        if (m_height > quotient * graph_widget_constants::grid_size)
            m_height = (quotient + 1) * graph_widget_constants::grid_size;
    }

    mTextPosition[0].setX(m_width / 2 - textWidth[0] / 2);
    mTextPosition[0].setY(std::max(m_height / 2 - sTextFontHeight[0] / 2 - s_inner_name_type_spacing / 2,
                          s_color_bar_height + s_outer_name_type_spacing + sTextFontHeight[0]));

    mTextPosition[1].setX(m_width / 2 - textWidth[1] / 2);
    mTextPosition[1].setY(mTextPosition[0].y() + sTextFontHeight[1] + s_inner_name_type_spacing / 2);

    qreal y = s_color_bar_height + s_pin_upper_vertical_spacing + s_pin_font_ascent + baseline;

    for (const QString& output_pin : m_output_pins)
    {
        qreal x = m_width - (pin_fm.size(0, output_pin).rwidth() + s_pin_outer_horizontal_spacing);
        m_output_pin_positions.append(QPointF(x, y));
        y += (s_pin_font_height + s_pin_inner_vertical_spacing);
    }
}
}
