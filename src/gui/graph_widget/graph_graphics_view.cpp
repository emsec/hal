#include "graph_widget/graph_graphics_view.h"

#include "core/log.h"
#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_widget/graph_widget.h"
#include "gui/graph_widget/graph_widget_constants.h"
#include "gui/graph_widget/graphics_scene.h"
#include "gui/graph_widget/graphics_view_zoom.h"
#include "gui/graph_widget/items/graphics_gate.h"
#include "gui/graph_widget/items/graphics_item.h"
#include "gui/graph_widget/items/io_graphics_net.h"
#include "gui/graph_widget/items/separated_graphics_net.h"
#include "gui/graph_widget/items/standard_graphics_gate.h"
#include "gui/graph_widget/items/standard_graphics_module.h"
#include "gui/graph_widget/items/standard_graphics_net.h"
#include "gui/graph_widget/items/utility_items/drag_shadow_gate.h"
#include "gui/gui_globals.h"
#include "netlist/gate.h"
#include "netlist/module.h"
#include "netlist/net.h"

#include <QApplication>
#include <QAction>
#include <QColorDialog>
#include <QDebug>
#include <QDrag>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMimeData>
#include <QScrollBar>
#include <QStyleOptionGraphicsItem>
#include <QWheelEvent>
#include <QWidgetAction>
#include <qmath.h>

graph_graphics_view::graph_graphics_view(graph_widget* parent)
    : QGraphicsView(parent), m_minimap_enabled(false), m_antialiasing_enabled(false), m_cosmetic_nets_enabled(false), m_grid_enabled(true), m_grid_clusters_enabled(true),
      m_grid_type(graph_widget_constants::grid_type::lines)
{
    m_graph_widget = parent;

    connect(&g_selection_relay, &selection_relay::subfocus_changed, this, &graph_graphics_view::conditional_update);
    connect(this, &graph_graphics_view::customContextMenuRequested, this, &graph_graphics_view::show_context_menu);

    setContextMenuPolicy(Qt::CustomContextMenu);
    setOptimizationFlags(QGraphicsView::DontSavePainterState);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    setAcceptDrops(true);

    graphics_view_zoom* z = new graphics_view_zoom(this);
    z->set_modifiers(Qt::NoModifier);
}

void graph_graphics_view::conditional_update()
{
    if (QStyleOptionGraphicsItem::levelOfDetailFromTransform(transform()) >= graph_widget_constants::gate_min_lod)
        update();
}

void graph_graphics_view::handle_change_color_action()
{
    QColor color = QColorDialog::getColor();

    if (!color.isValid())
        return;
}

void graph_graphics_view::handle_isolation_view_action()
{
    u32 cnt = 0;
    while (true)
    {
        ++cnt;
        QString name = "Isolated View " + QString::number(cnt);
        if (!g_graph_context_manager.context_with_name_exists(name))
        {
            auto context = g_graph_context_manager.create_new_context(name);
            context->add(g_selection_relay.m_selected_modules, g_selection_relay.m_selected_gates);
            return;
        }
    }
}

//////////
// If we have time:
// This class should NOT directly perform any of these actions.
// There should be a global manager for this.

void graph_graphics_view::handle_move_action(QAction* action)
{
    const u32 mod_id          = action->data().toInt();
    std::shared_ptr<module> m = g_netlist->get_module_by_id(mod_id);
    for (const auto& id : g_selection_relay.m_selected_gates)
    {
        m->assign_gate(g_netlist->get_gate_by_id(id));
    }
    for (const auto& id : g_selection_relay.m_selected_modules)
    {
        g_netlist->get_module_by_id(id)->set_parent_module(m);
    }

    auto gates   = g_selection_relay.m_selected_gates;
    auto modules = g_selection_relay.m_selected_modules;
    g_selection_relay.clear();
    g_selection_relay.relay_selection_changed(this);

    auto context = m_graph_widget->get_context();
    context->begin_change();
    context->remove(modules, gates);
    context->end_change();
}

