#include "graph_layouter/graph_layouter_view.h"
#include "graph_layouter/gui_graph_gate_selection_widget.h"
#include "graph_layouter/gui_graph_net.h"

#include "core/log.h"
#include "gui_globals.h"

#include <QGraphicsScene>
#include <QKeySequence>
#include <QList>
#include <QShortcut>
#include <cmath>

#include <QDebug>
#include <QMessageBox>

#include <QApplication>
#include <QDesktopWidget>

graph_layouter_view::graph_layouter_view(QGraphicsScene* scene, old_graph_layouter* layouter, std::shared_ptr<netlist> g) : QGraphicsView(scene)
{
    this->viewport()->installEventFilter(this);
    this->setMouseTracking(true);
    _modifiers        = Qt::NoModifier;
    _zoom_factor_base = 1.0020;
    //this->setStyleSheet("background: transparent");
    m_layouter = layouter;
    m_netlist  = g;


    QObject::connect(this->scene(), SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged()));
    //Shortcut
    QShortcut* shortcut = new QShortcut(QKeySequence(tr("Ctrl+a")), this);
    QObject::connect(shortcut, &QShortcut::activated, this, &graph_layouter_view::onShortcutActivated);
    QShortcut* zoom_in_shortcut  = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Plus), this);
    QShortcut* zoom_out_shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Minus), this);
    connect(zoom_in_shortcut, &QShortcut::activated, this, &graph_layouter_view::zoom_in);
    connect(zoom_out_shortcut, &QShortcut::activated, this, &graph_layouter_view::zoom_out);

    //the selection relay things
    QObject::connect(&g_selection_relay, &selection_relay::combined_selection_update, this, &graph_layouter_view::handle_combined_selection_update);

    g_selection_relay.register_sender(this, "graph");

    //test
    //setOptimizationFlags(QGraphicsView::DontSavePainterState);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    setDragMode(QGraphicsView::ScrollHandDrag);
}

graph_layouter_view::~graph_layouter_view()
{
    delete m_layouter;
    delete this->scene();    //temporary, do never ever do this, oh boi..
}

void graph_layouter_view::gentle_zoom(
    double factor)    //factor increases, decreases with speed...so level are useless so to speak, so the pen_width has to be calculated with the current zoom_factor(or normalize the factor
{
    //the conditions to jump straight out
    if (zoom_activated)
        return;

    //New Zooming Tests
    if (current_view_scene_ratio >= original_view_scene_ratio)
        if (factor > 1)
            return;

    if (current_view_scene_ratio <= entire_graph_view_scene_ratio)
        if (factor < 1)
            return;

    int zoom_level_before = zoom_level;

    //Normalize the factor so that the level can be used
    if (factor < 1)
    {
        factor = zoom_out_factor;    //normalize
        if (zoom_level >= 0)
        {
            pen_width += 1;
            set_graphicsitems_pen_width(pen_width);
        }
        zoom_level++;
    }
    if (factor > 1)
    {
        factor = zoom_in_factor;    //normalize
        if (zoom_level > 0)
        {
            pen_width -= 1;
            set_graphicsitems_pen_width(pen_width);
        }
        zoom_level--;
    }

    current_zoom_factor *= factor;
    this->scale(factor, factor);
    this->centerOn(target_scene_pos);
    QPointF delta_viewport_pos = target_viewport_pos - QPointF(this->viewport()->width() / 2.0, this->viewport()->height() / 2.0);
    QPointF viewport_center    = this->mapFromScene(target_scene_pos) - delta_viewport_pos;
    this->centerOn(this->mapToScene(viewport_center.toPoint()));

    //calculate the new ratio for the new zooming method
    ////------->it is divided by the factor (if the factor is 1,2, then it is current width / factor = new witdth, or you multipliy with the other => current width * 0.7(other factor of 1,2) = new width)
    current_view_scene_ratio = this->rect().width() / this->mapToScene(this->rect()).boundingRect().width();

    log_info("user", "Changed from zoomlevel {} to {}.", zoom_level_before, zoom_level);
    Q_EMIT zoomed();
}

