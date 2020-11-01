#include "gui/graph_widget/graphics_scene.h"

#include "hal_core/utilities/log.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/grouping.h"

#include "gui/graph_widget/graph_widget_constants.h"
#include "gui/graph_widget/graphics_factory.h"
#include "gui/graph_widget/items/nodes/gates/graphics_gate.h"
#include "gui/graph_widget/items/graphics_item.h"
#include "gui/graph_widget/items/nodes/modules/graphics_module.h"
#include "gui/graph_widget/items/nets/graphics_net.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/grouping/grouping_table_model.h"
#include "gui/gui_globals.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QString>

#include <QDebug>

namespace hal
{
    qreal GraphicsScene::s_lod = 0;

    const qreal GraphicsScene::s_grid_fade_start = 0.4;
    const qreal GraphicsScene::s_grid_fade_end = 1.0;

    bool GraphicsScene::s_grid_enabled = true;
    bool GraphicsScene::s_grid_clusters_enabled = true;
    graph_widget_constants::grid_type GraphicsScene::s_grid_type = graph_widget_constants::grid_type::Lines;

    QColor GraphicsScene::s_grid_base_line_color = QColor(30, 30, 30);
    QColor GraphicsScene::s_grid_cluster_line_color = QColor(15, 15, 15);

    QColor GraphicsScene::s_grid_base_dot_color = QColor(25, 25, 25);
    QColor GraphicsScene::s_grid_cluster_dot_color = QColor(170, 160, 125);

    void GraphicsScene::set_lod(const qreal& lod)
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

    void GraphicsScene::set_grid_enabled(const bool& value)
    {
        s_grid_enabled = value;
    }

    void GraphicsScene::set_grid_clusters_enabled(const bool& value)
    {
        s_grid_clusters_enabled = value;
    }

    void GraphicsScene::set_grid_type(const graph_widget_constants::grid_type& grid_type)
    {
        s_grid_type = grid_type;
    }

    void GraphicsScene::set_grid_base_line_color(const QColor& color)
    {
        s_grid_base_line_color = color;
    }

    void GraphicsScene::set_grid_cluster_line_color(const QColor& color)
    {
        s_grid_cluster_line_color = color;
    }

    void GraphicsScene::set_grid_base_dot_color(const QColor& color)
    {
        s_grid_base_dot_color = color;
    }

    void GraphicsScene::set_grid_cluster_dot_color(const QColor& color)
    {
        s_grid_cluster_dot_color = color;
    }

    QPointF GraphicsScene::snap_to_grid(const QPointF& pos)
    {
        int adjusted_x = qRound(pos.x() / graph_widget_constants::grid_size) * graph_widget_constants::grid_size;
        int adjusted_y = qRound(pos.y() / graph_widget_constants::grid_size) * graph_widget_constants::grid_size;
        return QPoint(adjusted_x, adjusted_y);
    }

    GraphicsScene::GraphicsScene(QObject* parent) : QGraphicsScene(parent),
        m_drag_shadow_gate(new NodeDragShadow())
    {
        // FIND OUT IF MANUAL CHANGE TO DEPTH IS NECESSARY / INCREASES PERFORMANCE
        //m_scene.setBspTreeDepth(10);

        connect_all();
        g_selection_relay->register_sender(this, "GraphView");

        QGraphicsScene::addItem(m_drag_shadow_gate);

        #ifdef GUI_DEBUG_GRID
        m_debug_grid_enable = g_settings_manager->get("debug/grid").toBool();
        #endif
    }

    void GraphicsScene::start_drag_shadow(const QPointF& posF, const QSizeF& sizeF, const NodeDragShadow::drag_cue cue)
    {
        m_drag_shadow_gate->set_visual_cue(cue);
        m_drag_shadow_gate->start(posF, sizeF);
    }

    void GraphicsScene::move_drag_shadow(const QPointF& posF, const NodeDragShadow::drag_cue cue)
    {
        m_drag_shadow_gate->setPos(posF);
        m_drag_shadow_gate->set_visual_cue(cue);
    }