void graph_graphics_view::handle_move_new_action()
{
    bool ok;
    QString name = QInputDialog::getText(nullptr, "", "Module Name:", QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty())
        return;
    // There is no easy way to allow the user to create a submodule on the fly here,
    // so this creates a new module under the top-module.
    std::shared_ptr<module> m = g_netlist->create_module(g_netlist->get_unique_module_id(), name.toStdString(), g_netlist->get_top_module());

    for (const auto& id : g_selection_relay.m_selected_gates)
    {
        m->assign_gate(g_netlist->get_gate_by_id(id));
    }
    for (const auto& id : g_selection_relay.m_selected_modules)
    {
        g_netlist->get_module_by_id(id)->set_parent_module(m);
    }

    auto gates   = g_selection_relay.m_selected_gates;
    auto modules = g_selection_relay.m_selected_modules;
    g_selection_relay.clear();
    g_selection_relay.relay_selection_changed(this);

    auto context = m_graph_widget->get_context();
    context->begin_change();
    context->remove(modules, gates);
    context->add({m->get_id()}, {});
    context->end_change();
}

void graph_graphics_view::handle_rename_action()
{
    if (m_item->item_type() == hal::item_type::gate)
    {
        std::shared_ptr<gate> g = g_netlist->get_gate_by_id(m_item->id());
        const QString name      = QString::fromStdString(g->get_name());
        bool confirm;
        const QString new_name = QInputDialog::getText(this, "Rename gate", "New name:", QLineEdit::Normal, name, &confirm);
        if (confirm)
        {
            g->set_name(new_name.toStdString());
        }
    }
    else if (m_item->item_type() == hal::item_type::module)
    {
        std::shared_ptr<module> m = g_netlist->get_module_by_id(m_item->id());
        const QString name        = QString::fromStdString(m->get_name());
        bool confirm;
        const QString new_name = QInputDialog::getText(this, "Rename module", "New name:", QLineEdit::Normal, name, &confirm);
        if (confirm)
        {
            m->set_name(new_name.toStdString());
        }
    }
}
//////////

void graph_graphics_view::adjust_min_scale()
{
    if (!scene())
        return;

    m_min_scale = std::min(viewport()->width() / scene()->width(), viewport()->height() / scene()->height());
}

void graph_graphics_view::paintEvent(QPaintEvent* event)
{
    qreal lod = QStyleOptionGraphicsItem::levelOfDetailFromTransform(transform());

    // USE CONSISTENT METHOD NAMES
    graphics_scene::set_lod(lod);
    graphics_scene::set_grid_enabled(m_grid_enabled);
    graphics_scene::set_grid_clusters_enabled(m_grid_clusters_enabled);
    graphics_scene::set_grid_type(m_grid_type);

    graphics_item::set_lod(lod);
    drag_shadow_gate::set_lod(lod);

    standard_graphics_module::update_alpha();
    standard_graphics_gate::update_alpha();
    standard_graphics_net::update_alpha();
    separated_graphics_net::update_alpha();
    io_graphics_net::update_alpha();

    QGraphicsView::paintEvent(event);
}

void graph_graphics_view::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
        return;

    graphics_item* item = static_cast<graphics_item*>(itemAt(event->pos()));

    if (!item)
        return;

    if (item->item_type() == hal::item_type::module)
        Q_EMIT module_double_clicked(item->id());
}

void graph_graphics_view::drawForeground(QPainter* painter, const QRectF& rect)
{
    Q_UNUSED(rect)

    if (!m_minimap_enabled)
        return;

    QRectF map(viewport()->width() - 210, viewport()->height() - 130, 200, 120);
    painter->resetTransform();
    painter->fillRect(map, QColor(0, 0, 0, 170));
}

void graph_graphics_view::mousePressEvent(QMouseEvent* event)
{
    if (event->modifiers() == Qt::ShiftModifier)
    {
        if (event->button() == Qt::LeftButton)
            m_move_position = event->pos();
    }
    else if (event->button() == Qt::MidButton)
    {
        m_zoom_scene_position = mapToScene(event->pos());
        // HIDE CURSOR
        // SHOW DUMMY ???
        // ZOOM
        // MOVE ACTUAL CURSOR TO FINAL POSITION
        // SHOW ACTUAL CURSOR

        QCursor cursor(Qt::BlankCursor);
        setCursor(cursor);
    }
    else if (event->button() == Qt::LeftButton)
    {
        graphics_item* item = static_cast<graphics_item*>(itemAt(event->pos()));
        if (item && item_draggable(item))
        {
            m_drag_item = static_cast<graphics_gate*>(item);
            m_drag_mousedown_position = event->pos();
            m_drag_cursor_offset = m_drag_mousedown_position - mapFromScene(item->pos());
        }
        else
        {
            m_drag_item = nullptr;
        }

        // we still need the normal mouse logic for single clicks
        QGraphicsView::mousePressEvent(event);
    }
    else
        QGraphicsView::mousePressEvent(event);
}