void graph_layouter_view::set_modifiers(Qt::KeyboardModifiers modifiers)
{
    _modifiers = modifiers;
}

void graph_layouter_view::set_zoom_factor_base(double value)
{
    _zoom_factor_base = value;
}

bool graph_layouter_view::eventFilter(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::MouseMove)
    {
        QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);
        QPointF delta            = target_viewport_pos - mouse_event->pos();
        if (qAbs(delta.x()) > 5 || qAbs(delta.y()) > 5)
        {
            target_viewport_pos = mouse_event->pos();
            target_scene_pos    = this->mapToScene(mouse_event->pos());
        }
    }
    else if (event->type() == QEvent::Wheel)
    {
        QWheelEvent* wheel_event = static_cast<QWheelEvent*>(event);
        if (QApplication::keyboardModifiers() == _modifiers)
        {
            if (wheel_event->orientation() == Qt::Vertical)
            {
                double angle  = wheel_event->angleDelta().y();
                double factor = (angle > 0) ? qPow(_zoom_factor_base, 120) : qPow(_zoom_factor_base, -120);
                //double factor = qPow(_zoom_factor_base, angle);
                gentle_zoom(factor);
                return true;
            }
        }
    }
    Q_UNUSED(object)
    return false;
}

void graph_layouter_view::gate_selected_key_pressed(QKeyEvent* event, gui_graph_gate* gate)
{
    if (event->key() == Qt::Key_Left)
    {
        if (gate->input_pin_count() == 0)
            return;
        gate->enable_input_pin_navigation(true);
        state = gate_input_pins_selected;
        scene()->update();
    }
    else if (event->key() == Qt::Key_Right)
    {
        if (gate->output_pin_count() == 0)
            return;
        gate->enable_output_pin_navigation(true);
        state = gate_output_pins_selected;
        scene()->update();
    }
}

void graph_layouter_view::gate_left_pins_selected_key_pressed(QKeyEvent* event, gui_graph_gate* gate)
{
    if (event->key() == Qt::Key_Up)
    {
        gate->decrease_current_pin();
        scene()->update();
    }
    else if (event->key() == Qt::Key_Down)
    {
        gate->increase_current_pin();
        scene()->update();
    }
    else if (event->key() == Qt::Key_Left)
    {
        //####oroginal
        //        auto predecessor = gate->get_ref_gate()->get_predecessor(gate->get_current_pin_type());

        //        auto predecessor_gate = predecessor.gate;
        //        if (!predecessor_gate)
        //            return;
        //###end original

        //###hotfix
        auto net = gate->get_ref_gate()->get_fan_in_net(gate->get_current_pin_type());
        if (net == nullptr || net->get_src().gate == nullptr)
            return;

        //###end hotfix

        gui_graph_gate* predecessor_gui_gate = m_layouter->get_gui_gate_from_gate(net->get_src().gate);
        gate->setSelected(false);
        gate->enable_input_pin_navigation(false);
        predecessor_gui_gate->setSelected(true);
        this->centerOn(predecessor_gui_gate);
        state = gate_selected;
        gate_history.push(gate);
        currently_selected_gate = predecessor_gui_gate;
    }
    else if (event->key() == Qt::Key_Right)
    {
        gate->enable_input_pin_navigation(false);
        state = gate_selected;
        scene()->update();
    }
}

