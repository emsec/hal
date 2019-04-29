#include "graph_layouter/gui_graph_net.h"
#include "graph_layouter/input_dialog.h"
#include "graph_layouter/old_graph_layouter.h"

#include "core/log.h"

#include <QBrush>
#include <QTime>

#include <QDebug>
#include <QFontMetricsF>
#include "graph_layouter/old_graphics_item_qss_adapter.h"

gui_graph_net::gui_graph_net(std::shared_ptr<net> net, QGraphicsItem* parent)
{
    Q_UNUSED(parent)
    refNet = net;
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
}

gui_graph_net::gui_graph_net(std::shared_ptr<net> net, QGraphicsScene* scene, old_graph_layouter* layouter, QGraphicsItem* parent)
{
    Q_UNUSED(parent)
    refNet     = net;
    m_layouter = layouter;
    m_scene    = scene;
    //line_width = m_layouter->get_net_sub_channel_width();

    setFlag(QGraphicsItem::ItemIsSelectable, true);
    default_color = old_graphics_item_qss_adapter::instance()->net_default_color();
    vcc_gnd_font  = QFont("Helvetica", 12);
    vcc_gnd_text  = "";

    auto src_gate = net->get_src().gate;
    if (src_gate == nullptr)
    {
        src_gui_gate = nullptr;
        src_pin      = QPointF(0, 0);
        type         = net_type::global_input;
    }
    else
    {
        src_gui_gate            = m_layouter->get_gui_gate_from_gate(src_gate);
        QPointF offset_pin_cood = src_gui_gate->get_unmapped_output_pin_coordinates(QString::fromStdString(net->get_src().pin_type));
        src_pin                 = src_gui_gate->scenePos() + offset_pin_cood;
    }

    auto dst_set = net->get_dsts();
    if (dst_set.size() < 1)
        type = net_type::global_output;

    for (auto tup : dst_set)
    {
        gui_graph_gate* temp_dst = m_layouter->get_gui_gate_from_gate(tup.gate);
        dst_gui_gates.insert(temp_dst);
        QPointF offset_pin_cood = temp_dst->get_unmapped_input_pin_coordinates(QString::fromStdString(tup.pin_type));
        QPointF temp_dst_pin    = temp_dst->scenePos() + offset_pin_cood;
        temp_dst_pin.rx() -= 5;
        dst_pins.push_back(temp_dst_pin);
    }

    //checking for gnd/vcc net
    auto gnd_gates = net->get_netlist()->get_global_gnd_gates();
    auto vcc_gates = net->get_netlist()->get_global_vcc_gates();
    for (auto g : vcc_gates)
    {
        if (src_gate == g)
        {
            type         = net_type::vcc_net;
            vcc_gnd_text = "'1'";
            break;
        }
    }
    for (auto g : gnd_gates)
    {
        if (src_gate == g)
        {
            type         = net_type::gnd_net;
            vcc_gnd_text = "'0'";
            break;
        }
    }
}

QRectF gui_graph_net::boundingRect() const
{
    QPainterPathStroker stroker;
    stroker.setWidth(10);
    return stroker.createStroke(path).boundingRect();
}

