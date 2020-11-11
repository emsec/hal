//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include "gui/graph_widget/items/nodes/modules/graphics_module.h"
#include <QPixmap>

namespace hal
{
    class StandardGraphicsModule final : public GraphicsModule
    {
    public:
        static void load_settings();
        static void update_alpha();

        explicit StandardGraphicsModule(Module* m, bool adjust_size_to_grid = true);

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

        QPointF get_input_scene_position(const u32 net_id, const QString& pin_type) const override;
        QPointF get_output_scene_position(const u32 net_id, const QString& pin_type) const override;
        QPointF endpointPositionByIndex(int index, bool isInput) const override;
        float   yEndpointDistance() const override;
        float   yTopPinDistance() const override;

    private:
        static qreal s_alpha;

        static QPen s_pen;

        static QColor s_text_color;

        static QFont sTextFont[3];
        static QFont s_pin_font;

        static qreal sTextFontHeight[3];

        static qreal s_color_bar_height;

        static qreal s_pin_inner_horizontal_spacing;
        static qreal s_pin_outer_horizontal_spacing;

        static qreal s_pin_inner_vertical_spacing;
        static qreal s_pin_outer_vertical_spacing;
        static qreal s_pin_upper_vertical_spacing;
        static qreal s_pin_lower_vertical_spacing;

        static qreal s_pin_font_height;
        static qreal s_pin_font_ascent;
        static qreal s_pin_font_descent;
        static qreal s_pin_font_baseline;

        static qreal s_inner_name_type_spacing;
        static qreal s_outer_name_type_spacing;

        void format(const bool& adjust_size_to_grid);

        QPointF mTextPosition[3];

        QVector<QPointF> m_output_pin_positions;
        static const int sIconPadding;
        static const QSize sIconSize;

        static QPixmap* sIconInstance;
        static const QPixmap& iconPixmap();
    };
}    // namespace hal