void graph_layouter_view::gate_right_pins_selected_key_pressed(QKeyEvent* event, gui_graph_gate* gate)
{
    if (event->key() == Qt::Key_Up)
    {
        gate->decrease_current_pin();
        scene()->update();
    }
    else if (event->key() == Qt::Key_Down)
    {
        gate->increase_current_pin();
        scene()->update();
    }
    else if (event->key() == Qt::Key_Left)
    {
        gate->enable_output_pin_navigation(false);
        state = gate_selected;
        scene()->update();
    }
    else if (event->key() == Qt::Key_Right)
    {
        //#######original#####
        //        auto successors = gate->get_ref_gate()->get_successors(gate->get_current_pin_type());
        //        if (successors.size() == 0)
        //            return;
        //#######end original#####

        //####HOTFIX
        auto net = gate->get_ref_gate()->get_fan_out_net(gate->get_current_pin_type());
        if ((net == nullptr) || (net->get_num_of_dsts() == 0))
            return;

        auto successors = net->get_dsts();
        //#####END HOTFIX

        //deleteonclose attribute is set
        gui_graph_gate_selection_widget* widget =
            new gui_graph_gate_selection_widget(this, successors, gate->get_ref_gate(), gate->get_current_pin_type());
        auto rect = QApplication::desktop()->availableGeometry(this);
        widget->move(QPoint(rect.x() + (rect.width() - widget->width()) / 2, rect.y() + (rect.height() - widget->height()) / 2));
        widget->show();
        widget->setFocus();
    }
}

void graph_layouter_view::net_selected_key_pressed(QKeyEvent* event)
{
    if (selected_nets.size() > 1 || selected_nets.size() == 0)
        return;
    if (event->key() == Qt::Key_Left)
    {
        auto net                     = selected_nets.at(0)->get_ref_net();
        auto dst_tuple               = net->get_src();
        auto src_gate                = dst_tuple.gate;
        gui_graph_gate* src_gui_gate = m_layouter->get_gui_gate_from_gate(src_gate);
        this->centerOn(src_gui_gate);
    }
    //needs to be improved, maybe an own class, not gui_graph_gate_selection_widget
    else if (event->key() == Qt::Key_Right)
    {
        auto gui_net                            = selected_nets.at(0);
        auto net                                = selected_nets.at(0)->get_ref_net();
        auto src_tuple                          = net->get_src();
        auto dsts_set                           = net->get_dsts();
        currently_selected_gate                 = m_layouter->get_gui_gate_from_gate(src_tuple.get_gate());
//        gui_graph_gate_selection_widget* widget = new gui_graph_gate_selection_widget(this, dsts_set, src_tuple.gate, src_tuple.get_pin_type());
//        widget->set_is_net_selected(false);
//        widget->exec();
//        gui_net->setSelected(false);
    }
}

void graph_layouter_view::paintEvent(QPaintEvent* event)
{
    QGraphicsView::paintEvent(event);
    Q_EMIT redrawn();
}

void graph_layouter_view::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Control)
    {
        this->setDragMode(QGraphicsView::RubberBandDrag);
    }
    if (event->key() == Qt::Key_Space)
    {
        //1. sich das oberste item holen, das als "focus" setzen wie bei den anderen auch, dann löschen
        if (!gate_history.isEmpty())
        {
            if (currently_selected_gate)
            {
                currently_selected_gate->enable_input_pin_navigation(false);    //the other mode will automatically disabled
                currently_selected_gate->setSelected(false);
            }

            gui_graph_gate* last_gate = gate_history.pop();
            last_gate->setSelected(true);
            this->centerOn(last_gate);
            currently_selected_gate = last_gate;
        }
    }

    QList<QGraphicsItem*> items = this->scene()->selectedItems();
    if (items.size() > 1 || items.size() == 0)
        return;
    gui_graph_gate* selected_gui_gate = dynamic_cast<gui_graph_gate*>(items.at(0));

    switch (state)
    {
        case gate_selected:
            gate_selected_key_pressed(event, selected_gui_gate);
            break;
        case gate_input_pins_selected:
            gate_left_pins_selected_key_pressed(event, selected_gui_gate);
            break;
        case gate_output_pins_selected:
            gate_right_pins_selected_key_pressed(event, selected_gui_gate);
            break;
        case net_selected:
            net_selected_key_pressed(event);
            break;
        case no_gate_selected:
            break;    // added to suppress warning
    }
}

void graph_layouter_view::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Control)
        this->setDragMode(QGraphicsView::ScrollHandDrag);
}

