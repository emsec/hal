#include "gui/graph_widget/graphics_scene.h"

#include "netlist/gate.h"
#include "netlist/net.h"
#include "core/log.h"

#include "gui/graph_widget/graph_widget_constants.h"
#include "gui/graph_widget/graphics_factory.h"
#include "gui/graph_widget/items/graphics_gate.h"
#include "gui/graph_widget/items/graphics_item.h"
#include "gui/graph_widget/items/graphics_module.h"
#include "gui/graph_widget/items/graphics_net.h"
#include "netlist/gate.h"
//#include "gui/graph_widget/graphics_items/utility_items/gate_navigation_popup.h"
#include "gui/gui_globals.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QString>

#include <QDebug>

qreal graphics_scene::s_lod = 0;

const qreal graphics_scene::s_grid_fade_start = 0.4;
const qreal graphics_scene::s_grid_fade_end = 1.0;

bool graphics_scene::s_grid_enabled = true;
bool graphics_scene::s_grid_clusters_enabled = true;
graph_widget_constants::grid_type graphics_scene::s_grid_type = graph_widget_constants::grid_type::lines;

QColor graphics_scene::s_grid_base_line_color = QColor(30, 30, 30);
QColor graphics_scene::s_grid_cluster_line_color = QColor(15, 15, 15);

QColor graphics_scene::s_grid_base_dot_color = QColor(25, 25, 25);
QColor graphics_scene::s_grid_cluster_dot_color = QColor(170, 160, 125);

void graphics_scene::set_lod(const qreal& lod)
{
    s_lod = lod;

    if (lod >= s_grid_fade_start && lod <= s_grid_fade_end)
    {
        const qreal alpha = (lod - s_grid_fade_start) / (s_grid_fade_end - s_grid_fade_start);

        s_grid_base_line_color.setAlphaF(alpha);
        s_grid_cluster_line_color.setAlphaF(alpha);

        s_grid_base_dot_color.setAlphaF(alpha);
        s_grid_cluster_dot_color.setAlphaF(alpha);
    }
    else
    {
        const int alpha = 255;

        s_grid_base_line_color.setAlpha(alpha);
        s_grid_cluster_line_color.setAlpha(alpha);

        s_grid_base_dot_color.setAlpha(alpha);
        s_grid_cluster_dot_color.setAlpha(alpha);
    }
}

void graphics_scene::set_grid_enabled(const bool& value)
{
    s_grid_enabled = value;
}

void graphics_scene::set_grid_clusters_enabled(const bool& value)
{
    s_grid_clusters_enabled = value;
}

void graphics_scene::set_grid_type(const graph_widget_constants::grid_type& grid_type)
{
    s_grid_type = grid_type;
}

void graphics_scene::set_grid_base_line_color(const QColor& color)
{
    // ALTERNATIVELY ADRESS ADAPTER MEMBERS DIRECTLY
    s_grid_base_line_color = color;
}

void graphics_scene::set_grid_cluster_line_color(const QColor& color)
{
    s_grid_cluster_line_color = color;
}

void graphics_scene::set_grid_base_dot_color(const QColor& color)
{
    s_grid_base_dot_color = color;
}

void graphics_scene::set_grid_cluster_dot_color(const QColor& color)
{
    s_grid_cluster_dot_color = color;
}

QPointF graphics_scene::snap_to_grid(const QPointF& pos)
{
    int adjusted_x = qRound(pos.x() / graph_widget_constants::grid_size) * graph_widget_constants::grid_size;
    int adjusted_y = qRound(pos.y() / graph_widget_constants::grid_size) * graph_widget_constants::grid_size;
    return QPoint(adjusted_x, adjusted_y);
}

graphics_scene::graphics_scene(QObject* parent) : QGraphicsScene(parent),
    m_drag_shadow_gate(new drag_shadow_gate())
