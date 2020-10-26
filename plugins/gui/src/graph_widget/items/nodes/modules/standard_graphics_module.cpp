#include "gui/graph_widget/items/nodes/modules/standard_graphics_module.h"

#include "hal_core/netlist/gate.h"

#include "gui/graph_widget/graph_widget_constants.h"
#include "gui/gui_globals.h"
#include "gui/gui_utils/graphics.h"

#include <QFont>
#include <QFontMetricsF>
#include <QPainter>
#include <QPen>
#include <QStyle>
#include <QStyleOptionGraphicsItem>
#include <QImage>

namespace hal
{
    static const qreal baseline = 1;

    qreal StandardGraphicsModule::s_alpha;

    QPen StandardGraphicsModule::s_pen;

    QColor StandardGraphicsModule::s_text_color;

    QFont StandardGraphicsModule::sTextFont[3];
    QFont StandardGraphicsModule::s_pin_font;

    qreal StandardGraphicsModule::sTextFontHeight[3] = {0, 0, 0};

    qreal StandardGraphicsModule::s_color_bar_height = 30;

    qreal StandardGraphicsModule::s_pin_inner_horizontal_spacing = 12;
    qreal StandardGraphicsModule::s_pin_outer_horizontal_spacing = 2.4;

    qreal StandardGraphicsModule::s_pin_inner_vertical_spacing = 1.2;
    qreal StandardGraphicsModule::s_pin_outer_vertical_spacing = 0.6;
    qreal StandardGraphicsModule::s_pin_upper_vertical_spacing = 0.6;
    qreal StandardGraphicsModule::s_pin_lower_vertical_spacing = 1.8;

    qreal StandardGraphicsModule::s_pin_font_height;
    qreal StandardGraphicsModule::s_pin_font_ascent;
    qreal StandardGraphicsModule::s_pin_font_descent;
    qreal StandardGraphicsModule::s_pin_font_baseline;

    qreal StandardGraphicsModule::s_inner_name_type_spacing = 1.2;
    qreal StandardGraphicsModule::s_outer_name_type_spacing = 3;

    const int StandardGraphicsModule::ICON_PADDING = 3;
    const QSize StandardGraphicsModule::ICON_SIZE(s_color_bar_height - 2 * ICON_PADDING,
                                                  s_color_bar_height - 2 * ICON_PADDING);
    QPixmap* StandardGraphicsModule::sIconInstance = nullptr;

    const QPixmap& StandardGraphicsModule::iconPixmap()
    {
        if (!sIconInstance) sIconInstance
                = new QPixmap(QPixmap::fromImage(QImage(":/icons/sel_module").scaled(ICON_SIZE)));
        return *sIconInstance;
    }

    void StandardGraphicsModule::load_settings()
    {
        s_pen.setCosmetic(true);
        s_pen.setJoinStyle(Qt::MiterJoin);

        s_text_color = QColor(160, 160, 160);

        QFont font = QFont("Iosevka");
        font.setPixelSize(graph_widget_constants::font_size);

        for (int iline = 0; iline < 3; iline++)
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

        s_selectionColor = QColor(240, 173, 0);
        s_highlightColor = QColor(40, 200, 240);
    }

    void StandardGraphicsModule::update_alpha()
    {
        if (s_lod <= graph_widget_constants::gate_max_lod)
            s_alpha = 1 - (s_lod - graph_widget_constants::gate_min_lod) / (graph_widget_constants::gate_max_lod - graph_widget_constants::gate_min_lod);
        else
            s_alpha = 0;
    }

    StandardGraphicsModule::StandardGraphicsModule(Module* m, bool adjust_size_to_grid) : GraphicsModule(m)
    {
        format(adjust_size_to_grid);
    }