void graph_graphics_view::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MidButton)
    {
        QCursor cursor;
        cursor.setPos(mapToGlobal(mapFromScene(m_zoom_scene_position)));
        setCursor(cursor);
    }
    else
        QGraphicsView::mouseReleaseEvent(event);
}

void graph_graphics_view::mouseMoveEvent(QMouseEvent* event)
{
    if (!scene())
        return;

    if (event->buttons().testFlag(Qt::LeftButton))
    {
        if (event->modifiers() == Qt::ShiftModifier)
        {
            QScrollBar* hBar = horizontalScrollBar();
            QScrollBar* vBar = verticalScrollBar();
            QPoint delta     = event->pos() - m_move_position;
            m_move_position  = event->pos();
            hBar->setValue(hBar->value() + (isRightToLeft() ? delta.x() : -delta.x()));
            vBar->setValue(vBar->value() - delta.y());
        }
        else
        {
            if (m_drag_item && (event->pos() - m_drag_mousedown_position).manhattanLength() >= QApplication::startDragDistance())
            {
                QDrag *drag = new QDrag(this);
                QMimeData *mimeData = new QMimeData;

                // TODO set MIME type and icon
                mimeData->setText("dragTest");
                drag->setMimeData(mimeData);
                // drag->setPixmap(iconPixmap);

                // enable DragMoveEvents until mouse released
                drag->exec(Qt::MoveAction);
            }
        }
    }
    QGraphicsView::mouseMoveEvent(event);
}

void graph_graphics_view::dragEnterEvent(QDragEnterEvent *event)
{
    qDebug() << "dragEnter";
    if (event->source() == this && event->proposedAction() == Qt::MoveAction)
    {
        event->acceptProposedAction();
        QSizeF size(m_drag_item->width(), m_drag_item->height());
        QPointF pos = m_drag_item->scenePos();
        static_cast<graphics_scene*>(scene())
            ->start_drag_shadow(pos, size, m_drag_item);
        // Process the data from the event.
    }
    else
    {
        QGraphicsView::dragEnterEvent(event);
    }
}

void graph_graphics_view::dragLeaveEvent(QDragLeaveEvent *event)
{
    static_cast<graphics_scene*>(scene())->stop_drag_shadow();
    //QGraphicsView::dragLeaveEvent(event);
}

void graph_graphics_view::dragMoveEvent(QDragMoveEvent *event)
{
    QPoint mouse = event->pos();
    QPoint shadow = mouse - m_drag_cursor_offset;
    static_cast<graphics_scene*>(scene())
        ->move_drag_shadow(mapToScene(shadow.x(), shadow.y()));
    // event->acceptProposedAction();
    //QGraphicsView::dragMoveEvent(event);
}

void graph_graphics_view::dropEvent(QDropEvent *event)
{
    if (event->source() == this && event->proposedAction() == Qt::MoveAction)
    {
        event->acceptProposedAction();
        graphics_scene* s = static_cast<graphics_scene*>(scene());
        bool success = s->stop_drag_shadow();
        if (success)
            m_drag_item->setPos(s->drop_target());
        // Process the data from the event.
    }
    else
    {
        QGraphicsView::dropEvent(event);
    }
}

void graph_graphics_view::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
        case Qt::Key_Space:
        {
            //qDebug() << "Space pressed";
        }
        break;
    }

    event->ignore();
}

void graph_graphics_view::keyReleaseEvent(QKeyEvent* event)
{
    switch (event->key())
    {
        case Qt::Key_Space:
        {
            //qDebug() << "Space released";
        }
        break;
    }

    event->ignore();
}

void graph_graphics_view::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event)

    adjust_min_scale();
}