//    m_left_gate_navigation_popup(new gate_navigation_popup(gate_navigation_popup::type::left)),
//    m_right_gate_navigation_popup(new gate_navigation_popup(gate_navigation_popup::type::right))
{
    // FIND OUT IF MANUAL CHANGE TO DEPTH IS NECESSARY / INCREASES PERFORMANCE
    //m_scene.setBspTreeDepth(10);

    connect_all();

//    QGraphicsScene::addItem(m_left_gate_navigation_popup);

    QGraphicsScene::addItem(m_drag_shadow_gate);

    #ifdef GUI_DEBUG_GRID
    m_debug_grid_enable = g_settings_manager.get("debug/grid").toBool();
    #endif
}

void graphics_scene::start_drag_shadow(const QPointF& posF, const QSizeF& sizeF, const drag_shadow_gate::drag_cue cue)
{
    m_drag_shadow_gate->set_visual_cue(cue);
    m_drag_shadow_gate->start(posF, sizeF);
}

void graphics_scene::move_drag_shadow(const QPointF& posF, const drag_shadow_gate::drag_cue cue)
{
    m_drag_shadow_gate->setPos(posF);
    m_drag_shadow_gate->set_visual_cue(cue);
}

void graphics_scene::stop_drag_shadow()
{
    m_drag_shadow_gate->stop();
}

QPointF graphics_scene::drop_target()
{
    return m_drag_shadow_gate->pos();
}

void graphics_scene::add_item(graphics_item* item)
{
    // SELECTION HAS TO BE UPDATED MANUALLY AFTER ADDING / REMOVING ITEMS

    if (!item)
        return;

    QGraphicsScene::addItem(item);

    switch (item->item_type())
    {
    case hal::item_type::gate:
    {
        graphics_gate* g = static_cast<graphics_gate*>(item);
        int i = 0;
        while (i < m_gate_items.size())
        {
            if (g->id() < m_gate_items.at(i).id)
                break;

            i++;
        }
        m_gate_items.insert(i, gate_data{g->id(), g});
        return;
    }
    case hal::item_type::net:
    {
        graphics_net* n = static_cast<graphics_net*>(item);
        int i = 0;
        while (i < m_net_items.size())
        {
            if (n->id() < m_net_items.at(i).id)
                break;

            i++;
        }
        m_net_items.insert(i, net_data{n->id(), n});
        return;
    }
    case hal::item_type::module:
    {
        graphics_module* m = static_cast<graphics_module*>(item);
        int i = 0;
        while (i < m_module_items.size())
        {
            if (m->id() < m_module_items.at(i).id)
                break;

            i++;
        }
        m_module_items.insert(i, module_data{m->id(), m});
        return;
    }
    }
}

void graphics_scene::remove_item(graphics_item* item)
{
    // SELECTION HAS TO BE UPDATED MANUALLY AFTER ADDING / REMOVING ITEMS

    if (!item)
        return;

    QGraphicsScene::removeItem(item);

    switch (item->item_type())
    {
    case hal::item_type::gate:
    {
        graphics_gate* g = static_cast<graphics_gate*>(item);
        u32 id = g->id();

        int i = 0;
        while (i < m_gate_items.size())
        {
            if (m_gate_items[i].id == id)
            {
                m_gate_items.remove(i);
                delete g;
                return;
            }

            ++i;
        }

        return;
    }
    case hal::item_type::net:
    {
        graphics_net* n = static_cast<graphics_net*>(item);
        u32 id = n->id();

        int i = 0;
        while (i < m_net_items.size())
        {
            if (m_net_items[i].id == id)
            {
                m_net_items.remove(i);
                delete n;
                return;
            }

            ++i;
        }

        return;
    }
    case hal::item_type::module:
    {
        graphics_module* m = static_cast<graphics_module*>(item);
        u32 id = m->id();

        int i = 0;
        while (i < m_module_items.size())
        {
            if (m_module_items[i].id == id)
            {
                m_module_items.remove(i);
                delete m;
                return;
            }

            ++i;
        }

        return;
    }
    }
}

const graphics_gate* graphics_scene::get_gate_item(const u32 id) const
{
    for (const gate_data& d : m_gate_items)
    {
        if (d.id > id)
            break;

        if (d.id == id)
            return d.item;
    }

    return nullptr;
}