void graph_layouter_view::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);

    //200 and 100 are the "default" sizes the graphicsview takes when it first starts
    if(!first_time_focus && rect().width() > 200 && rect().height() > 100)
    {
        original_view_scene_ratio = this->rect().width() / this->mapToScene(this->rect()).boundingRect().width();
        startingRect              = this->mapToScene(this->rect()).boundingRect();
        if (this->rect().width() < scene()->sceneRect().width())
            this->fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
        entire_graph_view_scene_ratio = this->rect().width() / this->mapToScene(this->rect()).boundingRect().width();
        current_view_scene_ratio      = entire_graph_view_scene_ratio;
        first_time_focus              = true;

        //Compute the number of zoom levels
        double orig_width            = startingRect.width();
        double entire_graph_width    = this->mapToScene(this->rect()).boundingRect().width();
        overall_number_of_zoom_level = (int)(log(orig_width / entire_graph_width) / log(zoom_out_factor)) + 2;    //only from original to maximal zoomout, doesnt count zoomin
        zoom_level                   = overall_number_of_zoom_level - 1;
        pen_width                    = zoom_level + 2;
        set_graphicsitems_pen_width(pen_width);
        QString log_string = "Starting at zoomlevel: " + QString::number(zoom_level);
        log_info("user", log_string.toStdString());
    }
}

QList<gui_graph_gate*> graph_layouter_view::get_selected_gates()
{
    return selected_gates;
}

QList<gui_graph_net*> graph_layouter_view::get_selected_nets()
{
    return selected_nets;
}

//need to be reorganized
void graph_layouter_view::onSelectionChanged()
{
    //test if it will be called because of the setselected function in the handle methods
    if (just_handle_call)
    {
        just_handle_call = false;
        return;
    }

    QList<QGraphicsItem*> items = this->scene()->selectedItems();
    selected_nets.clear();
    selected_gates.clear();

    //fill the currently selected items in the 2 lists so that the handling of the items becomes easier
    for (int i = 0; i < items.size(); i++)
    {
        gui_graph_gate* gui_gate = dynamic_cast<gui_graph_gate*>(items.at(i));
        if (!gui_gate)
        {
            gui_graph_net* gui_net = dynamic_cast<gui_graph_net*>(items.at(i));
            if (!gui_net)    //it isnt a gate and it isnt a net so ignore that item
                continue;

            selected_nets.append(gui_net);
            continue;
        }
        selected_gates.append(gui_gate);
    }

    // clear selection
    if (state != no_gate_selected)
    {
        state = no_gate_selected;
        if (currently_selected_gate)
        {
            currently_selected_gate->enable_input_pin_navigation(false);
            currently_selected_gate->enable_output_pin_navigation(false);
        }
    }
    currently_selected_gate = nullptr;
    currently_selected_net  = nullptr;

    //because cant handle multiple selection just yet
    if (items.size() == 1)
    {
        // for the relay
        QList<u32> gate_ids;
        QList<u32> net_ids;
        QList<u32> module_ids;

        // check selection
        gui_graph_gate* selected_gui_gate = dynamic_cast<gui_graph_gate*>(items.at(0));
        if (!selected_gui_gate)    //its a net, need to set the currently selected net
        {
            state                  = net_selected;
            currently_selected_net = dynamic_cast<gui_graph_net*>(items.at(0));

            net_ids.append(currently_selected_net->get_ref_net()->get_id());
            g_selection_relay.relay_combined_selection(this, gate_ids, net_ids, module_ids);

            //only trigger the new selection of selected net is still in netlist (temporarily prevents crash, aslong as deleted nets are still selectable in graph / tree / etc..)
            auto ref_net = currently_selected_net->get_ref_net();

            if (g_netlist->is_net_in_netlist(ref_net))
                g_selection_relay.relay_current_net(this, ref_net->get_id());
            else
                g_selection_relay.relay_current_deleted(this);
        }
        else
        {
            state                   = gate_selected;
            currently_selected_gate = selected_gui_gate;

            //for the RELAY, it is only needed here, not in the navigation functions
            gate_ids.append(currently_selected_gate->get_ref_gate()->get_id());
            g_selection_relay.relay_combined_selection(this, gate_ids, net_ids, module_ids);    //this is just for the tree_navigation

            //only trigger the new selection if selected gate is still in netlist (temporarily prevents chrash, aslong as deleted gates are still selectable in graph / tree / etc..)
            auto ref_gate = currently_selected_gate->get_ref_gate();

            if (g_netlist->is_gate_in_netlist(ref_gate))
                g_selection_relay.relay_current_gate(this, ref_gate->get_id() /*currently_selected_gate->get_ref_gate()->get_id()*/);
            else
                g_selection_relay.relay_current_deleted(this);
        }
    }

    scene()->update();
}

