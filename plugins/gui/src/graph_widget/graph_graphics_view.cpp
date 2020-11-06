#include "gui/graph_widget/graph_graphics_view.h"

#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_widget/graph_widget.h"
#include "gui/graph_widget/graph_widget_constants.h"
#include "gui/graph_widget/graphics_scene.h"
#include "gui/graph_widget/items/graphics_item.h"
#include "gui/graph_widget/items/nets/separated_graphics_net.h"
#include "gui/graph_widget/items/nets/standard_graphics_net.h"
#include "gui/graph_widget/items/nodes/gates/graphics_gate.h"
#include "gui/graph_widget/items/nodes/gates/standard_graphics_gate.h"
#include "gui/graph_widget/items/nodes/modules/standard_graphics_module.h"
#include "gui/graph_widget/items/utility_items/node_drag_shadow.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/grouping/grouping_table_model.h"
#include "gui/gui_globals.h"
#include "gui/gui_utils/netlist.h"
#include "gui/implementations/qpoint_extension.h"
#include "gui/selection_details_widget/selection_details_widget.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/utilities/log.h"

#include <QAction>
#include <QApplication>
#include <QColorDialog>
#include <QDebug>
#include <QDrag>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QScrollBar>
#include <QStyleOptionGraphicsItem>
#include <QWheelEvent>
#include <QWidgetAction>
#include <algorithm>
#include <qmath.h>

const QString ASSIGN_TO_GROUPING("Assign to grouping ");

namespace hal
{
    GraphGraphicsView::GraphGraphicsView(GraphWidget* parent)
        : QGraphicsView(parent), m_graph_widget(parent), m_minimap_enabled(false), m_grid_enabled(true), m_grid_clusters_enabled(true), m_grid_type(graph_widget_constants::grid_type::Lines),
          m_zoom_modifier(Qt::NoModifier), m_zoom_factor_base(1.0015)
    {
        connect(g_selection_relay, &SelectionRelay::subfocus_changed, this, &GraphGraphicsView::conditional_update);
        connect(this, &GraphGraphicsView::customContextMenuRequested, this, &GraphGraphicsView::show_context_menu);
        connect(g_settings_relay, &SettingsRelay::setting_changed, this, &GraphGraphicsView::handle_global_setting_changed);

        initialize_settings();

        setContextMenuPolicy(Qt::CustomContextMenu);
        setOptimizationFlags(QGraphicsView::DontSavePainterState);
        setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

        setAcceptDrops(true);
        setMouseTracking(true);
    }

    void GraphGraphicsView::initialize_settings()
    {
        unsigned int drag_modifier_setting = g_settings_manager->get("graph_view/drag_mode_modifier").toUInt();
        m_drag_modifier                    = Qt::KeyboardModifier(drag_modifier_setting);
        unsigned int move_modifier_setting = g_settings_manager->get("graph_view/move_modifier").toUInt();
        m_move_modifier                    = Qt::KeyboardModifier(move_modifier_setting);

        // might think about Q_ENUM to avoid separate enum and config file tokens
        const char* gridTypeNames[] = {"lines", "dots", "none", 0};

        QString sGridType = g_settings_manager->get("graph_view/grid_type").toString();
        for (int i = 0; gridTypeNames[i]; i++)
            if (sGridType == gridTypeNames[i])
            {
                m_grid_type = static_cast<graph_widget_constants::grid_type>(i);
                break;
            }

#ifdef GUI_DEBUG_GRID
        m_debug_gridpos_enable = g_settings_manager->get("debug/grid").toBool();
#endif
    }

    void GraphGraphicsView::conditional_update()
    {
        if (QStyleOptionGraphicsItem::levelOfDetailFromTransform(transform()) >= graph_widget_constants::gate_min_lod)
            update();
    }

    void GraphGraphicsView::handle_change_color_action()
    {
        QColor color = QColorDialog::getColor();

        if (!color.isValid())
            return;
    }

    void GraphGraphicsView::handle_isolation_view_action()
    {
        u32 cnt = 0;
        while (true)
        {
            ++cnt;
            QString name = "Isolated View " + QString::number(cnt);
            if (!g_graph_context_manager->context_with_name_exists(name))
            {
                auto context = g_graph_context_manager->create_new_context(name);
                context->add(g_selection_relay->m_selected_modules, g_selection_relay->m_selected_gates);
                return;
            }
        }
    }

    void GraphGraphicsView::handle_move_action(QAction* action)
    {
        const u32 mod_id = action->data().toInt();
        Module* m        = g_netlist->get_module_by_id(mod_id);
        for (const auto& id : g_selection_relay->m_selected_gates)
        {
            m->assign_gate(g_netlist->get_gate_by_id(id));
        }
        for (const auto& id : g_selection_relay->m_selected_modules)
        {
            g_netlist->get_module_by_id(id)->set_parent_module(m);
        }

        auto gates   = g_selection_relay->m_selected_gates;
        auto modules = g_selection_relay->m_selected_modules;
        g_selection_relay->clear();
        g_selection_relay->relay_selection_changed(this);
    }

