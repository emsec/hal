#include "graph_widget/graph_graphics_view.h"

#include "gui/graph_widget/graph_widget.h"
#include "gui/graph_widget/graph_widget_constants.h"
#include "gui/graph_widget/graphics_scene.h"
#include "gui/graph_widget/graphics_view_zoom.h"
#include "gui/graph_widget/items/io_graphics_net.h"
#include "gui/graph_widget/items/graphics_gate.h"
#include "gui/graph_widget/items/graphics_item.h"
#include "gui/graph_widget/items/separated_graphics_net.h"
#include "gui/graph_widget/items/standard_graphics_gate.h"
#include "gui/graph_widget/items/standard_graphics_module.h"
#include "gui/graph_widget/items/standard_graphics_net.h"
#include "gui/gui_globals.h"
#include "netlist/gate.h"
#include "netlist/module.h"

#include <QAction>
#include <QColorDialog>
#include <qmath.h>
#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <QScrollBar>
#include <QStyleOptionGraphicsItem>
#include <QWheelEvent>

graph_graphics_view::graph_graphics_view(QWidget* parent) : QGraphicsView(parent),
    m_minimap_enabled(false),
    m_antialiasing_enabled(false),
    m_cosmetic_nets_enabled(false),
    m_grid_enabled(true),
    m_grid_clusters_enabled(true),
    m_grid_type(graph_widget_constants::grid_type::lines)
{
    connect(&g_selection_relay, &selection_relay::subfocus_changed, this, &graph_graphics_view::conditional_update);
    connect(this, &graph_graphics_view::customContextMenuRequested, this, &graph_graphics_view::show_context_menu);

    setContextMenuPolicy(Qt::CustomContextMenu);
    setOptimizationFlags(QGraphicsView::DontSavePainterState);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    
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

void graph_graphics_view::handle_cone_view_action()
{
    std::shared_ptr<gate> g = g_netlist->get_gate_by_id(m_item->id());

    if (!g)
        return;
}

//////////
// If we have time:
// This class should NOT directly perform any of these actions.
// There should be a global manager for this.

void graph_graphics_view::handle_move_action(QAction *action)
{
    const u32 mod_id = action->data().toInt();
    const u32 gate_id = m_item->id();
    std::shared_ptr<module> m = g_netlist->get_module_by_id(mod_id);
    std::shared_ptr<gate> g = g_netlist->get_gate_by_id(gate_id);
    if (g)
        m->assign_gate(g);
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

    const u32 gate_id = m_item->id();
    std::shared_ptr<gate> g = g_netlist->get_gate_by_id(gate_id);

    if (g)
        m->assign_gate(g);
}

void graph_graphics_view::handle_rename_action()
{
    // move this to separate class in the future, if this gets any bigger

    // get the selected gate and its current name
    std::shared_ptr<gate> g = g_netlist->get_gate_by_id(m_item->id());
    const QString name = QString::fromStdString(g->get_name());
    // show single-line input dialog
    bool confirm;
    const QString new_name = QInputDialog::getText(this, "Rename gate", "New name:", QLineEdit::Normal, name, &confirm);
    // if dialog has been confirmed, apply the new name
    if (confirm)
        g->set_name(new_name.toStdString());
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

    if(item->item_type() == hal::item_type::module)
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
    else
        if (event->button() == Qt::MidButton)
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

    if (event->modifiers() == Qt::ShiftModifier)
    {
        if (event->buttons().testFlag(Qt::LeftButton))
        {
            QScrollBar *hBar = horizontalScrollBar();
            QScrollBar *vBar = verticalScrollBar();
            QPoint delta = event->pos() - m_move_position;
            m_move_position = event->pos();
            hBar->setValue(hBar->value() + (isRightToLeft() ? delta.x() : -delta.x()));
            vBar->setValue(vBar->value() - delta.y());
        }
    }
        QGraphicsView::mouseMoveEvent(event);
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

    QGraphicsItem* item = itemAt(pos);
    if (item)
    {
        m_item = static_cast<graphics_item*>(item);

        switch (m_item->item_type())
        {
            case hal::item_type::gate:
            {
                // DEBUG ONLY
                //QAction* color_action = context_menu.addAction("Change Color");
                //QObject::connect(color_action, &QAction::triggered, this, &graph_graphics_view::handle_change_color_action);
                //context_menu.addAction(color_action);

                QAction* rename_action = context_menu.addAction("Rename …");
                QObject::connect(rename_action, &QAction::triggered, this, &graph_graphics_view::handle_rename_action);
                context_menu.addAction(rename_action);

                QMenu* module_submenu = context_menu.addMenu("Move to module");
                QActionGroup* module_actions = new QActionGroup(module_submenu);
                for (auto& module : g_netlist->get_modules())
                {
                    std::shared_ptr<gate> g = g_netlist->get_gate_by_id(m_item->id());
                    if (!module->contains_gate(g))
                    {
                        QString mod_name = QString::fromStdString(module->get_name());
                        const u32 mod_id = module->get_id();
                        QAction* action = module_submenu->addAction(mod_name);
                        module_actions->addAction(action);
                        action->setData(mod_id);
                    }
                }
                QObject::connect(module_actions, SIGNAL(triggered(QAction*)), this, SLOT(handle_move_action(QAction*)));
                module_submenu->addSeparator();

                QAction* new_mod_action = module_submenu->addAction("New module …");
                QObject::connect(new_mod_action, &QAction::triggered, this, &graph_graphics_view::handle_move_new_action);
                module_submenu->addAction(new_mod_action);

                QAction* cone_view_action = context_menu.addAction("Open in Cone View");
                QObject::connect(cone_view_action, &QAction::triggered, this, &graph_graphics_view::handle_cone_view_action);
                context_menu.addAction(cone_view_action);
                break;
            }
            default:
                break;
        }
    }
    else
    {
        QAction* antialiasing_action = context_menu.addAction("Antialiasing");
        QAction* cosmetic_action = context_menu.addAction("Cosmetic Nets");
        QMenu* grid_menu = context_menu.addMenu("Grid");
        QMenu* type_menu = grid_menu->addMenu("Type");
        QMenu* cluster_menu = grid_menu->addMenu("Clustering");
        QAction* lines_action = type_menu->addAction("Lines");
        QAction* dots_action = type_menu->addAction("Dots");
        QAction* none_action = type_menu->addAction("None");
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
