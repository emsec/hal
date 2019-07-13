#ifndef ADVANCED_GRAPHICS_GATE_H
#define ADVANCED_GRAPHICS_GATE_H

#include "graphics_gate.h"

class standard_graphics_gate : public graphics_gate
{
public:
    static void load_settings();
    static void update_alpha();

    standard_graphics_gate(std::shared_ptr<gate> g, const bool adjust_size_to_grid = true);

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) Q_DECL_OVERRIDE;

    QPointF get_input_scene_position(const u32 net_id, const QString& pin_type) const Q_DECL_OVERRIDE;
    QPointF get_output_scene_position(const u32 net_id, const QString& pin_type) const Q_DECL_OVERRIDE;

    virtual void set_visuals(const visuals& v) Q_DECL_OVERRIDE;

private:
    static qreal s_alpha;

    static QPen s_pen;

    static QColor s_default_main_color;
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

    static qreal s_first_pin_y;
    static qreal s_pin_y_stride;

    void format(const bool& adjust_size_to_grid);

    QPointF m_name_position;
    QPointF m_type_position;

    QVector<QPointF> m_input_pin_positions;
    QVector<QPointF> m_output_pin_positions;
};

#endif // ADVANCED_GRAPHICS_GATE_H