void gui_graph_net::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    //needed for a bugfix, surprisingly there are nets without a source gate(unrouted maybe)
    if (!src_gui_gate && type != (net_type::global_input))
        return;
    /*the zvalue is needed to push the selected net in the foreground(or put it back in the background)*/
    QPen l_pen;
    l_pen.setWidth(pen_width);
    setZValue(0);
    QColor actually_displayed_color = (!module_color_history.empty()) ? module_color_history.last().first : old_graphics_item_qss_adapter::instance()->net_default_color();

    if (isSelected())
    {
        this->setZValue(1);
        if(actually_displayed_color == old_graphics_item_qss_adapter::instance()->net_default_color())
            actually_displayed_color = old_graphics_item_qss_adapter::instance()->net_selected_color();
    }
    else
    {
        l_pen.setWidth((int)(pen_width * 0.75));
//        actually_displayed_color = (actually_displayed_color == old_graphics_item_qss_adapter::instance()->net_default_color())
//                                       ? old_graphics_item_qss_adapter::instance()->net_selected_color()
//                                       : QColor::fromHsv(actually_displayed_color.hue(), actually_displayed_color.saturation() - 100, actually_displayed_color.value());
        if(actually_displayed_color != old_graphics_item_qss_adapter::instance()->net_default_color())
            actually_displayed_color = QColor::fromHsv(actually_displayed_color.hue(), actually_displayed_color.saturation() - 100, actually_displayed_color.value());
    }

    l_pen.setColor(actually_displayed_color);
    painter->setPen(l_pen);
    painter->drawPath(path);

    //draw a circle at the end if the net is a global input- or output-net
    //sometimes the paths are empty, (maybe inout?), so test if the path is empty or not
    if ((type == net_type::global_output || type == net_type::global_input) && !path.isEmpty())
    {
        //painter->setBrush(QBrush(Qt::white));
        painter->setBrush(QBrush(old_graphics_item_qss_adapter::instance()->net_global_input_output_color()));
        painter->drawEllipse(pol.last(), 5, 5);
    }

    if (type == net_type::vcc_net || type == net_type::gnd_net)
    {
        painter->setFont(vcc_gnd_font);
        for (QPointF point : vcc_gnd_text_points)
            painter->drawText(point, vcc_gnd_text);
    }
}

QPainterPath gui_graph_net::shape() const
{
    /*this function returns the boundingrect by default (this is used for collision/selection detection)*/
    QPainterPathStroker stroker;
    stroker.setWidth(15);
    return stroker.createStroke(path);
}

void gui_graph_net::calculate_net(int sub_channel_a)
{
    sub_channel = sub_channel_a;

    /*TODO: change this unbelievable ugly piece of code*/
    switch (type)
    {
        case net_type::global_output:
            compute_global_output_net();
            break;
        case net_type::global_input:
            compute_global_input_net();
            break;
        case net_type::standard:
            compute_standard_net_v2(sub_channel_a);
            break;
        case net_type::gnd_net:
            compute_vcc_or_gnd_net();
            break;
        case net_type::vcc_net:
            compute_vcc_or_gnd_net();
            break;
    }
}

int gui_graph_net::get_sub_channel()
{
    return sub_channel;
}

void gui_graph_net::set_pen_width(int width)
{
    pen_width = width;
}

void gui_graph_net::open_edit_name_dialog()
{
    auto dialog = new input_dialog(nullptr, "New name:", QString::fromStdString(refNet->get_name()));
    if (dialog->exec() == QDialog::Accepted)
    {
        refNet->set_name(dialog->get_text().toStdString());
    }
    delete dialog;
}

std::shared_ptr<net> gui_graph_net::get_ref_net()
{
    return refNet;
}

void gui_graph_net::add_new_module_color(QColor color, int module_id)
{
    bool new_id = true;
    for (int i = 0; i < module_color_history.size(); i++)
    {
        if (module_color_history.at(i).second == module_id)
        {
            new_id = false;
            break;
        }
    }
    if (new_id)
        module_color_history.append(QPair<QColor, int>(color, module_id));
}

void gui_graph_net::remove_module_color(int module_id)
{
    for (int i = 0; i < module_color_history.size(); i++)
    {
        if (module_color_history.at(i).second == module_id)
        {
            module_color_history.removeAt(i);
            break;
        }
    }
}

