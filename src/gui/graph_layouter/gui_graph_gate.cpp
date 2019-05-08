#include "graph_layouter/gui_graph_gate.h"
#include "core/log.h"
#include "gui_globals.h"
#include "netlist/net.h"

#include <QColorDialog>
#include <QDebug>
#include <QGraphicsScene>
#include <QPainter>
#include <QPen>
#include <QStyleOptionGraphicsItem>

#include "graph_layouter/input_dialog.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QGraphicsProxyWidget>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <functional>
#include "graph_layouter/old_graphics_item_qss_adapter.h"

int gui_graph_gate::MAX_HEIGHT = 0;

int gui_graph_gate::MAX_WIDTH = 0;

/*some hilarious big values*/
int gui_graph_gate::MIN_HEIGHT = 20000000;

int gui_graph_gate::MIN_WIDTH = 20000000;

gui_graph_gate::gui_graph_gate(std::shared_ptr<gate> gate, QGraphicsItem* parent)
{
    Q_UNUSED(parent)
    m_ref_gate = gate;

    number_output_pins = static_cast<int>(m_ref_gate->get_output_pin_types().size());
    number_input_pins  = static_cast<int>(m_ref_gate->get_input_pin_types().size());

    //calculate pin name width
    int counter = 0;
    pin_font    = QFont("Iosevka", 13, QFont::Bold);
    pin_font.setStyle(QFont::StyleOblique);

    //fill the input pin vector and calculate the maximal pin name width
    for (auto v : gate->get_input_pin_types())
    {
        pin temp_pin;
        temp_pin.name = QString::fromStdString(v);
        temp_pin.x    = 0;
        temp_pin.y    = counter * overall_pin_height;
        temp_pin.type = QString::fromStdString(v);
        input_pins.push_back(temp_pin);
        counter++;
        QGraphicsTextItem temp_item(QString::fromStdString(v));
        temp_item.setFont(pin_font);
        if ((int)temp_item.boundingRect().width() > input_pin_name_width)
            input_pin_name_width = (int)temp_item.boundingRect().width();
    }

    //calculate the maximal pin name width for the output pins, sadly you cant fill the pin vector yet (like above)
    for (auto v : gate->get_output_pin_types())
    {
        QGraphicsTextItem temp_item(QString::fromStdString(v));
        temp_item.setFont(pin_font);
        if ((int)temp_item.boundingRect().width() > output_pin_name_width)
            output_pin_name_width = (int)temp_item.boundingRect().width();
    }

    /*some steps to calculate the width of the name and then the overall width*/
    update_name();

    QString fix_width_string;
    for (int i = 0; i < 30; i++)
        fix_width_string.append(" ");
    QGraphicsTextItem fix_text_item(fix_width_string);
    fix_text_item.setFont(QFont(g_settings.value("font/family").toString(), 14, QFont::Bold));    //, QFont::PreferAntialias));

    name = new QGraphicsTextItem(text_gate_name);
    //name_font = QFont(g_settings.value("font/family").toString(), 14, QFont::Bold);    //, QFont::PreferAntialias);
    name_font = QFont("Iosevka", 14, QFont::Bold);
    name->setFont(name_font);
    //defaultColor = QColor(255, 255, 255);
    defaultColor = QColor(30,30,30);
    //gate_color   = QColor(255, 20, 20);
    defaultColor = old_graphics_item_qss_adapter::instance()->gate_default_color();
    gate_color = QColor(30, 30, 30);
    m_width      = fix_text_item.boundingRect().width() + input_pin_name_width + output_pin_name_width + (2 * pin_width);
    int zuviel   = m_width % 5;
    int strechy  = 0;
    if (zuviel != 0)
        strechy = strechy - (m_width % 5) + 5;

    m_width += strechy;

    /*calculate the overall height of the gate*/
    if (number_input_pins >= number_output_pins)
    {
        if (name->boundingRect().height() > number_input_pins * overall_pin_height)
            height = name->boundingRect().height();
        else
            height = number_input_pins * overall_pin_height;
    }
    else
    {
        if (name->boundingRect().height() > number_output_pins * overall_pin_height)
            height = name->boundingRect().height();
        else
            height = number_output_pins * overall_pin_height;
    }

    if (height > MAX_HEIGHT)
        MAX_HEIGHT = height;
    if (m_width > MAX_WIDTH)
        MAX_WIDTH = m_width;
    if (height < MIN_HEIGHT)
        MIN_HEIGHT = height;
    if (m_width < MIN_WIDTH)
        MIN_WIDTH = m_width;

    /*calculation and initialising of the required paint-onjects*/
    name_rect            = QRect(pin_width + input_pin_name_width, 0, fix_text_item.boundingRect().width(), height);
    input_pin_name_rect  = QRect(pin_width, 0, input_pin_name_width, height);
    output_pin_name_rect = QRect(pin_width + input_pin_name_width + fix_text_item.boundingRect().width(), 0, output_pin_name_width, height);
    drawn_outer_rect     = QRect(pin_width, 0, input_pin_name_width + output_pin_name_width + fix_text_item.boundingRect().width() + strechy, height);

    /*fill the output_pin-vector*/
    counter = 0;
    for (auto v : gate->get_output_pin_types())
    {
        pin temp_pin;
        //temp_pin.name = QString::fromStdString(m_ref_gate->get_graph()->get_output_pin_type_name(v));
        temp_pin.name = QString::fromStdString(v);
        temp_pin.x    = drawn_outer_rect.topRight().rx();
        temp_pin.y    = counter * overall_pin_height;
        temp_pin.type = QString::fromStdString(v);
        output_pins.push_back(temp_pin);
        counter++;
    }

    //load the QColor out of the node
    gate_color = defaultColor;

    //this->setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    //just testing
    this->setToolTip("Orig. Name: " + text_gate_name_without_type);
    //name is not needed anymore, so delete it
    delete name;
}