//void graphics_scene::update_utility_items()
//{
//    // TEST IMPLEMENTATION
//    // EXPAND WITH MORE ITEMS

//    if (g_selection_relay.m_focus_type == selection_relay::item_type::gate)
//    {
//        for (gate_data& d : m_gate_items)
//        {
//            if (d.id == g_selection_relay.m_selected_gates[0])
//            {
//                graphics_gate* g = d.item;

//                // IF (ANIMATE) ANIMATE ELSE DONT
//                // ALTERNATIVELY ANIMATE IF SCENE HAS LESS THAN X ITEMS
//                m_left_gate_navigation_popup->stop();
//                m_left_gate_navigation_popup->set_height(g->height());
//                m_left_gate_navigation_popup->start(QPointF(g->pos().x() - 120, g->pos().y()));
//            }
//        }
//    }
//    else
//        m_left_gate_navigation_popup->stop();
//}

void graphics_scene::connect_all()
{
    connect(&g_settings_relay, &settings_relay::setting_changed, this, &graphics_scene::handle_global_setting_changed);

    connect(this, &graphics_scene::selectionChanged, this, &graphics_scene::handle_intern_selection_changed);

    connect(&g_selection_relay, &selection_relay::selection_changed, this, &graphics_scene::handle_extern_selection_changed);
    connect(&g_selection_relay, &selection_relay::subfocus_changed, this, &graphics_scene::handle_extern_subfocus_changed);
}

void graphics_scene::disconnect_all()
{
    disconnect(&g_settings_relay, &settings_relay::setting_changed, this, &graphics_scene::handle_global_setting_changed);

    disconnect(this, &graphics_scene::selectionChanged, this, &graphics_scene::handle_intern_selection_changed);

    disconnect(&g_selection_relay, &selection_relay::selection_changed, this, &graphics_scene::handle_extern_selection_changed);
    disconnect(&g_selection_relay, &selection_relay::subfocus_changed, this, &graphics_scene::handle_extern_subfocus_changed);
}

void graphics_scene::delete_all_items()
{
    // this breaks the m_drag_shadow_gate
    // clear();
    // so we do this instead
    // TODO check performance hit
    for (auto item : items())
    {
        if (item != m_drag_shadow_gate)
        {
            removeItem(item);
        }
    }

    m_module_items.clear();
    m_gate_items.clear();
    m_net_items.clear();
}

void graphics_scene::update_visuals(const graph_shader::shading& s)
{
    for (module_data m : m_module_items)
    {
        m.item->set_visuals(s.module_visuals.value(m.id));
    }

    for (gate_data g : m_gate_items)
    {
        g.item->set_visuals(s.gate_visuals.value(g.id));
    }
}

void graphics_scene::move_nets_to_background()
{
    for (net_data d : m_net_items)
        d.item->setZValue(-1);
}

void graphics_scene::handle_intern_selection_changed()
{
    g_selection_relay.clear();

    int gates = 0;
    int nets = 0;
    int modules = 0;

    for (const QGraphicsItem* const item : selectedItems())
    {
        switch (static_cast<const graphics_item* const>(item)->item_type())
        {
        case hal::item_type::gate:
        {
            g_selection_relay.m_selected_gates.insert(static_cast<const graphics_item* const>(item)->id());
            ++gates;
            break;
        }
        case hal::item_type::net:
        {
            g_selection_relay.m_selected_nets.insert(static_cast<const graphics_item* const>(item)->id());
            ++nets;
            break;
        }
        case hal::item_type::module:
        {
            g_selection_relay.m_selected_modules.insert(static_cast<const graphics_item* const>(item)->id());
            ++modules;
            break;
        }
        }
    }

    // TEST CODE
    // ADD FOCUS DEDUCTION INTO RELAY ???
    if (gates + nets + modules == 1)
    {
        if (gates)
        {
            g_selection_relay.m_focus_type = selection_relay::item_type::gate;
            g_selection_relay.m_focus_id = *g_selection_relay.m_selected_gates.begin(); // UNNECESSARY ??? USE ARRAY[0] INSTEAD OF MEMBER VARIABLE ???
        }
        else if (nets)
        {
            g_selection_relay.m_focus_type = selection_relay::item_type::net;
            g_selection_relay.m_focus_id = *g_selection_relay.m_selected_nets.begin(); // UNNECESSARY ??? USE ARRAY[0] INSTEAD OF MEMBER VARIABLE ???
        }
        else
        {
            g_selection_relay.m_focus_type = selection_relay::item_type::module;
            g_selection_relay.m_focus_id = *g_selection_relay.m_selected_modules.begin(); // UNNECESSARY ??? USE ARRAY[0] INSTEAD OF MEMBER VARIABLE ???
        }
    }
    else
    {
        g_selection_relay.m_focus_type = selection_relay::item_type::none;
    }
    g_selection_relay.m_subfocus = selection_relay::subfocus::none;
    // END OF TEST CODE

    g_selection_relay.relay_selection_changed(this);
}

