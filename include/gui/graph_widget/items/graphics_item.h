#pragma once

#include "def.h"

#include "gui/gui_def.h"

#include <QColor>
#include <QGraphicsItem>

class graphics_item : public QGraphicsItem
{
public:
    static void load_settings();
    static void set_lod(const qreal lod);

    graphics_item(const hal::item_type type, const u32 id);

    hal::item_type item_type() const;
    u32 id() const;

protected:
    static qreal s_lod;
    static QColor s_selection_color;

    hal::item_type m_item_type; // USE QT ITEM TYPE SYSTEM INSTEAD ???
    u32 m_id;
    QColor m_color;
};