QRectF gui_graph_gate::boundingRect() const
{
    /*needs to be bigger than the actual painting for renderpuposes*/
    return QRectF(0, 0, m_width, height);
}

void gui_graph_gate::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    QPen pen;
    QColor actually_displayed_color;
    actually_displayed_color = (!module_color_history.empty()) ? module_color_history.last().first : old_graphics_item_qss_adapter::instance()->gate_default_color();
    if (isSelected())
    {
        //painter->fillRect(drawn_outer_rect, Qt::white);
        //painter->fillRect(drawn_outer_rect, Qt::black);
        painter->fillRect(drawn_outer_rect, old_graphics_item_qss_adapter::instance()->gate_selected_background_color());
    }
    else
    {
        //painter->fillRect(drawn_outer_rect, Qt::lightGray);
        painter->fillRect(drawn_outer_rect, old_graphics_item_qss_adapter::instance()->gate_background_color());
        actually_displayed_color = (actually_displayed_color == old_graphics_item_qss_adapter::instance()->gate_default_color())
                                       ? actually_displayed_color.darker(150)
                                       : actually_displayed_color.darker(110);//QColor::fromHsv(actually_displayed_color.hue(), actually_displayed_color.saturation() - 100, actually_displayed_color.value());
    }

    pen.setColor(actually_displayed_color);
    pen.setWidth(pen_width);
    painter->setPen(pen);

    const qreal lod = option->levelOfDetailFromTransform(painter->worldTransform());
    if (lod < 0.3)
    {
        painter->drawRect(drawn_outer_rect);
        return;    //just return, dont draw the rest below
    }

    QString displayed_name;
    if (text_gate_name_without_type.size() > 27)
        displayed_name = text_gate_name_without_type.left(27) + "..." + "\n(" + QString::fromStdString(m_ref_gate->get_type()) + ", ID: " + QString::number(m_ref_gate->get_id()) + ")";
    else
        displayed_name = text_gate_name_without_type + "\n(" + QString::fromStdString(m_ref_gate->get_type()) + ", ID: " + QString::number(m_ref_gate->get_id()) + ")";

    painter->drawRect(drawn_outer_rect);

    if (isSelected())
    {
        //pen.setColor(QColor(Qt::black));
        //pen.setColor(Qt::white);
        pen.setColor(old_graphics_item_qss_adapter::instance()->gate_selected_font_color());
    }
    else
        pen.setColor(old_graphics_item_qss_adapter::instance()->gate_font_color());

    painter->setPen(pen);

    QFont default_Font = painter->font();
    painter->setFont(name_font);
    //painter->drawText(name_rect, Qt::AlignCenter, text_gate_name);
    painter->drawText(name_rect, Qt::AlignCenter, displayed_name);
    painter->setFont(default_Font);

    paint_input_pins(painter);
    paint_output_pins(painter);

    //the following is a hack to draw the 1 and 0 instead of the net.. but sadly its not a net the way it is done now
    QFont vcc_gnd_font("Helvetica", 12);
    pen.setColor(QColor(Qt::white));
    painter->setPen(pen);
    painter->setFont(vcc_gnd_font);
    for (pin p : input_pins_vcc)
    {
        painter->drawLine(p.x - 5, p.y + 12, p.x - 18, p.y + 12);
        painter->drawText(p.x - 35, p.y + 16, "'1'");
    }

    for (pin p : input_pins_gnd)
    {
        painter->drawLine(p.x - 5, p.y + 12, p.x - 18, p.y + 12);
        painter->drawText(p.x - 35, p.y + 16, "'0'");
        //painter->drawText(p.x-20,p.y+16,"'0'");
    }

    //draw_module_color_squares(painter);
    painter->setFont(default_Font);
}