    void GraphicsScene::stop_drag_shadow()
    {
        m_drag_shadow_gate->stop();
    }

    QPointF GraphicsScene::drop_target()
    {
        return m_drag_shadow_gate->pos();
    }

    void GraphicsScene::add_item(GraphicsItem* item)
    {
        // SELECTION HAS TO BE UPDATED MANUALLY AFTER ADDING / REMOVING ITEMS

        if (!item)
            return;

        QGraphicsScene::addItem(item);

        switch (item->item_type())
        {
        case hal::item_type::gate:
        {
            GraphicsGate* g = static_cast<GraphicsGate*>(item);
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
            GraphicsNet* n = static_cast<GraphicsNet*>(item);
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
            GraphicsModule* m = static_cast<GraphicsModule*>(item);
            int i = 0;
            while (i < m_ModuleItems.size())
            {
                if (m->id() < m_ModuleItems.at(i).id)
                    break;

                i++;
            }
            m_ModuleItems.insert(i, module_data{m->id(), m});
            return;
        }
        }
    }

    void GraphicsScene::remove_item(GraphicsItem* item)
    {
        // SELECTION HAS TO BE UPDATED MANUALLY AFTER ADDING / REMOVING ITEMS

        if (!item)
            return;

        QGraphicsScene::removeItem(item);

        switch (item->item_type())
        {
        case hal::item_type::gate:
        {
            GraphicsGate* g = static_cast<GraphicsGate*>(item);
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
            GraphicsNet* n = static_cast<GraphicsNet*>(item);
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
            GraphicsModule* m = static_cast<GraphicsModule*>(item);
            u32 id = m->id();

            int i = 0;
            while (i < m_ModuleItems.size())
            {
                if (m_ModuleItems[i].id == id)
                {
                    m_ModuleItems.remove(i);
                    delete m;
                    return;
                }

                ++i;
            }

            return;
        }
        }
    }

    const GraphicsGate* GraphicsScene::get_gate_item(const u32 id) const
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

    const GraphicsNet* GraphicsScene::get_net_item(const u32 id) const
    {
        for (const net_data& d : m_net_items)
        {
            if (d.id > id)
                break;

            if (d.id == id)
                return d.item;
        }

        return nullptr;
    }

    const GraphicsModule* GraphicsScene::get_ModuleItem(const u32 id) const
    {
        for (const module_data& d : m_ModuleItems)
        {
            if (d.id > id)
                break;

            if (d.id == id)
                return d.item;
        }

        return nullptr;
    }

    void GraphicsScene::connect_all()
    {
        connect(g_settings_relay, &SettingsRelay::setting_changed, this, &GraphicsScene::handle_global_setting_changed);

        connect(this, &GraphicsScene::selectionChanged, this, &GraphicsScene::handle_intern_selection_changed);

        connect(g_selection_relay, &SelectionRelay::selection_changed, this, &GraphicsScene::handle_extern_selection_changed);
        connect(g_selection_relay, &SelectionRelay::subfocus_changed, this, &GraphicsScene::handle_extern_subfocus_changed);
        connect(g_netlist_relay, &NetlistRelay::grouping_module_assigned, this, &GraphicsScene::handleGroupingAssignModule);
        connect(g_netlist_relay, &NetlistRelay::grouping_module_removed, this, &GraphicsScene::handleGroupingAssignModule);
        connect(g_netlist_relay, &NetlistRelay::grouping_gate_assigned, this, &GraphicsScene::handleGroupingAssignGate);
        connect(g_netlist_relay, &NetlistRelay::grouping_gate_removed, this, &GraphicsScene::handleGroupingAssignGate);
        connect(g_netlist_relay, &NetlistRelay::grouping_net_assigned, this, &GraphicsScene::handleGroupingAssignNet);
        connect(g_netlist_relay, &NetlistRelay::grouping_net_removed, this, &GraphicsScene::handleGroupingAssignNet);
        connect(g_content_manager->getGroupingManagerWidget()->getModel(),&GroupingTableModel::groupingColorChanged,
                this, &GraphicsScene::handleGroupingColorChanged);
    }