void graph_graphics_view::show_context_menu(const QPoint& pos)
{
    graphics_scene* s = static_cast<graphics_scene*>(scene());

    if (!s)
        return;

    QMenu context_menu(this);
    QAction* action;

    QGraphicsItem* item = itemAt(pos);
    if (item)
    {
        m_item = static_cast<graphics_item*>(item);

        if (m_item->item_type() == hal::item_type::gate)
        {
            if (g_selection_relay.m_selected_gates.find(m_item->id()) == g_selection_relay.m_selected_gates.end())
            {
                g_selection_relay.clear();
                g_selection_relay.m_selected_gates.insert(m_item->id());
                g_selection_relay.m_focus_type = selection_relay::item_type::gate;
                g_selection_relay.m_focus_id   = m_item->id();
                g_selection_relay.m_subfocus   = selection_relay::subfocus::none;
                g_selection_relay.relay_selection_changed(this);
            }

            context_menu.addAction("This gate:")->setEnabled(false);

            action = context_menu.addAction("  Rename …");
            QObject::connect(action, &QAction::triggered, this, &graph_graphics_view::handle_rename_action);

            action = context_menu.addAction("  Fold parent module");
            QObject::connect(action, &QAction::triggered, this, &graph_graphics_view::handle_fold_single_action);
        }
        else if (m_item->item_type() == hal::item_type::module)
        {
            if (g_selection_relay.m_selected_modules.find(m_item->id()) == g_selection_relay.m_selected_modules.end())
            {
                g_selection_relay.clear();
                g_selection_relay.m_selected_modules.insert(m_item->id());
                g_selection_relay.m_focus_type = selection_relay::item_type::module;
                g_selection_relay.m_focus_id   = m_item->id();
                g_selection_relay.m_subfocus   = selection_relay::subfocus::none;
                g_selection_relay.relay_selection_changed(this);
            }

            context_menu.addAction("This module:")->setEnabled(false);

            action = context_menu.addAction("  Rename …");
            QObject::connect(action, &QAction::triggered, this, &graph_graphics_view::handle_rename_action);

            action = context_menu.addAction("  Unfold module");
            QObject::connect(action, &QAction::triggered, this, &graph_graphics_view::handle_unfold_single_action);
        }

        if (g_selection_relay.m_selected_gates.size() + g_selection_relay.m_selected_modules.size() > 1)
        {
            context_menu.addSeparator();
            context_menu.addAction("Entire selection:")->setEnabled(false);
        }

        action = context_menu.addAction("  Isolate In New View");
        QObject::connect(action, &QAction::triggered, this, &graph_graphics_view::handle_isolation_view_action);

        action = context_menu.addAction("  Add successors to view");
        connect(action, &QAction::triggered, this, &graph_graphics_view::handle_select_outputs);

        action = context_menu.addAction("  Add predecessors to view");
        connect(action, &QAction::triggered, this, &graph_graphics_view::handle_select_inputs);

        QMenu* module_submenu = context_menu.addMenu("  Move to module …");

        action = module_submenu->addAction("  New module …");
        QObject::connect(action, &QAction::triggered, this, &graph_graphics_view::handle_move_new_action);
        module_submenu->addSeparator();

        QActionGroup* module_actions = new QActionGroup(module_submenu);
        for (auto& module : g_netlist->get_modules())
        {
            std::shared_ptr<gate> g = g_netlist->get_gate_by_id(m_item->id());
            if (!module->contains_gate(g))
            {
                QString mod_name = QString::fromStdString(module->get_name());
                const u32 mod_id = module->get_id();
                action           = module_submenu->addAction(mod_name);
                module_actions->addAction(action);
                action->setData(mod_id);
            }
        }
        QObject::connect(module_actions, SIGNAL(triggered(QAction*)), this, SLOT(handle_move_action(QAction*)));

        if (g_selection_relay.m_selected_gates.size() + g_selection_relay.m_selected_modules.size() > 1)
        {
            if (!g_selection_relay.m_selected_gates.empty())
            {
                context_menu.addSeparator();
                context_menu.addAction("All selected gates:")->setEnabled(false);

                action = context_menu.addAction("  Fold all parent modules");
                QObject::connect(action, &QAction::triggered, this, &graph_graphics_view::handle_fold_all_action);
            }
            if (!g_selection_relay.m_selected_modules.empty())
            {
                context_menu.addSeparator();
                context_menu.addAction("All selected modules:")->setEnabled(false);

                action = context_menu.addAction("  Unfold all");
                QObject::connect(action, &QAction::triggered, this, &graph_graphics_view::handle_unfold_all_action);
            }
        }
    }
    else
    {
        QAction* antialiasing_action = context_menu.addAction("Antialiasing");
        QAction* cosmetic_action     = context_menu.addAction("Cosmetic Nets");
        QMenu* grid_menu             = context_menu.addMenu("Grid");
        QMenu* type_menu             = grid_menu->addMenu("Type");
        QMenu* cluster_menu          = grid_menu->addMenu("Clustering");
        QAction* lines_action        = type_menu->addAction("Lines");
        QAction* dots_action         = type_menu->addAction("Dots");
        QAction* none_action         = type_menu->addAction("None");
        //connect(action, &QAction::triggered, this, SLOT);
    }

    context_menu.exec(mapToGlobal(pos));
    update();
}