void gui_graph_gate::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(event)
    open_edit_name_dialog();
}

void gui_graph_gate::paint_input_pins(QPainter* painter)
{
    /*saves the default-font temporary so that it doesnt get lost*/
    QFont default_font = painter->font();
    painter->setFont(pin_font);

    for (unsigned int i = 0; i < input_pins.size(); i++)
    {
        pin input_pin = input_pins.at(i);
        if (input_pin_navigation_on && (input_pin_counter == (int)i))
        {
            painter->fillRect(input_pin.x, input_pin.y + (0.5 * drawn_pin_height), pin_width - (0.5 * pen_width), drawn_pin_height, QBrush(Qt::blue));
        }
        else
        {
            painter->fillRect(input_pin.x, input_pin.y + (0.5 * drawn_pin_height), pin_width - (0.5 * pen_width), drawn_pin_height, QBrush(QColor(255, 69, 0)));
        }

        QRect temp_pin_name_rect(input_pin.x + pin_width + pen_width + 2, input_pin.y + pen_width, input_pin_name_width, overall_pin_height);
        painter->drawText(temp_pin_name_rect, Qt::AlignLeft, input_pin.name);
    }

    painter->setFont(default_font);
}

void gui_graph_gate::draw_module_color_squares(QPainter* painter)
{
    int square_length             = 10;
    int square_length_plus_offset = square_length + 5;

    //seems redudant (the if statement could be inside the loop), but this would be less efficient..
    if (isSelected())
    {
        for (int i = 0; i < module_color_history.size(); i++)
        {
            if (i >= 5)
                break;
            painter->fillRect(name_rect.x() + 10 + (i * square_length_plus_offset),
                              name_rect.height() - square_length - 2,
                              square_length,
                              square_length,
                              module_color_history.at(module_color_history.size() - (i + 1)).first);
        }
    }
    else
    {
        for (int i = 0; i < module_color_history.size(); i++)
        {
            if (i >= 5)
                break;
            QColor currentColor = module_color_history.at(module_color_history.size() - (i + 1)).first;
            painter->fillRect(name_rect.x() + 10 + (i * square_length_plus_offset),
                              name_rect.height() - square_length - 2,
                              square_length,
                              square_length,
                              QColor::fromHsv(currentColor.hue(), currentColor.saturation() - 100, currentColor.value()));
        }
    }
}