    void GraphicsScene::disconnect_all()
    {
        disconnect(g_settings_relay, &SettingsRelay::setting_changed, this, &GraphicsScene::handle_global_setting_changed);

        disconnect(this, &GraphicsScene::selectionChanged, this, &GraphicsScene::handle_intern_selection_changed);

        disconnect(g_selection_relay, &SelectionRelay::selection_changed, this, &GraphicsScene::handle_extern_selection_changed);
        disconnect(g_selection_relay, &SelectionRelay::subfocus_changed, this, &GraphicsScene::handle_extern_subfocus_changed);
        disconnect(g_netlist_relay, &NetlistRelay::grouping_module_assigned, this, &GraphicsScene::handleGroupingAssignModule);
        disconnect(g_netlist_relay, &NetlistRelay::grouping_module_removed, this, &GraphicsScene::handleGroupingAssignModule);
        disconnect(g_netlist_relay, &NetlistRelay::grouping_gate_assigned, this, &GraphicsScene::handleGroupingAssignGate);
        disconnect(g_netlist_relay, &NetlistRelay::grouping_gate_removed, this, &GraphicsScene::handleGroupingAssignGate);
        disconnect(g_netlist_relay, &NetlistRelay::grouping_net_assigned, this, &GraphicsScene::handleGroupingAssignNet);
        disconnect(g_netlist_relay, &NetlistRelay::grouping_net_removed, this, &GraphicsScene::handleGroupingAssignNet);
        disconnect(g_content_manager->getGroupingManagerWidget()->getModel(),&GroupingTableModel::groupingColorChanged,
                   this, &GraphicsScene::handleGroupingColorChanged);
    }

    void GraphicsScene::delete_all_items()
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