void graphics_scene::handle_extern_selection_changed(void* sender)
{
    // CLEAR CURRENT SELECTION EITHER MANUALLY OR USING clearSelection()
    // UNCERTAIN ABOUT THE SENDER PARAMETER

    if (sender == this)
        return;

    bool original_value = blockSignals(true);

    clearSelection();

    if (!g_selection_relay.m_selected_modules.isEmpty())
    {
        for (auto& element : m_module_items)
        {
            if (g_selection_relay.m_selected_modules.find(element.id) != g_selection_relay.m_selected_modules.end())
            {
                element.item->setSelected(true);
                element.item->update();
            }
        }
    }

    if (!g_selection_relay.m_selected_gates.isEmpty())
    {
        for (auto& element : m_gate_items)
        {
            if (g_selection_relay.m_selected_gates.find(element.id) != g_selection_relay.m_selected_gates.end())
            {
                element.item->setSelected(true);
                element.item->update();
            }
        }
    }

    if (!g_selection_relay.m_selected_nets.isEmpty())
    {
        for (auto& element : m_net_items)
        {
            if (g_selection_relay.m_selected_nets.find(element.id) != g_selection_relay.m_selected_nets.end())
            {
                element.item->setSelected(true);
                element.item->update();
            }
        }
    }

    //update_utility_items();

    blockSignals(original_value);
}

void graphics_scene::handle_extern_subfocus_changed(void* sender)
{
    Q_UNUSED(sender)

    //update_utility_items();
}

void graphics_scene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    // CONTEXT MENU CLEARING SELECTION WORKAROUND
    if (event->button() == Qt::RightButton)
    {
        event->accept();
        return;
    }

    QGraphicsScene::mousePressEvent(event);
}

void graphics_scene::handle_global_setting_changed(void* sender, const QString& key, const QVariant& value)
{
    #ifdef GUI_DEBUG_GRID
    if (key == "debug/grid")
    {
        m_debug_grid_enable = value.toBool();
    }
    #endif
}

