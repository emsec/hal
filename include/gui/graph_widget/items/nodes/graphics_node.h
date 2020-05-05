#ifndef GRAPHICS_NODE_H
#define GRAPHICS_NODE_H

#include "gui/graph_widget/items/graphics_item.h"

#include <QString>

class graphics_node : public graphics_item
{
public:
    struct visuals
    {
        bool visible;

        QColor main_color;
        QColor name_color;
        QColor type_color;
        QColor pin_color;
        QColor background_color;
        QColor border_color;
        QColor highlight_color;

        // DRAW TYPE ENUM
    };

    graphics_node(const hal::item_type type, const u32 id, const QString& name);

    virtual QRectF boundingRect() const override;
    virtual QPainterPath shape() const override;

    virtual QPointF get_input_scene_position(const u32 net_id, const QString& pin_type) const = 0;
    virtual QPointF get_output_scene_position(const u32 net_id, const QString& pin_type) const = 0;

//    virtual std::string get_input_pin_type_at_position(const size_t pos) const = 0;
//    virtual std::string get_output_pin_type_at_position(const size_t pos) const = 0;

    virtual void set_visuals(const visuals& v);

    qreal width() const;
    qreal height() const;

    void set_name(QString name);

//    qreal x_offset() const;
//    qreal y_offset() const;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

    QString m_name;

    qreal m_width;
    qreal m_height;

//    qreal m_x_offset;
//    qreal m_y_offset;
};

#endif // GRAPHICS_NODE_H