        m_ModuleItems.clear();
        m_gate_items.clear();
        m_net_items.clear();
    }

    void GraphicsScene::update_visuals(const GraphShader::shading& s)
    {
        for (module_data& m : m_ModuleItems)
        {
            m.item->set_visuals(s.module_visuals.value(m.id));
        }

        for (gate_data& g : m_gate_items)
        {
            g.item->set_visuals(s.gate_visuals.value(g.id));
        }

        for (net_data& n : m_net_items)
        {
            n.item->set_visuals(s.net_visuals.value(n.id));
        }
    }

    void GraphicsScene::move_nets_to_background()
    {
        for (net_data d : m_net_items)
            d.item->setZValue(-1);
    }

    void GraphicsScene::handle_intern_selection_changed()
    {
        g_selection_relay->clear();

        int gates = 0;
        int nets = 0;
        int modules = 0;

        for (const QGraphicsItem* item : selectedItems())
        {
            switch (static_cast<const GraphicsItem*>(item)->item_type())
            {
            case hal::item_type::gate:
            {
                g_selection_relay->m_selected_gates.insert(static_cast<const GraphicsItem*>(item)->id());
                ++gates;
                break;
            }
            case hal::item_type::net:
            {
                g_selection_relay->m_selected_nets.insert(static_cast<const GraphicsItem*>(item)->id());
                ++nets;
                break;
            }
            case hal::item_type::module:
            {
                g_selection_relay->m_selected_modules.insert(static_cast<const GraphicsItem*>(item)->id());
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
                g_selection_relay->m_focus_type = SelectionRelay::item_type::gate;
                g_selection_relay->m_focus_id = *g_selection_relay->m_selected_gates.begin(); // UNNECESSARY ??? USE ARRAY[0] INSTEAD OF MEMBER VARIABLE ???
            }
            else if (nets)
            {
                g_selection_relay->m_focus_type = SelectionRelay::item_type::net;
                g_selection_relay->m_focus_id = *g_selection_relay->m_selected_nets.begin(); // UNNECESSARY ??? USE ARRAY[0] INSTEAD OF MEMBER VARIABLE ???
            }
            else
            {
                g_selection_relay->m_focus_type = SelectionRelay::item_type::module;
                g_selection_relay->m_focus_id = *g_selection_relay->m_selected_modules.begin(); // UNNECESSARY ??? USE ARRAY[0] INSTEAD OF MEMBER VARIABLE ???
            }
        }
        else
        {
            g_selection_relay->m_focus_type = SelectionRelay::item_type::none;
        }
        g_selection_relay->m_subfocus = SelectionRelay::subfocus::none;
        // END OF TEST CODE

        //LOG MANUAL SELECTION CHANGED:
        //log_info("gui", "Selection changed through manual interaction with a view to: insert here..");

        g_selection_relay->relay_selection_changed(this);
    }

    void GraphicsScene::handleGroupingAssignModule(Grouping *grp, u32 id)
    {
        Q_UNUSED(grp);

        GraphicsModule* gm = (GraphicsModule*) get_ModuleItem(id);
        if (gm) gm->update();
    }

    void GraphicsScene::handleGroupingAssignGate(Grouping *grp, u32 id)
    {
        Q_UNUSED(grp);

        GraphicsGate* gg = (GraphicsGate*) get_gate_item(id);
        if (gg) gg->update();
    }

    void GraphicsScene::handleGroupingAssignNet(Grouping *grp, u32 id)
    {
        Q_UNUSED(grp);

        GraphicsNet* gn = (GraphicsNet*) get_net_item(id);
        if (gn) gn->update();
    }

    void GraphicsScene::handleGroupingColorChanged(Grouping *grp)
    {
        for (const module_data& md : m_ModuleItems)
            if (grp->contains_module_by_id(md.id))
                md.item->update();
        for (const gate_data& gd : m_gate_items)
            if (grp->contains_gate_by_id(gd.id))
                gd.item->update();
        for (const net_data& nd : m_net_items)
            if (grp->contains_net_by_id(nd.id))
                nd.item->update();
    }

    void GraphicsScene::handleHighlight(const QVector<const SelectionTreeItem*>& highlightItems)
    {
        QSet<u32> highlightSet[SelectionTreeItem::MaxItem];
        for (const SelectionTreeItem* sti : highlightItems)
        {
            if (sti) highlightSet[sti->itemType()].insert(sti->id());
        }

        for (const module_data& mdata :  m_ModuleItems)
            mdata.item->setHightlight(highlightSet[SelectionTreeItem::ModuleItem].contains(mdata.id));
        for (const gate_data& gdata :  m_gate_items)
            gdata.item->setHightlight(highlightSet[SelectionTreeItem::GateItem].contains(gdata.id));
        for (const net_data& ndata :  m_net_items)
            ndata.item->setHightlight(highlightSet[SelectionTreeItem::NetItem].contains(ndata.id));
    }

    void GraphicsScene::handle_extern_selection_changed(void* sender)
    {
        // CLEAR CURRENT SELECTION EITHER MANUALLY OR USING clearSelection()
        // UNCERTAIN ABOUT THE SENDER PARAMETER

        if (sender == this)
            return;

        bool original_value = blockSignals(true);

        clearSelection();

        if (!g_selection_relay->m_selected_modules.isEmpty())
        {
            for (auto& element : m_ModuleItems)
            {
                if (g_selection_relay->isModuleSelected(element.id))
                {
                    element.item->setSelected(true);
                    element.item->update();
                }
            }
        }

        if (!g_selection_relay->m_selected_gates.isEmpty())
        {
            for (auto& element : m_gate_items)
            {
                if (g_selection_relay->isGateSelected(element.id))
                {
                    element.item->setSelected(true);
                    element.item->update();
                }
            }
        }

        if (!g_selection_relay->m_selected_nets.isEmpty())
        {
            for (auto& element : m_net_items)
            {
                if (g_selection_relay->isNetSelected(element.id))
                {
                    element.item->setSelected(true);
                    element.item->update();
                }
            }
        }

        blockSignals(original_value);
    }

    void GraphicsScene::handle_extern_subfocus_changed(void* sender)
    {
        Q_UNUSED(sender)
    }

    void GraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
    {
        // CONTEXT MENU CLEARING SELECTION WORKAROUND
        if (event->button() == Qt::RightButton)
        {
            event->accept();
            return;
        }

        QGraphicsScene::mousePressEvent(event);
    }

    void GraphicsScene::handle_global_setting_changed(void* sender, const QString& key, const QVariant& value)
    {
        Q_UNUSED(sender)

        #ifdef GUI_DEBUG_GRID
        if (key == "debug/grid")
        {
            m_debug_grid_enable = value.toBool();
        }
        #endif
    }

    void GraphicsScene::drawBackground(QPainter* painter, const QRectF& rect)
    {
        if (!s_grid_enabled)
            return;

        if (s_lod < s_grid_fade_start)
            return;

        QFlags original_flags = painter->renderHints(); // UNNECESSARY ?
        painter->setRenderHint(QPainter::Antialiasing, true);

        QPen pen;
        pen.setWidth(2);

        // OVERDRAW NEEDED BECAUSE QT FAILS AT BASIC geometry
        const int overdraw = 2;

        const int x_from = rect.left() - overdraw;
        const int x_to = rect.right() + overdraw;

        const int y_from = rect.top() - overdraw;
        const int y_to = rect.bottom() + overdraw;

        const int x_offset = x_from % graph_widget_constants::grid_size;
        const int y_offset = y_from % graph_widget_constants::grid_size;

        switch (s_grid_type)
        {
        case graph_widget_constants::grid_type::None:
            return; // nothing to do
        case graph_widget_constants::grid_type::Lines:
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

        case graph_widget_constants::grid_type::Dots:
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
    void GraphicsScene::debug_set_layouter_grid(const QVector<qreal>& debug_x_lines, const QVector<qreal>& debug_y_lines, qreal debug_default_height, qreal debug_default_width)
    {
        m_debug_x_lines = debug_x_lines;
        m_debug_y_lines = debug_y_lines;
        m_debug_default_height = debug_default_height;
        m_debug_default_width = debug_default_width;
    }

    void GraphicsScene::debug_draw_layouter_grid(QPainter* painter, const int x_from, const int x_to, const int y_from, const int y_to)
    {
        painter->setPen(QPen(Qt::magenta));

        for (qreal x : m_debug_x_lines)
        {
            QLineF line(x, y_from, x, y_to);
            painter->drawLine(line);
        }

        for (qreal y : m_debug_y_lines)
        {
            QLineF line(x_from, y, x_to, y);
            painter->drawLine(line);
        }

        painter->setPen(QPen(Qt::green));

        qreal x = m_debug_x_lines.last() + m_debug_default_width;

        while (x <= x_to)
        {
            QLineF line(x, y_from, x, y_to);
            painter->drawLine(line);
            x += m_debug_default_width;
        }

        x = m_debug_x_lines.first() - m_debug_default_width;

        while (x >= x_from)
        {
            QLineF line(x, y_from, x, y_to);
            painter->drawLine(line);
            x -= m_debug_default_width;
        }

        qreal y = m_debug_y_lines.last() + m_debug_default_height;

        while (y <= y_to)
        {
            QLineF line(x_from, y, x_to, y);
            painter->drawLine(line);
            y += m_debug_default_height;
        }

        y = m_debug_y_lines.first() - m_debug_default_height;

        while (y >= y_from)
        {
            QLineF line(x_from, y, x_to, y);
            painter->drawLine(line);
            y -= m_debug_default_height;
        }
    }
    #endif
}
