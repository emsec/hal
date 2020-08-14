#include "graph_widget/items/nets/standard_graphics_net.h"

#include "graph_widget/graph_widget_constants.h"

#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QStyleOptionGraphicsItem>

#include <limits>

#include <assert.h>

namespace hal
{
    qreal StandardGraphicsNet::s_alpha;

    qreal StandardGraphicsNet::s_wire_length;
    qreal StandardGraphicsNet::s_left_arrow_offset;
    qreal StandardGraphicsNet::s_right_arrow_offset;
    qreal StandardGraphicsNet::s_arrow_left_x_shift;
    qreal StandardGraphicsNet::s_arrow_right_x_shift;
    qreal StandardGraphicsNet::s_arrow_side_length;
    qreal StandardGraphicsNet::s_arrow_width;
    qreal StandardGraphicsNet::s_arrow_height;

    QPainterPath StandardGraphicsNet::s_arrow;

    qreal StandardGraphicsNet::s_split_radius;

    void StandardGraphicsNet::load_settings()
    {
        s_wire_length = 26;

        s_left_arrow_offset = 3;
        s_right_arrow_offset = 3;

        s_arrow_left_x_shift = 0;
        s_arrow_right_x_shift = 3;
        s_arrow_side_length = 12;

        s_arrow_height = 6;
        s_arrow_width = s_arrow_left_x_shift + s_arrow_side_length + s_arrow_right_x_shift;

        QPointF point(s_arrow_left_x_shift, -s_arrow_height / 2);

        #if (QT_VERSION >= QT_VERSION_CHECK(5, 13, 0))
            // only available in Qt >= 5.13.0
            s_arrow.clear();
        #else
            // low-performance fallback for older Qt
            s_arrow = QPainterPath();
        #endif
        s_arrow.lineTo(point);
        point.setX(point.x() + s_arrow_side_length);
        s_arrow.lineTo(point);
        point.setX(point.x() + s_arrow_right_x_shift);
        point.setY(0);
        s_arrow.lineTo(point);
        point.setX(point.x() - s_arrow_right_x_shift);
        point.setY(s_arrow_height / 2);
        s_arrow.lineTo(point);
        point.setX(point.x() - s_arrow_side_length);
        s_arrow.lineTo(point);
        s_arrow.closeSubpath();

        s_split_radius = 3;
    }

    void StandardGraphicsNet::update_alpha()
    {
        if (s_lod >= graph_widget_constants::net_fade_in_lod && s_lod <= graph_widget_constants::net_fade_out_lod)
            s_alpha = (s_lod - graph_widget_constants::net_fade_in_lod) / (graph_widget_constants::net_fade_out_lod - graph_widget_constants::net_fade_in_lod);
        else
            s_alpha = 1;
    }

    StandardGraphicsNet::StandardGraphicsNet(const std::shared_ptr<const Net> n, const lines& l) : GraphicsNet(n)
    {
        for (const h_line& h : l.h_lines)
        {
            for (const v_line& v : l.v_lines)
            {
                if (h.small_x <= v.x && v.x <= h.big_x)
                    if (v.small_y < h.y && h.y < v.big_y)
                    {
                        QPointF point(v.x, h.y);
                        m_splits.append(point);
                        m_shape.addEllipse(point, s_split_radius, s_split_radius);
                    }

                if (v.small_y <= h.y && h.y <= v.big_y)
                    if (h.small_x < v.x && v.x < h.big_x)
                    {
                        QPointF point(v.x, h.y);
                        m_splits.append(point);
                        m_shape.addEllipse(point, s_split_radius, s_split_radius);
                    }
            }
        }

        qreal smallest_x = std::numeric_limits<qreal>::max();
        qreal biggest_x = std::numeric_limits<qreal>::min();

        qreal smallest_y = std::numeric_limits<qreal>::max();
        qreal biggest_y = std::numeric_limits<qreal>::min();

        for (const h_line& h : l.h_lines)
        {
            if (h.small_x < smallest_x)
                smallest_x = h.small_x;

            if (h.big_x > biggest_x)
                biggest_x = h.big_x;

            if (h.y < smallest_y)
                smallest_y = h.y;
            else if (h.y > biggest_y)
                biggest_y = h.y;

            QLineF line(h.small_x, h.y, h.big_x, h.y);
            m_lines.append(line);
            QRectF rect(h.small_x - s_shape_width / 2, h.y - s_shape_width / 2, h.big_x - h.small_x + s_shape_width, s_shape_width);
            m_shape.addRect(rect);
        }

        for (const v_line& v : l.v_lines)
        {
            if (v.x < smallest_x)
                smallest_x = v.x;
            else if (v.x > biggest_x)
                biggest_x = v.x;

            if (v.small_y < smallest_y)
                smallest_y = v.small_y;

            if (v.big_y > biggest_y)
                biggest_y = v.big_y;

            QLineF line(v.x, v.small_y, v.x, v.big_y);
            m_lines.append(line);
            QRectF rect(v.x - s_shape_width / 2, v.small_y - s_shape_width / 2, s_shape_width, v.big_y - v.small_y + s_shape_width);
            m_shape.addRect(rect);
        }

        const qreal padding = s_split_radius + s_shape_width;
        m_rect = QRectF(smallest_x - padding, smallest_y - padding, biggest_x - smallest_x + padding, biggest_y - smallest_y + padding);
    }