void gui_graph_net::compute_standard_net_v2(int line)
{
    this->moveBy(src_pin.x(), src_pin.y());
    pol.append(QPointF(5, 0));

    //first, the coordinates from the destination need to be mapped to the items coordinate-system (the source is always (0,0) )
    for (QPointF point : dst_pins)
    {
        QPointF item_dest = this->mapFromScene(point);
        dst_pins_mapped.push_back(item_dest);
    }

    for (QPointF dst_point : dst_pins_mapped)
    {
        path = subPaths;

        if (0 < dst_point.x())
        {
            if (dst_point.x() > (gui_graph_gate::get_max_width() + m_layouter->get_channal_width()))    //bigger than channel_width, then "dodge"
            {
                //int distance_to_canal = gui_graph_gate::get_max_width() - src_gui_gate->get_width() + 20;//die 20 mit lines ersetzen //Unused
                int down = src_gui_gate->pos().y() + gui_graph_gate::get_max_height() - src_pin.y();

                //###[Version 2]###
                int to_right = src_gui_gate->pos().x() + gui_graph_gate::get_max_width() - src_pin.x();
                pol.append(QPointF(to_right + 30 + line * line_width, pol.last().y()));
                pol.append(QPointF(pol.last().x(), pol.last().y() + down + line * line_width / 4 + line_width));    //+line_width because line could be 0[4 because of channel_width_y]
                pol.append(QPointF(dst_point.x() - (line + 3) * line_width, pol.last().y()));                       //dst_points ist kein vielfaches von line_width...., iwas muss angepasst werden
                pol.append(QPointF(pol.last().x(), dst_point.y()));
                pol.append(QPointF(dst_point.x(), dst_point.y()));
                path.addPolygon(pol);
            }
            else
            {
                //ordinary left to right net if dodging is not needed
                //channel 0 is the most left, channel 1 the second most etc ###[Version 2]###
                int to_right = src_gui_gate->pos().x() + gui_graph_gate::get_max_width() - src_pin.x();
                pol.append(QPointF(to_right + 30 + line * line_width, pol.last().y()));
                pol.append(QPointF(pol.last().x(), dst_point.y()));
                pol.append(QPointF(dst_point.x(), pol.last().y()));
                path.addPolygon(pol);
            }
        }
        else
        {
            int down = src_gui_gate->pos().y() + gui_graph_gate::get_max_height() - src_pin.y();

            //->before line_width : 10
            pol.append(QPointF(20 + line_width * line, 0));
            pol.append(QPointF(pol.last().x(), down + (20 + 5 * line) / 4));                 //Hier die lines und die größte gate beachten beim runtergehen[4 because of channel_width_y]
            pol.append(QPointF(dst_point.x() - line_width * (line + 3), pol.last().y()));    //puffer for the new "0"and "1" displaying
            pol.append(QPointF(pol.last().x(), dst_point.y()));
            pol.append(QPointF(dst_point.x(), dst_point.y()));
            path.addPolygon(pol);
            //Von Rechts nach Links Netz
        }

        subPaths = path;
        subPaths.moveTo(5, 0);
        pol = QPolygonF();
        pol.append(QPointF(5, 0));
    }
}

void gui_graph_net::compute_global_input_net()
{
    if (dst_pins.size() == 1)
    {
        QPointF start = dst_pins.at(0);
        this->moveBy(start.x(), start.y());
        pol.append(QPointF(0, 0));
        pol.append(QPointF(pol.last().x() - 25, pol.last().y()));
        path.addPolygon(pol);
    }
    else if (dst_pins.size() == 0)
        log_debug("gui", "Error: Global-Input-Net has no Destinations");
    else
        log_debug("gui", "Error: Global-Input-Net has more than 1 Destination");
}

void gui_graph_net::compute_global_output_net()
{
    this->moveBy(src_pin.x(), src_pin.y());
    pol.append(QPointF(5, 0));
    pol.append(QPointF(pol.last().x() + 25, pol.last().y()));
    path.addPolygon(pol);
}

void gui_graph_net::compute_vcc_or_gnd_net()
{
    this->moveBy(src_pin.x(), src_pin.y());
    path.moveTo(QPointF(5, 0));
    path.lineTo(QPointF(path.currentPosition().x() + 20, path.currentPosition().y()));
    vcc_gnd_text_points.append(QPointF(path.currentPosition().x(), path.currentPosition().y() + 4));
    path.closeSubpath();

    for (QPointF point : dst_pins)
    {
        QPointF item_dest = this->mapFromScene(point);
        dst_pins_mapped.push_back(item_dest);

        path.moveTo(item_dest.x(), item_dest.y());
        path.lineTo(path.currentPosition().x() - 13, path.currentPosition().y());
        vcc_gnd_text_points.append(QPointF(path.currentPosition().x() - 13, path.currentPosition().y() + 4));
        path.closeSubpath();
    }
}