void gui_graph_gate::paint_output_pins(QPainter* painter)
{
    /*saves the default-font temporary so that it doesnt get lost*/
    QFont default_font = painter->font();
    painter->setFont(pin_font);

    for (unsigned int i = 0; i < output_pins.size(); i++)
    {
        pin output_pin = output_pins.at(i);
        QRect temp_pin_name_rect(output_pin.x - output_pin_name_width - pen_width, output_pin.y, output_pin_name_width - 2, overall_pin_height);
        painter->drawText(temp_pin_name_rect, Qt::AlignRight, output_pin.name);
        //need to check if the pin-navigation is enabled
        if (output_pin_navogation_on && (output_pin_counter == (int)i))
            painter->fillRect(output_pin.x + pen_width, output_pin.y + (0.5 * drawn_pin_height), pin_width - pen_width, drawn_pin_height, QBrush(Qt::blue));
        else
            painter->fillRect(output_pin.x + pen_width, output_pin.y + (0.5 * drawn_pin_height), pin_width - pen_width, drawn_pin_height, QBrush(QColor(255, 69, 0)));
    }

    painter->setFont(default_font);
}

void gui_graph_gate::set_layout_position(QPoint new_position)
{
    layout_position = new_position;
}

void gui_graph_gate::set_layout_position(int position_x, int position_y)
{
    layout_position.setX(position_x);
    layout_position.setY(position_y);
}

QPoint gui_graph_gate::get_layout_position()
{
    return layout_position;
}

void gui_graph_gate::enable_output_pin_navigation(bool activate)
{
    input_pin_navigation_on  = false;
    output_pin_navogation_on = activate;
}

void gui_graph_gate::enable_input_pin_navigation(bool activate)
{
    output_pin_navogation_on = false;
    input_pin_navigation_on  = activate;
}
//may check for negative or out of bounds
void gui_graph_gate::increase_current_pin()
{
    if (output_pin_navogation_on)
    {
        if (output_pin_counter < (int)output_pins.size() - 1)
            output_pin_counter++;
    }
    else
    {
        if (input_pin_counter < (int)input_pins.size() - 1)
            input_pin_counter++;
    }
}

void gui_graph_gate::decrease_current_pin()
{
    if (output_pin_navogation_on)
    {
        if (output_pin_counter > 0)
            output_pin_counter--;
    }
    else
    {
        if (input_pin_counter > 0)
            input_pin_counter--;
    }
}

std::string gui_graph_gate::get_current_pin_type()
{
    if (output_pin_navogation_on)
        return output_pins.at(output_pin_counter).type.toStdString();
    if (input_pin_navigation_on)
        return input_pins.at(input_pin_counter).type.toStdString();
    return "";
}

int gui_graph_gate::output_pin_count()
{
    return output_pins.size();
}

void gui_graph_gate::update_name()
{
    text_gate_name_without_type = QString::fromStdString(m_ref_gate->get_name());
    text_gate_name              = QString::fromStdString(m_ref_gate->get_name());
    text_gate_name              = text_gate_name + "\n(" + QString::fromStdString(m_ref_gate->get_type() + ")");
    if (this->scene() != nullptr)
    {
        this->scene()->update();
    }
}

void gui_graph_gate::set_color(QColor color)
{
    gate_color = color;
    this->scene()->update();
}

void gui_graph_gate::add_new_module_color(QColor color, int id)
{
    //check if the id is already in the history
    bool new_id = true;
    for (int i = 0; i < module_color_history.size(); i++)
    {
        if (module_color_history.at(i).second == id)
        {
            new_id = false;
            break;
        }
    }
    if (new_id)
        module_color_history.append(QPair<QColor, int>(color, id));
}

