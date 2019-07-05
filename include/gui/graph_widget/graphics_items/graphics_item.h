#ifndef GRAPH_GRAPHICS_ITEM_H
#define GRAPH_GRAPHICS_ITEM_H

#include "def.h"

#include <QColor>
#include <QGraphicsItem>

class graphics_item : public QGraphicsItem
{
public:
    enum class item_type // USE QT ITEM TYPE SYSTEM INSTEAD ???
    {
        module,
        gate,
        net
    };

    static void load_settings();
    static void set_lod(const qreal lod);

    graphics_item(const item_type type, const u32 id);

    u32 id() const;
    item_type get_item_type() const;

    void set_color(const QColor& color);

protected:
    static qreal s_lod;
    static QColor s_selection_color;

    void mousePressEvent(QGraphicsSceneMouseEvent* event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) Q_DECL_OVERRIDE;

    item_type m_class;
    u32 m_id;
    QColor m_color;
};

#endif // GRAPH_GRAPHICS_ITEM_H
