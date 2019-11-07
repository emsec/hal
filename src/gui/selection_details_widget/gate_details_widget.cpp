#include "selection_details_widget/gate_details_widget.h"

#include "core/plugin_manager.h"

#include "netlist/gate.h"
#include "netlist/net.h"

#include "gui_globals.h"

#include "netlist/module.h"
#include "selection_details_widget/table_selector_widget.h"

#include <QApplication>
#include <QCursor>
#include <QDateTime>
#include <QDebug>
#include <QDesktopWidget>
#include <QHeaderView>
#include <QLabel>
#include <QResizeEvent>
#include <QScrollArea>
#include <QTableWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

gate_details_widget::gate_details_widget(QWidget* parent) : QWidget(parent)
{
    m_last_click_time = 0;

    m_content_layout = new QVBoxLayout(this);
    m_content_layout->setContentsMargins(0, 0, 0, 0);
    m_content_layout->setSpacing(0);

    m_general_table = new QTableWidget(this);
    m_general_table->horizontalHeader()->setStretchLastSection(true);
    m_general_table->horizontalHeader()->hide();
    m_general_table->verticalHeader()->hide();
    m_general_table->setColumnCount(2);
    m_general_table->setRowCount(4);

    //m_general_table->setStyleSheet("QTableWidget {background-color : rgb(31, 34, 35);}");

    QFont font("Iosevka");
    font.setBold(true);
    font.setPixelSize(13);

    m_item_deleted_label = new QLabel(this);
    m_item_deleted_label->setText("Currently selected item has been removed. Please consider relayouting the Graph.");
    m_item_deleted_label->setWordWrap(true);
    m_item_deleted_label->setAlignment(Qt::AlignmentFlag::AlignTop);
    m_item_deleted_label->setHidden(true);
    m_content_layout->addWidget(m_item_deleted_label);

    QTableWidgetItem* name_item = new QTableWidgetItem("Name:");
    name_item->setFlags(Qt::ItemIsEnabled);
    //    name_item->setTextColor(Qt::red);
    name_item->setFont(font);
    m_general_table->setItem(0, 0, name_item);

    QTableWidgetItem* type_item = new QTableWidgetItem("Type:");
    type_item->setFlags(Qt::ItemIsEnabled);
    type_item->setFont(font);
    m_general_table->setItem(1, 0, type_item);

    QTableWidgetItem* id_item = new QTableWidgetItem("ID  :");
    id_item->setFlags(Qt::ItemIsEnabled);
    id_item->setFont(font);
    m_general_table->setItem(2, 0, id_item);

    QTableWidgetItem* module_item = new QTableWidgetItem("modules: ");
    module_item->setFlags(Qt::ItemIsEnabled);
    module_item->setFont(font);
    m_general_table->setItem(3, 0, module_item);

    m_name_item = new QTableWidgetItem();
    m_name_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
    m_general_table->setItem(0, 1, m_name_item);

    m_type_item = new QTableWidgetItem();
    m_type_item->setFlags(Qt::ItemIsEnabled);
    m_general_table->setItem(1, 1, m_type_item);

    m_id_item = new QTableWidgetItem();
    m_id_item->setFlags(Qt::ItemIsEnabled);
    m_general_table->setItem(2, 1, m_id_item);

    m_module_item = new QTableWidgetItem();
    m_module_item->setFlags(Qt::ItemIsEnabled);
    m_general_table->setItem(3, 1, m_module_item);

    m_general_table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_general_table->verticalHeader()->setDefaultSectionSize(16);
    //    m_general_table->resizeRowsToContents();
    m_general_table->resizeColumnToContents(0);
    m_general_table->setShowGrid(false);
    m_general_table->setFocusPolicy(Qt::NoFocus);
    m_general_table->setFrameStyle(QFrame::NoFrame);
    m_general_table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_general_table->setFixedHeight(m_general_table->verticalHeader()->length());
    m_content_layout->addWidget(m_general_table);

    m_container_layout = new QVBoxLayout(this);
    m_container_layout->setContentsMargins(0, 0, 0, 0);
    m_container_layout->setSpacing(0);
    m_container_layout->setAlignment(Qt::AlignTop);

    m_boolean_function = new QLabel(this);
    //    m_boolean_function->setMargin(4);
    m_boolean_function->setWordWrap(true);
    //    m_boolean_function->setStyleSheet("* {background : red;}");
    //    m_boolean_function->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    m_container_layout->addWidget(m_boolean_function);

    m_init_value = new QLabel(this);
    //    m_init_value->setMargin(4);
    m_init_value->setWordWrap(true);
    m_container_layout->addWidget(m_init_value);

    m_container = new QWidget(this);
    m_container->setLayout(m_container_layout);
    m_container->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    m_tree_widget = new QTreeWidget(this);
    m_tree_widget->setFrameStyle(QFrame::NoFrame);
    m_tree_widget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    m_tree_widget->header()->hide();
    m_tree_widget->setSelectionMode(QAbstractItemView::NoSelection);
    m_tree_widget->setFocusPolicy(Qt::NoFocus);
    //    m_tree_widget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tree_widget->setFocusPolicy(Qt::NoFocus);
    m_tree_widget->headerItem()->setText(0, "");
    m_tree_widget->headerItem()->setText(1, "");
    m_tree_widget->headerItem()->setText(2, "");
    m_tree_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_tree_widget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_tree_widget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_container_layout->addWidget(m_tree_widget);

    connect(m_tree_widget, &QTreeWidget::itemExpanded, this, &gate_details_widget::handle_tree_size_change);
    connect(m_tree_widget, &QTreeWidget::itemCollapsed, this, &gate_details_widget::handle_tree_size_change);
    connect(m_tree_widget, &QTreeWidget::itemClicked, this, &gate_details_widget::on_treewidget_item_clicked);

    /*
    m_table_widget = new QTableWidget(0, 4);
    m_table_widget->setFrameStyle(QFrame::NoFrame);
    m_table_widget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    m_table_widget->setSelectionMode(QAbstractItemView::NoSelection);
    m_table_widget->setFocusPolicy(Qt::NoFocus);
    m_table_widget->setHorizontalHeaderLabels(QStringList() << "category"
                                                            << "type"
                                                            << "key"
                                                            << "data");
    //m_table_widget->horizontalHeader()->hide();
    m_table_widget->horizontalHeader()->setStyleSheet("QHeaderView::section {Background-Color: rgba(49, 51, 53, 1);}");
    m_table_widget->verticalHeader()->hide();
    m_table_widget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_table_widget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_tree_widget_container_layout->addWidget(m_table_widget);
    */

    m_scroll_area = new QScrollArea(this);
    m_scroll_area->setFrameStyle(QFrame::NoFrame);
    m_scroll_area->setWidgetResizable(true);
    m_scroll_area->setWidget(m_container);
    m_content_layout->addWidget(m_scroll_area);

    m_input_pins = new QTreeWidgetItem(m_tree_widget);
    m_input_pins->setExpanded(true);
    //    m_input_pins->setForeground(0, QBrush(QColor(114, 140, 0),Qt::SolidPattern));
    //    m_input_pins->setBackground(0, QBrush(QColor(31, 34, 35),Qt::SolidPattern));
    //    m_input_pins->setBackground(1, QBrush(QColor(31, 34, 35),Qt::SolidPattern));
    //    m_input_pins->setBackground(2, QBrush(QColor(31, 34, 35),Qt::SolidPattern));

    m_output_pins = new QTreeWidgetItem(m_tree_widget);
    m_output_pins->setExpanded(true);
    //    m_output_pins->setForeground(0, QBrush(QColor(114, 140, 0),Qt::SolidPattern));
    //    m_output_pins->setBackground(0, QBrush(QColor(31, 34, 35),Qt::SolidPattern));
    //    m_output_pins->setBackground(1, QBrush(QColor(31, 34, 35),Qt::SolidPattern));
    //    m_output_pins->setBackground(2, QBrush(QColor(31, 34, 35),Qt::SolidPattern));

    connect(&g_netlist_relay, &netlist_relay::gate_event, this, &gate_details_widget::handle_gate_event);
    connect(&g_netlist_relay, &netlist_relay::module_event, this, &gate_details_widget::handle_module_event);
}