void graph_layouter_view::onSuccessorgateSelected(std::shared_ptr<gate> successor)
{
    gui_graph_gate* successor_gui_gate = m_layouter->get_gui_gate_from_gate(successor);
    currently_selected_gate->enable_output_pin_navigation(false);
    gate_history.push(currently_selected_gate);
    currently_selected_gate->setSelected(false);
    successor_gui_gate->setSelected(true);
    this->centerOn(successor_gui_gate);
    state                   = gate_selected;
    currently_selected_gate = successor_gui_gate;
}

void graph_layouter_view::onShortcutActivated()
{
    if (!zoom_activated)
    {
        //testing the drawlinethicker things
        set_graphicsitems_pen_width(5);
        defaultRect =
            this->mapToScene(this->rect()).boundingRect();    //cant be done in the constructor, must be done here(well not exactly, when die view shows something cool, then it needs to be done)
        this->fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);    //if you dont set the flag keepaspectratio, the graph wont be drawn correctly
        zoom_activated = true;
    }
    else
    {
        set_graphicsitems_pen_width(pen_width);    //pen_width must be right(maybe calculated with the level),
        this->fitInView(defaultRect, Qt::KeepAspectRatio);
        zoom_activated = false;
    }
    this->ensurePolished();
}

void graph_layouter_view::zoom_in()
{
    gentle_zoom(qPow(_zoom_factor_base, 120));
}

void graph_layouter_view::zoom_out()
{
    gentle_zoom(qPow(_zoom_factor_base, -120));
}

//IMPORTANT REMINDER, NEED TO FIND A BETTER SOLUTION: just_handle_call must be set to true before every "setSelected" function, because the onSelectionChanged method is always called immediately after
void graph_layouter_view::handle_current_gate_update(void* sender, u32 id)
{
    if (sender == this)    //needed to avoid loop
        return;

    //so that the onselectionchanged method doesnt send weird stuff
    just_handle_call = true;

    gui_graph_gate* gate = m_layouter->get_gui_gate_from_id(id);
    if (!gate)
        return;

    //deactivate the currently selected net, if there is one(because its the way it works in the selection tree)
    if (currently_selected_net)
    {
        currently_selected_net->setSelected(false);
        currently_selected_net = nullptr;
    }
    just_handle_call = true;
    //deactivate all pin related things
    if (currently_selected_gate)
    {
        currently_selected_gate->enable_output_pin_navigation(false);
        currently_selected_gate->enable_input_pin_navigation(false);
        currently_selected_gate->setSelected(false);
        currently_selected_gate = nullptr;
    }

    just_handle_call = true;
    gate->setSelected(true);
    this->centerOn(gate);
    state                   = gate_selected;
    currently_selected_gate = gate;
}

void graph_layouter_view::handle_current_net_update(void* sender, u32 id)
{
    if (sender == this)
        return;

    //so that the onselectionchanged method doesnt send wierd stuff
    just_handle_call = true;

    gui_graph_net* whereever_selected_net = m_layouter->get_gui_graph_net_from_id(id);
    if (!whereever_selected_net)
        return;

    if (currently_selected_net)
        currently_selected_net->setSelected(false);

    just_handle_call = true;
    whereever_selected_net->setSelected(true);
    currently_selected_net = whereever_selected_net;

    just_handle_call = true;
    //deactivate the selected gate now, cause this is the way the tree navigation works
    if (currently_selected_gate)
    {
        currently_selected_gate->setSelected(false);
        currently_selected_gate->enable_input_pin_navigation(false);
        currently_selected_gate->enable_output_pin_navigation(false);
        state                   = no_gate_selected;
        currently_selected_gate = nullptr;
    }
}

