#pragma once

#include "gui/graph_widget/items/nodes/modules/graphics_module.h"

class standard_graphics_module final : public graphics_module
{   
public:
    static void load_settings();
    static void update_alpha();

    explicit standard_graphics_module(const std::shared_ptr<module> m, bool adjust_size_to_grid = true);

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    QPointF get_input_scene_position(const u32 net_id, const QString& pin_type) const override;
    QPointF get_output_scene_position(const u32 net_id, const QString& pin_type) const override;

private:
    static qreal s_alpha;

    static QPen s_pen;

    static QColor s_text_color;

    static QFont s_name_font;
    static QFont s_type_font;
    static QFont s_pin_font;

    static qreal s_name_font_height;
    static qreal s_type_font_height;

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

    QPointF m_name_position;
    QPointF m_type_position;

    QVector<QPointF> m_output_pin_positions;
};