gate_details_widget::~gate_details_widget()
{
    delete m_name_item;
    delete m_id_item;
    delete m_type_item;
    delete m_module_item;
    delete m_input_pins;
    delete m_output_pins;
}

void gate_details_widget::handle_gate_event(gate_event_handler::event ev, std::shared_ptr<gate> gate, u32 associated_data)
{
    Q_UNUSED(associated_data)

    //check if details widget currently shows the affected gate
    if (m_current_id == gate->get_id())
    {
        if (ev == gate_event_handler::event::removed)
        {
            m_general_table->setHidden(true);
            m_item_deleted_label->setHidden(false);
            m_scroll_area->setHidden(true);
        }
        else if (ev == gate_event_handler::event::name_changed)
        {
            update(gate->get_id());
        }
    }
}

void gate_details_widget::handle_module_event(module_event_handler::event ev, std::shared_ptr<module> module, u32 associated_data)
{
    if (ev == module_event_handler::event::gate_assigned || ev == module_event_handler::event::gate_removed)
    {
        if (m_current_id == associated_data)
        {
            update(m_current_id);
        }
    }
    else if (ev == module_event_handler::event::name_changed)
    {
        auto g = g_netlist->get_gate_by_id(m_current_id);

        if (module->contains_gate(g))
        {
            update(m_current_id);
        }
    }
}