void graphics_scene::drawBackground(QPainter* painter, const QRectF& rect)
{
    if (!s_grid_enabled)
        return;

    if (s_lod < s_grid_fade_start)
        return;

    QFlags original_flags = painter->renderHints(); // UNNECESSARY ?
    painter->setRenderHint(QPainter::Antialiasing, true);

    QPen pen;
    pen.setWidth(2);

    // OVERDRAW NEEDED BECAUSE QT FAILS AT BASIC GEOMETRY
    const int overdraw = 2;

    const int x_from = rect.left() - overdraw;
    const int x_to = rect.right() + overdraw;

    const int y_from = rect.top() - overdraw;
    const int y_to = rect.bottom() + overdraw;

    const int x_offset = x_from % graph_widget_constants::grid_size;
    const int y_offset = y_from % graph_widget_constants::grid_size;

    switch (s_grid_type)
    {
    case graph_widget_constants::grid_type::lines:
    {
        QVarLengthArray<QLine, 512> base_lines;
        QVarLengthArray<QLine, 64> cluster_lines;

        for (int x = x_from - x_offset; x < x_to; x += graph_widget_constants::grid_size)
        {
            QLine line(x, y_from, x, y_to);
            if (x % (graph_widget_constants::grid_size * graph_widget_constants::cluster_size))
                base_lines.append(line);
            else
                cluster_lines.append(line);
        }

        for (int y = y_from - y_offset; y < y_to; y += graph_widget_constants::grid_size)
        {
            QLine line(x_from, y, x_to, y);
            if (y % (graph_widget_constants::grid_size * graph_widget_constants::cluster_size))
                base_lines.append(line);
            else
                cluster_lines.append(line);
        }

        pen.setColor(s_grid_base_line_color);
        painter->setPen(pen);

        painter->drawLines(base_lines.data(), base_lines.size());

        if (s_grid_clusters_enabled)
        {
            pen.setColor(s_grid_cluster_line_color);
            painter->setPen(pen);
        }

        painter->drawLines(cluster_lines.data(), cluster_lines.size());
        break;
    }

    case graph_widget_constants::grid_type::dots:
    {
        QVector<QPoint> base_points;
        QVector<QPoint> cluster_points;

        for (int x = x_from - x_offset; x < x_to; x += graph_widget_constants::grid_size)
            for (int y = y_from - y_offset; y < y_to; y += graph_widget_constants::grid_size)
            {
                if ((x % (graph_widget_constants::grid_size * graph_widget_constants::cluster_size)) && (y % (graph_widget_constants::grid_size * graph_widget_constants::cluster_size)))
                    base_points.append(QPoint(x,y));
                else
                    cluster_points.append(QPoint(x,y));
            }

        pen.setColor(s_grid_base_dot_color);
        painter->setPen(pen);

        painter->drawPoints(base_points.data(), base_points.size());

        if (s_grid_clusters_enabled)
        {
            pen.setColor(s_grid_cluster_dot_color);
            painter->setPen(pen);
        }

        painter->drawPoints(cluster_points.data(), cluster_points.size());
        break;
    }
    }

    #ifdef GUI_DEBUG_GRID
    if (m_debug_grid_enable)
        debug_draw_layouter_grid(painter, x_from, x_to, y_from, y_to);
    #endif

    painter->setRenderHints(original_flags); // UNNECESSARY ?
}

#ifdef GUI_DEBUG_GRID
void graphics_scene::debug_set_layouter_grid(const QVector<qreal>& debug_x_lines, const QVector<qreal>& debug_y_lines, qreal debug_default_height, qreal debug_default_width)
{
    m_debug_x_lines = debug_x_lines;
    m_debug_y_lines = debug_y_lines;
    m_debug_default_height = debug_default_height;
    m_debug_default_width = debug_default_width;
}

void graphics_scene::debug_draw_layouter_grid(QPainter* painter, const int x_from, const int x_to, const int y_from, const int y_to)
{   
    painter->setPen(QPen(Qt::magenta));
    for (qreal x : m_debug_x_lines) {
        QLine line(x, y_from, x, y_to);
        painter->drawLine(line);
    }
    for (qreal y : m_debug_y_lines) {
        QLine line(x_from, y, x_to, y);
        painter->drawLine(line);
    }
    painter->setPen(QPen(Qt::green));
    int x = m_debug_x_lines.last() + m_debug_default_width;
    while (x <= x_to)
    {
        QLine line(x, y_from, x, y_to);
        painter->drawLine(line);
        x += m_debug_default_width;
    }
    x = m_debug_x_lines.first() - m_debug_default_width;
    while (x >= x_from)
    {
        QLine line(x, y_from, x, y_to);
        painter->drawLine(line);
        x -= m_debug_default_width;
    }
    int y = m_debug_y_lines.last() + m_debug_default_height;
    while (y <= y_to)
    {
        QLine line(x_from, y, x_to, y);
        painter->drawLine(line);
        y += m_debug_default_height;
    }
    y = m_debug_y_lines.first() - m_debug_default_height;
    while (y >= y_from)
    {
        QLine line(x_from, y, x_to, y);
        painter->drawLine(line);
        y -= m_debug_default_height;
    }
}
#endif