void graph_graphics_view::update_matrix(const int delta)
{
    qreal scale = qPow(2.0, delta / 100.0);

    QMatrix matrix;
    matrix.scale(scale, scale);
    setMatrix(matrix);
}

void graph_graphics_view::toggle_antialiasing()
{
    setRenderHint(QPainter::Antialiasing, !(renderHints() & QPainter::Antialiasing));
}

bool graph_graphics_view::item_draggable(graphics_item* item)
{
    hal::item_type type = item->item_type();
    return type == hal::item_type::gate || type == hal::item_type::module;
}

void graph_graphics_view::handle_select_outputs()
{
    auto context           = m_graph_widget->get_context();
    QAction* sender_action = dynamic_cast<QAction*>(sender());
    if (sender_action)
    {
        QSet<u32> gates;
        for (auto id : g_selection_relay.m_selected_gates)
        {
            auto gate = g_netlist->get_gate_by_id(id);
            for (const auto& net : gate->get_fan_out_nets())
            {
                for (const auto& suc : net->get_dsts())
                {
                    bool found = false;
                    for (const auto& id : context->modules())
                    {
                        auto m = g_netlist->get_module_by_id(id);
                        if (m->contains_gate(suc.gate, true))
                        {
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                    {
                        gates.insert(suc.gate->get_id());
                    }
                }
            }
        }
        for (auto id : g_selection_relay.m_selected_modules)
        {
            auto module = g_netlist->get_module_by_id(id);
            for (const auto& net : module->get_output_nets())
            {
                for (const auto& suc : net->get_dsts())
                {
                    bool found = false;
                    for (const auto& id : context->modules())
                    {
                        auto m = g_netlist->get_module_by_id(id);
                        if (m->contains_gate(suc.gate, true))
                        {
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                    {
                        gates.insert(suc.gate->get_id());
                    }
                }
            }
        }

        context->add({}, gates);
    }
}
void graph_graphics_view::handle_select_inputs()
{
    auto context           = m_graph_widget->get_context();
    QAction* sender_action = dynamic_cast<QAction*>(sender());
    if (sender_action)
    {
        QSet<u32> gates;
        for (auto id : g_selection_relay.m_selected_gates)
        {
            auto gate = g_netlist->get_gate_by_id(id);
            for (const auto& net : gate->get_fan_in_nets())
            {
                if (net->get_src().gate != nullptr)
                {
                    bool found = false;
                    for (const auto& id : context->modules())
                    {
                        auto m = g_netlist->get_module_by_id(id);
                        if (m->contains_gate(net->get_src().gate, true))
                        {
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                    {
                        gates.insert(net->get_src().gate->get_id());
                    }
                }
            }
        }
        for (auto id : g_selection_relay.m_selected_modules)
        {
            auto module = g_netlist->get_module_by_id(id);
            for (const auto& net : module->get_input_nets())
            {
                if (net->get_src().gate != nullptr)
                {
                    bool found = false;
                    for (const auto& id : context->modules())
                    {
                        auto m = g_netlist->get_module_by_id(id);
                        if (m->contains_gate(net->get_src().gate, true))
                        {
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                    {
                        gates.insert(net->get_src().gate->get_id());
                    }
                }
            }
        }
        context->add({}, gates);
    }
}

void graph_graphics_view::handle_fold_single_action()
{
    auto context = m_graph_widget->get_context();
    context->fold_module_of_gate(m_item->id());
}

void graph_graphics_view::handle_unfold_single_action()
{
    auto context = m_graph_widget->get_context();
    context->unfold_module(m_item->id());
}

void graph_graphics_view::handle_fold_all_action()
{
    auto context = m_graph_widget->get_context();

    context->begin_change();
    for (u32 id : g_selection_relay.m_selected_gates)
    {
        context->fold_module_of_gate(id);
    }
    context->end_change();
}

void graph_graphics_view::handle_unfold_all_action()
{
    auto context = m_graph_widget->get_context();

    context->begin_change();
    for (u32 id : g_selection_relay.m_selected_modules)
    {
        context->unfold_module(id);
    }
    context->end_change();
}