//THIS IS THE SIMPLIEST VERSION POSSIBLE, WILL BE UPDATED SHORTLY!!!!(Just the gate or the net, not both
void graph_layouter_view::handle_combined_selection_update(void* sender, const QList<u32>& gate_ids, const QList<u32>& net_ids, const QList<u32>& /* module_ids */, selection_relay::Mode /* mode */)
{
    if (sender == this)
        return;

    if (!gate_ids.isEmpty())    //THE gate BRANCH
    {
        //so that the onselectionchanged method doesnt send wierd stuff
        just_handle_call = true;

        gui_graph_gate* gate = m_layouter->get_gui_gate_from_id(gate_ids.at(0));
        if (!gate)
            return;

        //deactivate the currently selected net, if there is one(because its the way it works in the selection tree)
        if (currently_selected_net)
        {
            currently_selected_net->setSelected(false);
            currently_selected_net = nullptr;
        }
        just_handle_call = true;
        //deactivate all the pin things
        if (currently_selected_gate)
        {
            currently_selected_gate->enable_output_pin_navigation(false);
            currently_selected_gate->enable_input_pin_navigation(false);
            currently_selected_gate->setSelected(false);
            currently_selected_gate = nullptr;
        }

        just_handle_call = true;
        gate->setSelected(true);
        this->centerOn(gate);
        state                   = gate_selected;
        currently_selected_gate = gate;
    }
    else if (!net_ids.isEmpty())    //THE NET BRANCH
    {
        //so that the onselectionchanged method doesnt send wierd stuff
        just_handle_call = true;

        gui_graph_net* whereever_selected_net = m_layouter->get_gui_graph_net_from_id(net_ids.at(0));
        if (!whereever_selected_net)
            return;

        if (currently_selected_net)
            currently_selected_net->setSelected(false);

        just_handle_call = true;
        whereever_selected_net->setSelected(true);
        currently_selected_net = whereever_selected_net;

        just_handle_call = true;
        //deactivate the selected gate now, cause this is the way the tree navigation works
        if (currently_selected_gate)
        {
            currently_selected_gate->setSelected(false);
            currently_selected_gate->enable_input_pin_navigation(false);
            currently_selected_gate->enable_output_pin_navigation(false);
            state                   = no_gate_selected;
            currently_selected_gate = nullptr;
        }
    }
}

void graph_layouter_view::set_graphicsitems_pen_width(int width)    //net_width is 1 lower than the gates for now, so the width must be >=2
{
    QGraphicsScene* gscene      = this->scene();
    QList<QGraphicsItem*> items = gscene->items();
    for (int i = 0; i < items.size(); i++)
    {
        gui_graph_graphics_item* item = dynamic_cast<gui_graph_graphics_item*>(items.at(i));
        if (item)
            item->set_pen_width(width);
    }
}

void graph_layouter_view::set_layouter(old_graph_layouter* layouter)
{
    m_layouter = layouter;
}

void graph_layouter_view::handle_graph_relayouted()
{
    original_view_scene_ratio = 1;
    if (this->rect().width() < scene()->sceneRect().width())
        this->fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
    entire_graph_view_scene_ratio = this->rect().width() / this->mapToScene(this->rect()).boundingRect().width();
    current_view_scene_ratio      = entire_graph_view_scene_ratio;

    //Compute the number of zoom levels
    auto log_with_base = [](double base, double x){return log(x)/log(base);};

    //formula: 1 = current_view_scene_ration * zoom_in_factor^x, where 1 is the original_view_scene ratio (1 is fix, the 2 below is a correction value)
    overall_number_of_zoom_level = log_with_base(zoom_in_factor, 1.0f/current_view_scene_ratio)+2;
    zoom_level                   = overall_number_of_zoom_level - 1;
    pen_width                    = zoom_level + 2;
    set_graphicsitems_pen_width(pen_width);
    QString log_string = "Starting at new zoomlevel: " + QString::number(zoom_level);
    log_info("user", log_string.toStdString());
}