    void GraphGraphicsView::handle_move_new_action()
    {
        std::unordered_set<Gate*> gate_objs;
        std::unordered_set<Module*> module_objs;
        for (const auto& id : g_selection_relay->m_selected_gates)
        {
            gate_objs.insert(g_netlist->get_gate_by_id(id));
        }
        for (const auto& id : g_selection_relay->m_selected_modules)
        {
            module_objs.insert(g_netlist->get_module_by_id(id));
        }
        Module* parent      = gui_utility::first_common_ancestor(module_objs, gate_objs);
        QString parent_name = QString::fromStdString(parent->get_name());
        bool ok;
        QString name = QInputDialog::getText(nullptr, "", "New module will be created under \"" + parent_name + "\"\nModule Name:", QLineEdit::Normal, "", &ok);
        if (!ok || name.isEmpty())
            return;
        Module* m = g_netlist->create_module(g_netlist->get_unique_module_id(), name.toStdString(), parent);

        for (const auto& id : g_selection_relay->m_selected_gates)
        {
            m->assign_gate(g_netlist->get_gate_by_id(id));
        }
        for (const auto& id : g_selection_relay->m_selected_modules)
        {
            g_netlist->get_module_by_id(id)->set_parent_module(m);
        }

        auto gates   = g_selection_relay->m_selected_gates;
        auto modules = g_selection_relay->m_selected_modules;
        g_selection_relay->clear();
        g_selection_relay->relay_selection_changed(this);
    }

    void GraphGraphicsView::handle_rename_action()
    {
        if (m_item->item_type() == item_type::Gate)
        {
            Gate* g            = g_netlist->get_gate_by_id(m_item->id());
            const QString name = QString::fromStdString(g->get_name());
            bool confirm;
            const QString new_name = QInputDialog::getText(this, "Change gate name", "New name:", QLineEdit::Normal, name, &confirm);
            if (confirm)
            {
                g->set_name(new_name.toStdString());
            }
        }
        else if (m_item->item_type() == item_type::Module)
        {
            Module* m          = g_netlist->get_module_by_id(m_item->id());
            const QString name = QString::fromStdString(m->get_name());
            bool confirm;
            const QString new_name = QInputDialog::getText(this, "Change module name", "New name:", QLineEdit::Normal, name, &confirm);
            if (confirm)
            {
                m->set_name(new_name.toStdString());
            }
        }
        else if (m_item->item_type() == item_type::Net)
        {
            Net* n             = g_netlist->get_net_by_id(m_item->id());
            const QString name = QString::fromStdString(n->get_name());
            bool confirm;
            const QString new_name = QInputDialog::getText(this, "Change net name", "New name:", QLineEdit::Normal, name, &confirm);
            if (confirm)
            {
                n->set_name(new_name.toStdString());
            }
        }
    }

    void GraphGraphicsView::handle_change_type_action()
    {
        if (m_item->item_type() == item_type::Module)
        {
            Module* m          = g_netlist->get_module_by_id(m_item->id());
            const QString type = QString::fromStdString(m->get_type());
            bool confirm;
            const QString new_type = QInputDialog::getText(this, "Change module type", "New type:", QLineEdit::Normal, type, &confirm);
            if (confirm)
            {
                m->set_type(new_type.toStdString());
            }
        }
    }

    void GraphGraphicsView::adjust_min_scale()
    {
        if (!scene())
            return;
        m_min_scale = std::min(viewport()->width() / scene()->width(), viewport()->height() / scene()->height());
    }

    void GraphGraphicsView::paintEvent(QPaintEvent* event)
    {
        qreal lod = QStyleOptionGraphicsItem::levelOfDetailFromTransform(transform());

        // USE CONSISTENT METHOD NAMES
        GraphicsScene::set_lod(lod);
        GraphicsScene::set_grid_enabled(m_grid_enabled);
        GraphicsScene::set_grid_clusters_enabled(m_grid_clusters_enabled);
        GraphicsScene::set_grid_type(m_grid_type);

        GraphicsItem::set_lod(lod);
        NodeDragShadow::set_lod(lod);

        StandardGraphicsModule::update_alpha();
        StandardGraphicsGate::update_alpha();
        StandardGraphicsNet::update_alpha();
        SeparatedGraphicsNet::update_alpha();

        QGraphicsView::paintEvent(event);
    }

    void GraphGraphicsView::mouseDoubleClickEvent(QMouseEvent* event)
    {
        if (event->button() != Qt::LeftButton)
            return;

        GraphicsItem* item = static_cast<GraphicsItem*>(itemAt(event->pos()));

        if (!item)
            return;

        if (item->item_type() == item_type::Module)
            Q_EMIT module_double_clicked(item->id());
    }