    void StandardGraphicsNet::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
    {
        Q_UNUSED(widget);

        QColor color = penColor(option->state);
        s_pen.setColor(color);
        s_pen.setStyle(m_pen_style);

        painter->setPen(s_pen);
        painter->drawLines(m_lines);

        if (s_lod > graph_widget_constants::net_fade_in_lod)
        {
            color.setAlphaF(s_alpha);

            s_pen.setColor(color);
            s_pen.setStyle(Qt::SolidLine);
            painter->setPen(s_pen);

            s_brush.setColor(color);
            s_brush.setStyle(Qt::SolidPattern);
            painter->setBrush(s_brush);

            const bool original_antialiasing = painter->renderHints().testFlag(QPainter::Antialiasing);
            painter->setRenderHint(QPainter::Antialiasing, true);

            for (const QPointF& point : m_splits)
                painter->drawEllipse(point, s_split_radius, s_split_radius);

            painter->setRenderHint(QPainter::Antialiasing, original_antialiasing);
        }

        s_brush.setStyle(Qt::NoBrush);
        painter->setBrush(s_brush);

    #ifdef HAL_DEBUG_GUI_GRAPH_WIDGET
        s_pen.setColor(Qt::green);
        const bool original_cosmetic = s_pen.isCosmetic();
        s_pen.setCosmetic(true);
        painter->setPen(s_pen);
        painter->drawPath(m_shape);
        s_pen.setCosmetic(original_cosmetic);
    #endif
    }

    void StandardGraphicsNet::lines::append_h_line(const qreal small_x, const qreal big_x, const qreal y)
    {
        assert(small_x < big_x);

        h_lines.append(h_line{small_x, big_x, y});
    }

    void StandardGraphicsNet::lines::append_v_line(const qreal x, const qreal small_y, const qreal big_y)
    {
        assert(small_y < big_y);

        v_lines.append(v_line{x, small_y, big_y});
    }

    void StandardGraphicsNet::lines::merge_lines()
    {
        QVector<h_line> merged_h_lines;
        QVector<v_line> merged_v_lines;

        for (const h_line& h : h_lines)
        {
            QVector<int> overlaps;

            for (int i = 0; i < merged_h_lines.size(); ++i)
                if (h.y == merged_h_lines.at(i).y)
                {
                    if (h.big_x < merged_h_lines.at(i).small_x || merged_h_lines.at(i).big_x < h.small_x)
                        continue;

                    overlaps.append(i);
                }


            if (overlaps.isEmpty())
                merged_h_lines.append(h);
            else
            {
                qreal smallest_x = h.small_x;
                qreal biggest_x = h.big_x;

                for (int i = 0; i < overlaps.size(); ++i)
                {
                    int index = overlaps.at(i) - i;

                    if (merged_h_lines.at(index).small_x < smallest_x)
                        smallest_x = merged_h_lines.at(index).small_x;

                    if (merged_h_lines.at(index).big_x > biggest_x)
                        biggest_x = merged_h_lines.at(index).big_x;

                    merged_h_lines.remove(index);
                }

                merged_h_lines.append(h_line{smallest_x, biggest_x, h.y});
            }
        }

        for (const v_line& v : v_lines)
        {
            QVector<int> overlaps;

            for (int i = 0; i < merged_v_lines.size(); ++i)
                if (v.x == merged_v_lines.at(i).x)
                {
                    if (v.big_y < merged_v_lines.at(i).small_y || merged_v_lines.at(i).big_y < v.small_y)
                        continue;

                    overlaps.append(i);
                }


            if (overlaps.isEmpty())
                merged_v_lines.append(v);
            else
            {
                qreal smallest_y = v.small_y;
                qreal biggest_y = v.big_y;

                for (int i = 0; i < overlaps.size(); ++i)
                {
                    int index = overlaps.at(i) - i;

                    if (merged_v_lines.at(index).small_y < smallest_y)
                        smallest_y = merged_v_lines.at(index).small_y;

                    if (merged_v_lines.at(index).big_y > biggest_y)
                        biggest_y = merged_v_lines.at(index).big_y;

                    merged_v_lines.remove(index);
                }

                merged_v_lines.append(v_line{v.x, smallest_y, biggest_y});
            }
        }

        h_lines = merged_h_lines;
        v_lines = merged_v_lines;
    }
}