    void StandardGraphicsModule::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
    {
        Q_UNUSED(widget);

        if (s_lod < graph_widget_constants::gate_min_lod)
        {
            painter->fillRect(QRectF(0, 0, m_width, m_height), penColor(option->state));
            return;
        }
        else
        {
            // draw box and icon
            painter->fillRect(QRectF(0, 0, m_width, s_color_bar_height), m_color);
            painter->fillRect(QRectF(0, s_color_bar_height, m_width, m_height - s_color_bar_height), QColor(0, 0, 0, 200));
//            QRectF iconRect(ICON_PADDING,ICON_PADDING,ICON_SIZE.width(),ICON_SIZE.height());
//            painter->fillRect(iconRect,Qt::black);
//            painter->drawPixmap(QPoint(ICON_PADDING,ICON_PADDING), iconPixmap());

            // draw center text
            s_pen.setColor(penColor(option->state,s_text_color));
            painter->setPen(s_pen);

            for (int iline=0; iline<3; iline++)
            {
                int i = 0;

                switch (iline)
                {
                case 0: i = 2; break;
                case 1: i = 0; break;
                case 2: i = 1; break;
                }

                if (mNodeText[iline].isEmpty()) continue;
                painter->setFont(sTextFont[iline]);
                painter->drawText(QPointF(mTextPosition[i].x(), mTextPosition[iline].y()), mNodeText[i]);

                // Weird generalization but okay...
            }

            bool moduleHasFocus =
                    g_selection_relay->m_focus_type == SelectionRelay::item_type::module
                    && g_selection_relay->m_focus_id == m_id;
            int subFocusIndex = static_cast<int>(g_selection_relay->m_subfocus_index);

            painter->setFont(s_pin_font);
            QPointF text_pos(s_pin_outer_horizontal_spacing, s_color_bar_height + s_pin_upper_vertical_spacing + s_pin_font_ascent + baseline);

            for (int i = 0; i < m_input_pins.size(); ++i)
            {
                if (moduleHasFocus)
                    if (g_selection_relay->m_subfocus == SelectionRelay::subfocus::left
                            && i == subFocusIndex)
                        s_pen.setColor(selectionColor());
                    else
                        s_pen.setColor(s_text_color);
                else
                   s_pen.setColor(penColor(option->state,s_text_color));
                painter->setPen(s_pen);
                painter->drawText(text_pos, m_input_pins.at(i).name);
                text_pos.setY(text_pos.y() + s_pin_font_height + s_pin_inner_vertical_spacing);
            }

            for (int i = 0; i < m_output_pins.size(); ++i)
            {
                if (moduleHasFocus)
                    if (g_selection_relay->m_subfocus == SelectionRelay::subfocus::right
                            && i == subFocusIndex)
                        s_pen.setColor(selectionColor());
                    else
                        s_pen.setColor(s_text_color);
                else
                   s_pen.setColor(penColor(option->state,s_text_color));
                painter->setPen(s_pen);
                painter->drawText(m_output_pin_positions.at(i), m_output_pins.at(i).name);
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
                    painter->drawRect(boundingRect());
                }
            }
        }
    }

    QPointF StandardGraphicsModule::get_input_scene_position(const u32 net_id, const QString& pin_type) const
    {
        Q_UNUSED(pin_type)

        int index = 0;

        for (; index < m_input_pins.size(); ++index)
            if (m_input_pins.at(index).net_id == net_id)
                break;

        assert(index < m_input_pins.size());

        qreal y = s_color_bar_height + s_pin_upper_vertical_spacing;

        if (index > 0)
            y += index * (s_pin_font_height + s_pin_inner_vertical_spacing);

        y += s_pin_font_height / 2;

        return mapToScene(QPointF(0, y));
    }

    QPointF StandardGraphicsModule::get_output_scene_position(const u32 net_id, const QString& pin_type) const
    {
        Q_UNUSED(pin_type)

        int index = 0;

        for (; index < m_output_pins.size(); ++index)
            if (m_output_pins.at(index).net_id == net_id)
                break;

        assert(index < m_output_pins.size());

        qreal y = s_color_bar_height + s_pin_upper_vertical_spacing;

        if (index > 0)
            y += index * (s_pin_font_height + s_pin_inner_vertical_spacing);

        y += s_pin_font_height / 2;

        return mapToScene(QPointF(m_width, y));
    }

    void StandardGraphicsModule::format(const bool& adjust_size_to_grid)
    {
        qreal textWidth[3] = {0, 0, 0};
        for (int iline = 0; iline < 3; iline++)
        {
            QFontMetricsF fmf(sTextFont[iline]);
            textWidth[iline] = fmf.width(mNodeText[iline]);
        }

        QFontMetricsF pin_fm(s_pin_font);
        qreal max_pin_width = 0;

        for (const module_pin& input_pin : m_input_pins)
        {
            qreal width = pin_fm.width(input_pin.name);
            if (width > max_pin_width)
                max_pin_width = width;
        }

        for (const module_pin& output_pin : m_output_pins)
        {
            qreal width = pin_fm.width(output_pin.name);
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
        for (int iline=0; iline<3; iline++)
        {
            if (iline != 2 || !mNodeText[iline].isEmpty())
                min_body_height += sTextFontHeight[iline];
            if (maxTextWidth < textWidth[iline]) maxTextWidth = textWidth[iline];
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

        qreal ytext = std::max(m_height / 2 - sTextFontHeight[0] * 3/2 - s_inner_name_type_spacing / 2,
                               s_color_bar_height + s_outer_name_type_spacing);

        for (int iline=0; iline<3; iline++)
        {
            ytext += sTextFontHeight[iline];
            mTextPosition[iline].setX(m_width / 2 - textWidth[iline] / 2);
            mTextPosition[iline].setY(ytext);
            ytext += s_inner_name_type_spacing / 2;
        }

        qreal y = s_color_bar_height + s_pin_upper_vertical_spacing + s_pin_font_ascent + baseline;

        for (const module_pin& output_pin : m_output_pins)
        {
            qreal x = m_width - (pin_fm.size(0, output_pin.name).rwidth() + s_pin_outer_horizontal_spacing);
            m_output_pin_positions.append(QPointF(x, y));
            y += (s_pin_font_height + s_pin_inner_vertical_spacing);
        }
    }
}