    void GraphGraphicsView::drawForeground(QPainter* painter, const QRectF& rect)
    {
        Q_UNUSED(rect)

#ifdef GUI_DEBUG_GRID
        if (m_debug_gridpos_enable)
            debug_draw_layouter_gridpos(painter);
#endif

        if (!m_minimap_enabled)
            return;

        QRectF map(viewport()->width() - 210, viewport()->height() - 130, 200, 120);
        painter->resetTransform();
        painter->fillRect(map, QColor(0, 0, 0, 170));
    }

#ifdef GUI_DEBUG_GRID
    void GraphGraphicsView::debug_draw_layouter_gridpos(QPainter* painter)
    {
        painter->resetTransform();
        painter->setPen(QPen(Qt::magenta));
        QString pos_str = QString("(%1, %2)").arg(m_debug_gridpos.x()).arg(m_debug_gridpos.y());
        painter->drawText(QPoint(25, 25), pos_str);
    }
#endif

    void GraphGraphicsView::mousePressEvent(QMouseEvent* event)
    {
        if (event->modifiers() == m_move_modifier)
        {
            if (event->button() == Qt::LeftButton)
                m_move_position = event->pos();
        }
        else if (event->button() == Qt::LeftButton)
        {
            GraphicsItem* item = static_cast<GraphicsItem*>(itemAt(event->pos()));
            if (item && item_draggable(item))
            {
                m_drag_item               = static_cast<GraphicsGate*>(item);
                m_drag_mousedown_position = event->pos();
                m_drag_start_gridpos      = closest_layouter_pos(mapToScene(m_drag_mousedown_position))[0];
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

    void GraphGraphicsView::mouseMoveEvent(QMouseEvent* event)
    {
        if (!scene())
            return;

        QPointF delta = m_target_viewport_pos - event->pos();

        if (qAbs(delta.x()) > 5 || qAbs(delta.y()) > 5)
        {
            m_target_viewport_pos = event->pos();
            m_target_scene_pos    = mapToScene(event->pos());
        }

        if (event->buttons().testFlag(Qt::LeftButton))
        {
            if (event->modifiers() == m_move_modifier)
            {
                QScrollBar* hBar  = horizontalScrollBar();
                QScrollBar* vBar  = verticalScrollBar();
                QPoint delta_move = event->pos() - m_move_position;
                m_move_position   = event->pos();
                hBar->setValue(hBar->value() + (isRightToLeft() ? delta_move.x() : -delta_move.x()));
                vBar->setValue(vBar->value() - delta_move.y());
            }
            else
            {
                if (m_drag_item && (event->pos() - m_drag_mousedown_position).manhattanLength() >= QApplication::startDragDistance())
                {
                    QDrag* drag         = new QDrag(this);
                    QMimeData* mimeData = new QMimeData;

                    // TODO set MIME type and icon
                    mimeData->setText("dragTest");
                    drag->setMimeData(mimeData);
                    // drag->setPixmap(iconPixmap);

                    // enable DragMoveEvents until mouse released
                    drag->exec(Qt::MoveAction);
                }
            }
        }
#ifdef GUI_DEBUG_GRID
        debug_show_layouter_gridpos(event->pos());
#endif

        QGraphicsView::mouseMoveEvent(event);
    }

    void GraphGraphicsView::dragEnterEvent(QDragEnterEvent* event)
    {
        if (event->source() == this && event->proposedAction() == Qt::MoveAction)
        {
            event->acceptProposedAction();
            QSizeF size(m_drag_item->width(), m_drag_item->height());
            QPointF mouse = event->posF();
            QPointF snap  = closest_layouter_pos(mapToScene(mouse.x(), mouse.y()))[1];
            if (g_selection_relay->m_selected_gates.size() > 1)
            {
                // if we are in multi-select mode, reduce the selection to the
                // item we are dragging
                g_selection_relay->clear();
                g_selection_relay->m_selected_gates.insert(m_drag_item->id());
                g_selection_relay->m_focus_type = SelectionRelay::item_type::gate;
                g_selection_relay->m_focus_id   = m_drag_item->id();
                g_selection_relay->m_subfocus   = SelectionRelay::subfocus::none;
                g_selection_relay->relay_selection_changed(nullptr);
            }
            m_drop_allowed = false;
            static_cast<GraphicsScene*>(scene())->start_drag_shadow(snap, size, NodeDragShadow::drag_cue::rejected);
        }
        else
        {
            //causes a bug where dockbar-buttons just disappear instead of snapping back when dragged into the view
//            QGraphicsView::dragEnterEvent(event);
        }
    }

    void GraphGraphicsView::dragLeaveEvent(QDragLeaveEvent* event)
    {
        Q_UNUSED(event)
        static_cast<GraphicsScene*>(scene())->stop_drag_shadow();
    }

    void GraphGraphicsView::dragMoveEvent(QDragMoveEvent* event)
    {
        if (event->source() == this && event->proposedAction() == Qt::MoveAction)
        {
            bool swapModifier    = event->keyboardModifiers() == m_drag_modifier;
            QVector<QPoint> snap = closest_layouter_pos(mapToScene(event->pos()));

            if (snap[0] == m_drag_current_gridpos && swapModifier == m_drag_current_modifier)
            {
                return;
            }
            m_drag_current_gridpos  = snap[0];
            m_drag_current_modifier = swapModifier;

            auto context            = m_graph_widget->get_context();
            GraphLayouter* layouter = context->debug_get_layouter();
            assert(layouter->done());    // ensure grid stable
            QMap<QPoint, Node>::const_iterator node_iter = layouter->position_to_node_map().find(snap[0]);

            NodeDragShadow::drag_cue cue = NodeDragShadow::drag_cue::rejected;
            // disallow dropping an item on itself
            if (snap[0] != m_drag_start_gridpos)
            {
                if (swapModifier)
                {
                    if (node_iter != layouter->position_to_node_map().end())
                    {
                        // allow move only on empty cells
                        cue = NodeDragShadow::drag_cue::swappable;
                    }
                }
                else
                {
                    if (node_iter == layouter->position_to_node_map().end())
                    {
                        // allow move only on empty cells
                        cue = NodeDragShadow::drag_cue::movable;
                    }
                }
            }
            m_drop_allowed = (cue != NodeDragShadow::drag_cue::rejected);

            static_cast<GraphicsScene*>(scene())->move_drag_shadow(snap[1], cue);
        }
    }

    void GraphGraphicsView::dropEvent(QDropEvent* event)
    {
        if (event->source() == this && event->proposedAction() == Qt::MoveAction)
        {
            event->acceptProposedAction();
            GraphicsScene* s = static_cast<GraphicsScene*>(scene());
            s->stop_drag_shadow();
            if (m_drop_allowed)
            {
                auto context            = m_graph_widget->get_context();
                GraphLayouter* layouter = context->debug_get_layouter();
                assert(layouter->done());    // ensure grid stable

                // convert scene coordinates into layouter grid coordinates
                QPointF targetPos        = s->drop_target();
                QPoint targetLayouterPos = closest_layouter_pos(targetPos)[0];
                QPoint sourceLayouterPos = layouter->gridPointByItem(m_drag_item);

                if (targetLayouterPos == sourceLayouterPos)
                {
                    qDebug() << "Attempted to drop gate onto itself, this should never happen!";
                    return;
                }
                // assert(targetLayouterPos != sourceLayouterPos);

                bool modifierPressed = event->keyboardModifiers() == m_drag_modifier;
                if (modifierPressed)
                {
                    // swap mode; swap gates

                    Node nodeFrom = layouter->position_to_node_map().value(sourceLayouterPos);
                    Node nodeTo   = layouter->position_to_node_map().value(targetLayouterPos);
                    assert(!nodeFrom.isNull());    // assert that value was found
                    assert(!nodeTo.isNull());
                    layouter->swap_node_positions(nodeFrom, nodeTo);
                }
                else
                {
                    // move mode; move gate to the selected location
                    QMap<QPoint,Node> nodeMap = layouter->position_to_node_map();
                    auto nodeToMoveIt = nodeMap.find(sourceLayouterPos);
                    Q_ASSERT(nodeToMoveIt != nodeMap.end());
                    layouter->set_node_position(nodeToMoveIt.value(), targetLayouterPos);
                }
                // re-layout the nets
                context->schedule_scene_update();
            }
        }
        else
        {
            QGraphicsView::dropEvent(event);
        }
    }

    void GraphGraphicsView::wheelEvent(QWheelEvent* event)
    {
        if (QApplication::keyboardModifiers() == m_zoom_modifier)
        {
            if (event->orientation() == Qt::Vertical)
            {
                qreal angle  = event->angleDelta().y();
                qreal factor = qPow(m_zoom_factor_base, angle);
                gentle_zoom(factor);
            }
        }
    }

    void GraphGraphicsView::keyPressEvent(QKeyEvent* event)
    {
        switch (event->key())
        {
            case Qt::Key_Space: {
                //qDebug() << "Space pressed";
            }
            break;
        }

        event->ignore();
    }

    void GraphGraphicsView::keyReleaseEvent(QKeyEvent* event)
    {
        switch (event->key())
        {
            case Qt::Key_Space: {
                //qDebug() << "Space released";
            }
            break;
        }

        event->ignore();
    }

    void GraphGraphicsView::resizeEvent(QResizeEvent* event)
    {
        QGraphicsView::resizeEvent(event);
        adjust_min_scale();
    }

    void GraphGraphicsView::show_context_menu(const QPoint& pos)
    {
        GraphicsScene* s = static_cast<GraphicsScene*>(scene());

        if (!s)
            return;

        QMenu context_menu(this);
        QAction* action;

        QGraphicsItem* item = itemAt(pos);
        bool isGate         = false;
        bool isModule       = false;
        bool isMultiSelect  = false;
        bool isNet          = false;

        if (item)
        {
            m_item   = static_cast<GraphicsItem*>(item);
            isGate   = m_item->item_type() == item_type::Gate;
            isModule = m_item->item_type() == item_type::Module;
            isNet    = m_item->item_type() == item_type::Net;

            if (isGate)
            {
                if (g_selection_relay->m_selected_gates.find(m_item->id()) == g_selection_relay->m_selected_gates.end())
                {
                    g_selection_relay->clear();
                    g_selection_relay->m_selected_gates.insert(m_item->id());
                    g_selection_relay->m_focus_type = SelectionRelay::item_type::gate;
                    g_selection_relay->m_focus_id   = m_item->id();
                    g_selection_relay->m_subfocus   = SelectionRelay::subfocus::none;
                    g_selection_relay->relay_selection_changed(this);
                }

                context_menu.addAction("This gate:")->setEnabled(false);

                action = context_menu.addAction("  Change gate name");
                QObject::connect(action, &QAction::triggered, this, &GraphGraphicsView::handle_rename_action);

                action = context_menu.addAction("  Fold parent module");
                QObject::connect(action, &QAction::triggered, this, &GraphGraphicsView::handle_fold_single_action);
            }
            else if (isModule)
            {
                if (g_selection_relay->m_selected_modules.find(m_item->id()) == g_selection_relay->m_selected_modules.end())
                {
                    g_selection_relay->clear();
                    g_selection_relay->m_selected_modules.insert(m_item->id());
                    g_selection_relay->m_focus_type = SelectionRelay::item_type::module;
                    g_selection_relay->m_focus_id   = m_item->id();
                    g_selection_relay->m_subfocus   = SelectionRelay::subfocus::none;
                    g_selection_relay->relay_selection_changed(this);
                }

                context_menu.addAction("This module:")->setEnabled(false);

                action = context_menu.addAction("  Change module name");
                QObject::connect(action, &QAction::triggered, this, &GraphGraphicsView::handle_rename_action);

                action = context_menu.addAction("  Change module type");
                QObject::connect(action, &QAction::triggered, this, &GraphGraphicsView::handle_change_type_action);

                action = context_menu.addAction("  Unfold module");
                QObject::connect(action, &QAction::triggered, this, &GraphGraphicsView::handle_unfold_single_action);
            }
            else if (isNet)
            {
                if (g_selection_relay->m_selected_nets.find(m_item->id()) == g_selection_relay->m_selected_nets.end())
                {
                    g_selection_relay->clear();
                    g_selection_relay->m_selected_nets.insert(m_item->id());
                    g_selection_relay->m_focus_type = SelectionRelay::item_type::net;
                    g_selection_relay->m_focus_id   = m_item->id();
                    g_selection_relay->m_subfocus   = SelectionRelay::subfocus::none;
                    g_selection_relay->relay_selection_changed(this);
                }

                context_menu.addAction("This net:")->setEnabled(false);

                action = context_menu.addAction("  Change net name");
                QObject::connect(action, &QAction::triggered, this, &GraphGraphicsView::handle_rename_action);
            }

            if (g_selection_relay->m_selected_gates.size() + g_selection_relay->m_selected_modules.size() > 1)
            {
                context_menu.addSeparator();
                context_menu.addAction("Entire selection:")->setEnabled(false);
            }

            if (g_selection_relay->m_selected_gates.size() + g_selection_relay->m_selected_modules.size() + g_selection_relay->m_selected_nets.size() > 1)
                isMultiSelect = true;

            if (isGate || isModule)
            {
                action = context_menu.addAction("  Isolate In New View");
                QObject::connect(action, &QAction::triggered, this, &GraphGraphicsView::handle_isolation_view_action);

                action = context_menu.addAction("  Add successors to view");
                connect(action, &QAction::triggered, this, &GraphGraphicsView::handle_select_outputs);

                action = context_menu.addAction("  Add predecessors to view");
                connect(action, &QAction::triggered, this, &GraphGraphicsView::handle_select_inputs);

                Gate* g   = isGate ? g_netlist->get_gate_by_id(m_item->id()) : nullptr;
                Module* m = isModule ? g_netlist->get_module_by_id(m_item->id()) : nullptr;

                // only allow move actions on anything that is not the top module
                if (!(isModule && m == g_netlist->get_top_module()))
                {
                    QMenu* module_submenu = context_menu.addMenu("  Move to module …");

                    action = module_submenu->addAction("New module …");
                    QObject::connect(action, &QAction::triggered, this, &GraphGraphicsView::handle_move_new_action);
                    module_submenu->addSeparator();

                    QActionGroup* module_actions = new QActionGroup(module_submenu);
                    for (auto& module : g_netlist->get_modules())
                    {
                        // don't allow a gate to be moved into its current module
                        // && don't allow a module to be moved into its current module
                        // && don't allow a module to be moved into itself
                        // (either check automatically passes if g respective m is nullptr, so we
                        // don't have to create two loops)
                        if (!module->contains_gate(g) && !module->contains_module(m) && module != m && !m->contains_module(module))
                        {
                            QString mod_name = QString::fromStdString(module->get_name());
                            const u32 mod_id = module->get_id();
                            action           = module_submenu->addAction(mod_name);
                            module_actions->addAction(action);
                            action->setData(mod_id);
                        }
                    }
                    QObject::connect(module_actions, SIGNAL(triggered(QAction*)), this, SLOT(handle_move_action(QAction*)));
                }

                Grouping* assignedGrouping = nullptr;
                if (isGate)
                {
                    if (g)
                        assignedGrouping = g->get_grouping();
                }
                if (isModule)
                {
                    if (m)
                        assignedGrouping = m->get_grouping();
                }
                QMenu* groupingSubmenu;
                if (isMultiSelect)
                    groupingSubmenu = context_menu.addMenu("  Assign all to grouping …");
                else if (assignedGrouping)
                    groupingSubmenu = context_menu.addMenu("  Change grouping assignment …");
                else
                    groupingSubmenu = context_menu.addMenu("  Assign to grouping …");

                QString assignedGroupingName;
                if (assignedGrouping && !isMultiSelect)
                {
                    action = groupingSubmenu->addAction("Delete current assignment");
                    connect(action, &QAction::triggered, this, &GraphGraphicsView::handleGroupingUnassign);
                    assignedGroupingName = QString::fromStdString(assignedGrouping->get_name());
                }
                action = groupingSubmenu->addAction("New grouping …");
                connect(action, &QAction::triggered, this, &GraphGraphicsView::handleGroupingAssignNew);
                for (Grouping* grouping : g_netlist->get_groupings())
                {
                    QString groupingName = QString::fromStdString(grouping->get_name());
                    if (groupingName == assignedGroupingName && !isMultiSelect)
                        continue;
                    action = groupingSubmenu->addAction(ASSIGN_TO_GROUPING + groupingName);
                    connect(action, &QAction::triggered, this, &GraphGraphicsView::handleGroupingAssingExisting);
                }
            }

            if (g_selection_relay->m_selected_gates.size() + g_selection_relay->m_selected_modules.size() > 1)
            {
                if (!g_selection_relay->m_selected_gates.empty())
                {
                    context_menu.addSeparator();
                    context_menu.addAction("All selected gates:")->setEnabled(false);

                    action = context_menu.addAction("  Fold all parent modules");
                    QObject::connect(action, &QAction::triggered, this, &GraphGraphicsView::handle_fold_all_action);
                }
                if (!g_selection_relay->m_selected_modules.empty())
                {
                    context_menu.addSeparator();
                    context_menu.addAction("All selected modules:")->setEnabled(false);

                    action = context_menu.addAction("  Unfold all");
                    QObject::connect(action, &QAction::triggered, this, &GraphGraphicsView::handle_unfold_all_action);
                }
            }
        }

        // if (!item || isNet)
        // {
        // QAction* antialiasing_action = context_menu.addAction("Antialiasing");
        // QAction* cosmetic_action     = context_menu.addAction("Cosmetic Nets");
        // QMenu* grid_menu = context_menu.addMenu("Grid");
        // QMenu* type_menu = grid_menu->addMenu("Type");
        // QMenu* cluster_menu          = grid_menu->addMenu("Clustering");
        // QAction* lines_action        = type_menu->addAction("Lines");
        // QAction* dots_action         = type_menu->addAction("Dots");
        // QAction* none_action         = type_menu->addAction("None");
        // connect(action, &QAction::triggered, this, SLOT);
        // }

        context_menu.exec(mapToGlobal(pos));
        update();
    }

    void GraphGraphicsView::update_matrix(const int delta)
    {
        qreal scale = qPow(2.0, delta / 100.0);

        QMatrix matrix;
        matrix.scale(scale, scale);
        setMatrix(matrix);
    }

    void GraphGraphicsView::toggle_antialiasing()
    {
        setRenderHint(QPainter::Antialiasing, !(renderHints() & QPainter::Antialiasing));
    }

    bool GraphGraphicsView::item_draggable(GraphicsItem* item)
    {
        item_type type = item->item_type();
        return type == item_type::Gate || type == item_type::Module;
    }

    void GraphGraphicsView::gentle_zoom(const qreal factor)
    {
        scale(factor, factor);
        centerOn(m_target_scene_pos);
        QPointF delta_viewport_pos = m_target_viewport_pos - QPointF(viewport()->width() / 2.0, viewport()->height() / 2.0);
        QPointF viewport_center    = mapFromScene(m_target_scene_pos) - delta_viewport_pos;
        centerOn(mapToScene(viewport_center.toPoint()));
    }

    void GraphGraphicsView::viewport_center_zoom(const qreal factor)
    {
        QPointF target_pos = mapToScene(viewport()->rect().center());
        scale(factor, factor);
        centerOn(target_pos.toPoint());
    }

    void GraphGraphicsView::handle_select_outputs()
    {
        auto context           = m_graph_widget->get_context();
        QAction* sender_action = dynamic_cast<QAction*>(sender());
        if (sender_action)
        {
            QSet<u32> gates;
            for (auto sel_id : g_selection_relay->m_selected_gates)
            {
                auto gate = g_netlist->get_gate_by_id(sel_id);
                for (auto net : gate->get_fan_out_nets())
                {
                    for (const auto& suc : net->get_destinations())
                    {
                        bool found = false;
                        for (const auto& id : context->modules())
                        {
                            auto m = g_netlist->get_module_by_id(id);
                            if (m->contains_gate(suc->get_gate(), true))
                            {
                                found = true;
                                break;
                            }
                        }
                        if (!found)
                        {
                            gates.insert(suc->get_gate()->get_id());
                        }
                    }
                }
            }
            for (auto sel_id : g_selection_relay->m_selected_modules)
            {
                auto module = g_netlist->get_module_by_id(sel_id);
                for (auto net : module->get_output_nets())
                {
                    for (const auto& suc : net->get_destinations())
                    {
                        bool found = false;
                        for (const auto& id : context->modules())
                        {
                            auto m = g_netlist->get_module_by_id(id);
                            if (m->contains_gate(suc->get_gate(), true))
                            {
                                found = true;
                                break;
                            }
                        }
                        if (!found)
                        {
                            gates.insert(suc->get_gate()->get_id());
                        }
                    }
                }
            }

            context->add({}, gates);
        }
    }
    void GraphGraphicsView::handle_select_inputs()
    {
        auto context           = m_graph_widget->get_context();
        QAction* sender_action = dynamic_cast<QAction*>(sender());
        if (sender_action)
        {
            QSet<u32> gates;
            for (auto sel_id : g_selection_relay->m_selected_gates)
            {
                auto gate = g_netlist->get_gate_by_id(sel_id);
                for (auto net : gate->get_fan_in_nets())
                {
                    if (!net->get_sources().empty() && net->get_sources().at(0)->get_gate() != nullptr)
                    {
                        bool found = false;
                        for (const auto& id : context->modules())
                        {
                            auto m = g_netlist->get_module_by_id(id);
                            if (m->contains_gate(net->get_sources().at(0)->get_gate(), true))
                            {
                                found = true;
                                break;
                            }
                        }
                        if (!found)
                        {
                            gates.insert(net->get_sources().at(0)->get_gate()->get_id());
                        }
                    }
                }
            }
            for (auto sel_id : g_selection_relay->m_selected_modules)
            {
                auto module = g_netlist->get_module_by_id(sel_id);
                for (auto net : module->get_input_nets())
                {
                    if (!net->get_sources().empty() && net->get_sources().at(0)->get_gate() != nullptr)
                    {
                        bool found = false;
                        for (const auto& id : context->modules())
                        {
                            auto m = g_netlist->get_module_by_id(id);
                            if (m->contains_gate(net->get_sources().at(0)->get_gate(), true))
                            {
                                found = true;
                                break;
                            }
                        }
                        if (!found)
                        {
                            gates.insert(net->get_sources().at(0)->get_gate()->get_id());
                        }
                    }
                }
            }
            context->add({}, gates);
        }
    }

    void GraphGraphicsView::handle_global_setting_changed(void* sender, const QString& key, const QVariant& value)
    {
        UNUSED(sender);
        if (key == "graph_view/drag_mode_modifier")
        {
            unsigned int modifier = value.toUInt();
            m_drag_modifier       = Qt::KeyboardModifier(modifier);
        }
        else if (key == "graph_view/move_modifier")
        {
            unsigned int modifier = value.toUInt();
            m_move_modifier       = Qt::KeyboardModifier(modifier);
        }
#ifdef GUI_DEBUG_GRID
        else if (key == "debug/grid")
        {
            m_debug_gridpos_enable = value.toBool();
        }
#endif
    }

    void GraphGraphicsView::handle_fold_single_action()
    {
        auto context = m_graph_widget->get_context();
        context->fold_module_of_gate(m_item->id());
    }

    void GraphGraphicsView::handle_unfold_single_action()
    {
        auto context = m_graph_widget->get_context();
        auto m       = g_netlist->get_module_by_id(m_item->id());
        if (m->get_gates().empty() && m->get_submodules().empty())
        {
            QMessageBox msg;
            msg.setText("This module is empty.\nYou can't unfold it.");
            msg.setWindowTitle("Error");
            msg.exec();
            return;
            // We would otherwise unfold the empty module into nothing, so the user
            // would have nowhere to click to get their module back
        }
        context->unfold_module(m_item->id());
    }

    void GraphGraphicsView::handle_fold_all_action()
    {
        auto context = m_graph_widget->get_context();

        context->begin_change();
        for (u32 id : g_selection_relay->m_selected_gates)
        {
            context->fold_module_of_gate(id);
        }
        context->end_change();
    }

    void GraphGraphicsView::handle_unfold_all_action()
    {
        auto context = m_graph_widget->get_context();

        context->begin_change();
        for (u32 id : g_selection_relay->m_selected_modules)
        {
            context->unfold_module(id);
        }
        context->end_change();
    }

    void GraphGraphicsView::handleGroupingUnassign()
    {
        Grouping* assignedGrouping = nullptr;
        if (m_item->item_type() == item_type::Gate)
        {
            Gate* g = g_netlist->get_gate_by_id(m_item->id());
            if (g)
                assignedGrouping = g->get_grouping();
            if (!assignedGrouping)
                return;
            assignedGrouping->remove_gate(g);
        }
        if (m_item->item_type() == item_type::Module)
        {
            Module* m = g_netlist->get_module_by_id(m_item->id());
            if (m)
                assignedGrouping = m->get_grouping();
            if (!assignedGrouping)
                return;
            assignedGrouping->remove_module(m);
        }
    }

    void GraphGraphicsView::groupingAssignInternal(Grouping* grp)
    {
        if (g_selection_relay->m_selected_gates.size() + g_selection_relay->m_selected_modules.size() + g_selection_relay->m_selected_nets.size() > 1)
        {
            g_content_manager->getSelectionDetailsWidget()->selectionToGroupingInternal(grp);
            return;
        }

        if (m_item->item_type() == item_type::Gate)
        {
            Gate* g = g_netlist->get_gate_by_id(m_item->id());
            if (g)
                grp->assign_gate(g);
        }
        if (m_item->item_type() == item_type::Module)
        {
            Module* m = g_netlist->get_module_by_id(m_item->id());
            if (m)
                grp->assign_module(m);
        }
        g_selection_relay->clear();
        g_selection_relay->relay_selection_changed(nullptr);
    }

    void GraphGraphicsView::handleGroupingAssignNew()
    {
        handleGroupingUnassign();
        Grouping* grp = g_content_manager->getGroupingManagerWidget()->getModel()->addDefaultEntry();
        if (grp)
            groupingAssignInternal(grp);
    }

    void GraphGraphicsView::handleGroupingAssingExisting()
    {
        handleGroupingUnassign();
        const QAction* action = static_cast<const QAction*>(QObject::sender());
        QString grpName       = action->text();
        if (grpName.startsWith(ASSIGN_TO_GROUPING))
            grpName.remove(0, ASSIGN_TO_GROUPING.size());
        Grouping* grp = g_content_manager->getGroupingManagerWidget()->getModel()->groupingByName(grpName);
        if (grp)
            groupingAssignInternal(grp);
    }

#ifdef GUI_DEBUG_GRID
    void GraphGraphicsView::debug_show_layouter_gridpos(const QPoint& mouse_pos)
    {
        auto context = m_graph_widget->get_context();
        if (!context)
            return;

        GraphLayouter* layouter = context->debug_get_layouter();
        if (!(layouter->done()))
            return;

        QPointF scene_mouse_pos = mapToScene(mouse_pos);
        QPoint layouter_pos     = closest_layouter_pos(scene_mouse_pos)[0];
        m_debug_gridpos         = layouter_pos;
    }
#endif

    QVector<QPoint> GraphGraphicsView::closest_layouter_pos(const QPointF& scene_pos) const
    {
        auto context = m_graph_widget->get_context();
        assert(context);

        GraphLayouter* layouter = context->debug_get_layouter();
        assert(layouter->done());    // ensure grid stable

        int default_width      = layouter->default_grid_width();
        int default_height     = layouter->default_grid_height();
        int min_x              = layouter->min_x_index();
        int min_y              = layouter->min_y_index();
        QVector<qreal> x_vals  = layouter->x_values();
        QVector<qreal> y_vals  = layouter->y_values();
        layouter_point x_point = closest_layouter_point(scene_pos.x(), default_width, min_x, x_vals);
        layouter_point y_point = closest_layouter_point(scene_pos.y(), default_height, min_y, y_vals);
        return QVector({QPoint(x_point.index, y_point.index), QPoint(x_point.pos, y_point.pos)});
    }

    GraphGraphicsView::layouter_point GraphGraphicsView::closest_layouter_point(qreal scene_pos, int default_spacing, int min_index, QVector<qreal> sections) const
    {
        int index = min_index;
        qreal pos = 0;
        if (sections.first() > scene_pos)
        {
            int distance   = sections.first() - scene_pos;
            int nSections  = distance / default_spacing;    // this rounds down
            qreal posThis  = sections.first() - nSections * default_spacing;
            qreal distThis = qAbs(scene_pos - posThis);
            qreal posPrev  = posThis - default_spacing;
            qreal distPrev = qAbs(scene_pos - posPrev);
            if (distPrev < distThis)
            {
                index -= nSections + 1;
                pos = posPrev;
            }
            else
            {
                index -= nSections;
                pos = posThis;
            }
        }
        else if (sections.last() < scene_pos)
        {
            int distance   = scene_pos - sections.last();
            int nSections  = distance / default_spacing;    // this rounds down
            qreal posThis  = sections.last() + nSections * default_spacing;
            qreal distThis = qAbs(scene_pos - posThis);
            qreal posNext  = posThis + default_spacing;
            qreal distNext = qAbs(scene_pos - posNext);
            if (distNext < distThis)
            {
                index += nSections + sections.size();
                pos = posNext;
            }
            else
            {
                index += nSections + sections.size() - 1;
                pos = posThis;
            }
        }
        else
        {
            // binary search for first value in sections larger than or equal to scene_pos
            const qreal* needle = std::lower_bound(sections.constBegin(), sections.constEnd(), scene_pos);
            int i               = needle - sections.begin();    // position of needle in the vector
            index += i;
            // check if we're closer to this or the next position
            qreal posThis  = *needle;
            qreal distThis = qAbs(scene_pos - posThis);
            qreal posPrev  = (i > 0) ? sections[i - 1] : (sections.first() - default_spacing);
            qreal distPrev = qAbs(scene_pos - posPrev);
            if (distPrev < distThis)
            {
                index--;
                pos = posPrev;
            }
            else
            {
                pos = posThis;
            }
        }
        return layouter_point{index, pos};
    }
}    // namespace hal