void gate_details_widget::resizeEvent(QResizeEvent* event)
{
    m_container->setFixedWidth(event->size().width());
}

void gate_details_widget::update(const u32 gate_id)
{
    m_current_id = gate_id;

    auto g = g_netlist->get_gate_by_id(gate_id);

    m_name_item->setText(QString::fromStdString(g->get_name()));
    m_type_item->setText(QString::fromStdString(g->get_type()->get_name()));
    m_id_item->setText(QString::number(g->get_id()));

    //get modules
    QString module_text = "NONE";
    for (const auto sub : g_netlist->get_modules())
    {
        if (sub->contains_gate(g))
        {
            module_text = QString::fromStdString(sub->get_name()) + "[" + QString::number(sub->get_id()) + "]";
        }
    }
    m_module_item->setText(module_text);

    // display Boolean function (if present)
    m_boolean_function->setText("");

    std::string description = "";
    for (const auto& it : g->get_boolean_functions())
        description += " <b>Boolean Function</b>: " + it.first + " = " + it.second.to_string() + "<br>";

    if (description.empty())
    {
        g->set_data("gui", "boolean_function", "string", "-");
    }
    else
    {
        description = description.substr(0, description.size() - 4);
        m_boolean_function->setText(QString::fromStdString(description));
    }

    if (m_boolean_function->text().isEmpty())
        m_boolean_function->hide();
    else
        m_boolean_function->show();

    //display initial value if possible
    m_init_value->setText("");
    QString init_value_text = "";
    if (!std::get<1>(g->get_data_by_key("generic", "INIT")).empty())
    {
        init_value_text += "<b>INIT: </b>" + QString::fromStdString(std::get<1>(g->get_data_by_key("generic", "INIT")));
        m_init_value->setText(init_value_text);
    }

    if (m_init_value->text().isEmpty())
        m_init_value->hide();
    else
        m_init_value->show();

    disconnect(m_tree_widget, &QTreeWidget::itemClicked, this, 0);

    for (auto item : m_input_pins->takeChildren())
        delete item;

    auto input_pins = g_netlist->get_gate_by_id(gate_id)->get_input_pins();
    if (input_pins.size() == 1)
        m_input_pins->setText(0, "1 Input Pin");
    else
        m_input_pins->setText(0, QString::number(input_pins.size()) + " Input Pins");
    for (const auto& pin : input_pins)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem(m_input_pins);
        item->setText(0, QString::fromStdString(pin));
        item->setText(1, QChar(0x2b05));
        item->setForeground(1, QBrush(QColor(114, 140, 0), Qt::SolidPattern));

        auto input_net = g_netlist->get_gate_by_id(gate_id)->get_fan_in_net(pin);
        if (input_net == nullptr)
            item->setText(2, "unconnected");
        else
        {
            item->setText(2, QString::fromStdString(input_net->get_name()));
            item->setData(2, Qt::UserRole, input_net->get_id());
        }
    }

    for (auto item : m_output_pins->takeChildren())
        delete item;

    auto output_pins = g_netlist->get_gate_by_id(gate_id)->get_output_pins();
    if (output_pins.size() == 1)
        m_output_pins->setText(0, "1 Output Pin");
    else
        m_output_pins->setText(0, QString::number(output_pins.size()) + " Output Pins");
    for (const auto& pin : output_pins)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem(m_output_pins);
        item->setText(0, QString::fromStdString(pin));
        item->setText(1, QChar(0x27a1));
        item->setForeground(1, QBrush(QColor(114, 140, 0), Qt::SolidPattern));

        auto output_net = g_netlist->get_gate_by_id(gate_id)->get_fan_out_net(pin);
        if (output_net == nullptr)
            item->setText(2, "unconnected");
        else
        {
            item->setText(2, QString::fromStdString(output_net->get_name()));
            item->setData(2, Qt::UserRole, output_net->get_id());
        }
    }

    m_tree_widget->resizeColumnToContents(0);
    m_tree_widget->resizeColumnToContents(1);
    m_tree_widget->resizeColumnToContents(2);

    m_general_table->setHidden(false);
    m_scroll_area->setHidden(false);
    m_item_deleted_label->setHidden(true);

    connect(m_tree_widget, &QTreeWidget::itemClicked, this, &gate_details_widget::on_treewidget_item_clicked);

    //    std::set<std::string> inout_pins = gate->get_inout_pin_types();
    //    QTreeWidgetItem* inout_pin_item  = new QTreeWidgetItem(m_tree_widget);
    //    inout_pin_item->setText(0, QString::number(inout_pins.size()) + " Inout Pins");
    //    for (std::string type4 : inout_pins)
    //    {
    //        QTreeWidgetItem* item = new QTreeWidgetItem(inout_pin_item);
    //        item->setText(0, QString::fromStdString(type4));
    //    }
    //    std::set<net*> fan_in_nets = gate->get_fan_in_nets();
    //    QTreeWidgetItem* fan_in_item     = new QTreeWidgetItem(m_tree_widget);
    //    fan_in_item->setText(0, QString::number(fan_in_nets.size()) + " Fan In Nets");
    //    for (net* net : fan_in_nets)
    //    {
    //        QTreeWidgetItem* item = new QTreeWidgetItem(fan_in_item);
    //        item->setText(0, QString::fromStdString(net->get_name()));
    //    }
    //    std::set<net*> fan_out_nets = gate->get_fan_out_nets();
    //    QTreeWidgetItem* fan_out_item     = new QTreeWidgetItem(m_tree_widget);
    //    fan_out_item->setText(0, QString::number(fan_out_nets.size()) + " Fan Out Nets");
    //    for (net* net : fan_out_nets)
    //    {
    //        QTreeWidgetItem* item = new QTreeWidgetItem(fan_out_item);
    //        item->setText(0, QString::fromStdString(net->get_name()));
    //    }

    /*
    m_table_widget->setRowCount(0);
    std::map<std::tuple<std::string, std::string>, std::tuple<std::string, std::string>> gate_data = g->get_data();
    m_table_widget->setRowCount(gate_data.size());
    int i = 0;
    for (auto value : gate_data)
    {
        m_table_widget->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(std::get<0>(value.first))));
        m_table_widget->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(std::get<1>(value.first))));
        m_table_widget->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(std::get<0>(value.second))));
        m_table_widget->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(std::get<1>(value.second))));
        i++;
    }

    m_tree_widget->adjustSize();
    m_tree_widget->setColumnWidth(1, 50);
    m_tree_height = m_tree_widget->sizeHint().height();
    m_tree_widget->setMinimumHeight(m_tree_height);
    m_table_height = m_table_widget->verticalHeader()->length() + m_table_widget->horizontalHeader()->height();
    m_table_widget->setFixedHeight(m_table_height);
    m_table_widget->updateGeometry();
    m_tree_widget_container->setMinimumHeight(m_tree_height + m_table_height + m_spacing);
    m_table_widget->horizontalHeader()->setStretchLastSection(false);
    m_table_widget->resizeColumnToContents(3);
    m_table_widget->adjustSize();
    m_tree_widget->resizeColumnToContents(0);
    m_tree_widget_container->setMinimumWidth(m_table_widget->horizontalHeader()->length());
    m_table_widget->horizontalHeader()->setStretchLastSection(true);
    */
}