void gui_graph_gate::remove_module_color(int module_id)
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

void gui_graph_gate::open_edit_name_dialog()
{
    auto dialog = new input_dialog(nullptr, "New name:", QString::fromStdString(m_ref_gate->get_name()));
    if (dialog->exec() == QDialog::Accepted)
    {
        m_ref_gate->set_name(dialog->get_text().toStdString());
    }
    delete dialog;
}

void gui_graph_gate::change_color()
{
    QColor new_color = QColorDialog::getColor();

    if (new_color.isValid())
        set_color(new_color);
}

int gui_graph_gate::input_pin_count()
{
    return input_pins.size();
}

int gui_graph_gate::get_max_height()
{
    return MAX_HEIGHT;
}

int gui_graph_gate::get_max_width()
{
    return MAX_WIDTH;
}

int gui_graph_gate::get_min_height()
{
    return MIN_HEIGHT;
}

int gui_graph_gate::get_min_widht()
{
    return MIN_WIDTH;
}

void gui_graph_gate::reset_height_and_width()
{
    MAX_HEIGHT = 0;
    MAX_WIDTH  = 0;
    MIN_HEIGHT = 20000000;
    MIN_WIDTH  = 20000000;
}

void gui_graph_gate::set_pen_width(int new_width)
{
    pen_width = new_width;
}

int gui_graph_gate::get_width()
{
    return m_width;
}

int gui_graph_gate::get_height()
{
    return height;
}

std::string gui_graph_gate::get_input_pin_type_at_position(unsigned int pos)
{
    if (input_pins.size() == 0 || pos >= input_pins.size())    //pos < 0 removed
        return "";
    else
        return input_pins.at(pos).type.toStdString();
}

std::string gui_graph_gate::get_output_pin_type_at_position(unsigned int pos)
{
    if (output_pins.size() == 0 || pos >= output_pins.size())    //pos < 0 removed
        return "";
    else
        return output_pins.at(pos).type.toStdString();
}

std::shared_ptr<gate> gui_graph_gate::get_ref_gate()
{
    return m_ref_gate;
}

QPointF gui_graph_gate::get_unmapped_input_pin_coordinates(QString type)
{
    for (pin temp_pin : input_pins)
    {
        if (temp_pin.type == type)
        {
            return QPointF(temp_pin.x, temp_pin.y + overall_pin_height * 0.5);
        }
    }
    return QPointF(-1, -1);    //if the gate has no pin with the specified type, return a point with negative values(negative values are impossible)
}

QPointF gui_graph_gate::get_unmapped_output_pin_coordinates(QString type)
{
    for (pin temp_pin : output_pins)
    {
        if (temp_pin.type == type)
        {
            int x_overhead = (temp_pin.x + pin_width) % 5;
            return QPointF(temp_pin.x + pin_width - x_overhead + 5, temp_pin.y + overall_pin_height * 0.5);
        }
    }
    return QPointF(-1, -1);    //if the gate has no pin with the specified type, return a point with negative values(negative values are impossible)
}

/*doesnt work,dont know why*/
QPointF gui_graph_gate::get_mapped_input_pin_coordinates(QString type)
{
    //sadly i cant return a "wrong" value
    for (pin temp_pin : input_pins)
    {
        if (temp_pin.type == type)
            return this->mapToScene(temp_pin.x, temp_pin.y + overall_pin_height * 0.5);
    }
    return QPointF();
}

/*doesnt work,dont know why*/
QPointF gui_graph_gate::get_mapped_output_pin_coordinates(QString type)
{
    //sadly i cant return a "wrong" value
    for (pin temp_pin : input_pins)
    {
        if (temp_pin.type == type)
            return this->mapToScene(temp_pin.x + pin_width, temp_pin.y + 1.5 * drawn_pin_height);
    }
    return QPointF();
}