void gate_details_widget::handle_tree_size_change(QTreeWidgetItem* item)
{
    Q_UNUSED(item)

    //m_tree_widget->adjustSize();
    //m_tree_height = m_tree_widget->sizeHint().height();
    //m_tree_widget->setMinimumHeight(m_tree_height);
    //m_tree_widget_container->setMinimumHeight(m_tree_height + m_table_height + m_spacing);
    //m_tree_widget->updateGeometry();
}

void gate_details_widget::handle_item_expanded(QTreeWidgetItem* item)
{
    Q_UNUSED(item)
}

void gate_details_widget::handle_item_collapsed(QTreeWidgetItem* item)
{
    Q_UNUSED(item)
}

void gate_details_widget::on_treewidget_item_clicked(QTreeWidgetItem* item, int column)
{
    if (m_output_pins == item->parent() && column == 2)
    {
        int id                           = item->data(2, Qt::UserRole).toInt();
        std::shared_ptr<net> clicked_net = g_netlist->get_net_by_id(id);
        std::vector<endpoint> successors = clicked_net->get_dsts();
        std::set<std::shared_ptr<gate>> unique_succ;
        for (endpoint p : successors)
            unique_succ.insert(p.gate);

        if (successors.empty())
        {
            QList<u32> gate_ids;
            QList<u32> net_ids;
            QList<u32> submod_ids;
            net_ids.append(clicked_net->get_id());
            g_selection_relay.relay_combined_selection(this, gate_ids, net_ids, submod_ids);
            g_selection_relay.relay_current_net(this, net_ids.first());
            return;
        }

        if (unique_succ.size() == 1)
        {
            QList<u32> gate_id;
            QList<u32> net_ids;
            QList<u32> sub_ids;
            gate_id.append((*unique_succ.begin())->get_id());
            g_selection_relay.relay_combined_selection(this, gate_id, net_ids, sub_ids);
            update(gate_id.first());
            return;
        }

        table_selector_widget* w = new table_selector_widget(unique_succ, this);
        connect(w, &table_selector_widget::gateSelected, this, &gate_details_widget::on_gate_selected);
        auto rect = QApplication::desktop()->availableGeometry(this);
        w->move(QPoint(rect.x() + (rect.width() - w->width()) / 2, rect.y() + (rect.height() - w->height()) / 2));
        w->show();
        w->setFocus();
    }
    else if (m_input_pins == item->parent() && column == 2)
    {
        int id           = item->data(2, Qt::UserRole).toInt();
        auto clicked_net = g_netlist->get_net_by_id(id);

        if (!clicked_net)
            return;

        if (!clicked_net->get_src().gate)
        {
            QList<u32> gate_ids;
            QList<u32> net_ids;
            QList<u32> submod_ids;
            net_ids.append(clicked_net->get_id());
            g_selection_relay.relay_combined_selection(this, gate_ids, net_ids, submod_ids);
            g_selection_relay.relay_current_net(this, net_ids.first());
            return;
        }

        QList<u32> gate_id;
        QList<u32> net_ids;
        QList<u32> sub_ids;
        gate_id.append(clicked_net->get_src().gate->get_id());
        g_selection_relay.relay_combined_selection(this, gate_id, net_ids, sub_ids);
        update(gate_id.first());
    }
}

void gate_details_widget::on_gate_selected(std::shared_ptr<gate> selected)
{
    QList<u32> gate_id;
    QList<u32> net_ids;
    QList<u32> sub_ids;
    gate_id.append(selected->get_id());

    g_selection_relay.relay_combined_selection(this, gate_id, net_ids, sub_ids);
    update(gate_id.first());

    if (dynamic_cast<QWidget*>(sender()))
        static_cast<QWidget*>(sender())->close();
}
