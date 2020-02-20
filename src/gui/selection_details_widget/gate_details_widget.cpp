#include "selection_details_widget/gate_details_widget.h"

#include "core/plugin_manager.h"

#include "netlist/gate.h"
#include "netlist/net.h"

#include "gui_globals.h"

#include "graph_widget/graph_navigation_widget.h"
#include "netlist/module.h"

#include <QApplication>
#include <QCursor>
#include <QDateTime>
#include <QDesktopWidget>
#include <QHBoxLayout>
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
    m_current_id = 0;

    m_last_click_time = 0;

    m_content_layout = new QVBoxLayout(this);
    m_content_layout->setContentsMargins(0, 0, 0, 0);
    m_content_layout->setSpacing(0);

    m_tree_row_layout = new QHBoxLayout();

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

    QTableWidgetItem* module_item = new QTableWidgetItem("Module: ");
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
    //m_general_table->setStyleSheet("QTableWidget{color: red;}");

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

    m_data_fields = new QLabel(this);
    //    m_init_value->setMargin(4);
    m_data_fields->setWordWrap(true);
    m_container_layout->addWidget(m_data_fields);

    m_container = new QWidget(this);
    m_container->setLayout(m_container_layout);
    m_container->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    m_tree_widget = new QTreeWidget(this);
    m_tree_widget->setFrameStyle(QFrame::NoFrame);
    m_tree_widget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    m_tree_widget->header()->hide();
    m_tree_widget->setSelectionMode(QAbstractItemView::NoSelection);
    m_tree_widget->setFocusPolicy(Qt::NoFocus);
    m_tree_widget->headerItem()->setText(0, "");
    m_tree_widget->headerItem()->setText(1, "");
    m_tree_widget->headerItem()->setText(2, "");
    m_tree_widget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_tree_widget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_tree_widget->header()->setStretchLastSection(false);
    //by setting this you do not need to call resizecolumntocontents every time after updating
    m_tree_widget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_tree_widget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    m_tree_row_layout->addWidget(m_tree_widget);
    m_tree_row_layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum));
    m_tree_row_layout->addSpacing(0);
    m_container_layout->addLayout(m_tree_row_layout);

    connect(m_tree_widget, &QTreeWidget::itemClicked, this, &gate_details_widget::on_treewidget_item_clicked);

    //set parent and flag so it gets shown in its own window
    m_navigation_table = new graph_navigation_widget();
    m_navigation_table->setWindowFlags(Qt::CustomizeWindowHint);
    m_navigation_table->hide_when_focus_lost(true);
    m_navigation_table->hide();
    connect(m_navigation_table, &graph_navigation_widget::navigation_requested, this, &gate_details_widget::handle_navigation_jump_requested);

    m_scroll_area = new QScrollArea(this);
    m_scroll_area->setFrameStyle(QFrame::NoFrame);
    m_scroll_area->setWidgetResizable(true);
    m_scroll_area->setWidget(m_container);
    m_content_layout->addWidget(m_scroll_area);

    m_input_pins = new QTreeWidgetItem(m_tree_widget);
    m_input_pins->setExpanded(true);

    m_output_pins = new QTreeWidgetItem(m_tree_widget);
    m_output_pins->setExpanded(true);

    connect(m_general_table, &QTableWidget::doubleClicked, this, &gate_details_widget::on_general_table_item_double_clicked);

    //handle netlist modifications regarding gates
    connect(&g_netlist_relay, &netlist_relay::gate_name_changed, this, &gate_details_widget::handle_gate_name_changed);
    connect(&g_netlist_relay, &netlist_relay::gate_removed, this, &gate_details_widget::handle_gate_removed);

    //handle netlist modifications regarding modules
    connect(&g_netlist_relay, &netlist_relay::module_name_changed, this, &gate_details_widget::handle_module_name_changed);
    connect(&g_netlist_relay, &netlist_relay::module_removed, this, &gate_details_widget::handle_module_removed);
    connect(&g_netlist_relay, &netlist_relay::module_gate_assigned, this, &gate_details_widget::handle_module_gate_assigned);
    connect(&g_netlist_relay, &netlist_relay::module_gate_removed, this, &gate_details_widget::handle_module_gate_removed);

    //handle netlist modifications reagarding nets
    connect(&g_netlist_relay, &netlist_relay::net_name_changed, this, &gate_details_widget::handle_net_name_changed);
    connect(&g_netlist_relay, &netlist_relay::net_source_changed, this, &gate_details_widget::handle_net_source_changed);
    connect(&g_netlist_relay, &netlist_relay::net_destination_added, this, &gate_details_widget::handle_net_destination_added);
    connect(&g_netlist_relay, &netlist_relay::net_destination_removed, this, &gate_details_widget::handle_net_destination_removed);
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

void gate_details_widget::handle_gate_name_changed(std::shared_ptr<gate> gate)
{
    if (m_current_id == gate->get_id())
        update(m_current_id);
}

void gate_details_widget::handle_gate_removed(std::shared_ptr<gate> gate)
{
    if (m_current_id == gate->get_id())
    {
        m_general_table->setHidden(true);
        m_scroll_area->setHidden(true);
    }
}

void gate_details_widget::handle_net_name_changed(std::shared_ptr<net> net)
{
    bool update_needed = false;

    //check if currently shown gate is src of renamed net
    if (m_current_id == net->get_source().get_gate()->get_id())
        update_needed = true;

    //check if currently shown gate is dst of renamed net
    if (!update_needed)
    {
        for (auto& e : net->get_destinations())
        {
            if (m_current_id == e.get_gate()->get_id())
            {
                update_needed = true;
                break;
            }
        }
    }

    if (update_needed)
        update(m_current_id);
}

void gate_details_widget::handle_net_source_changed(std::shared_ptr<net> net)
{
    Q_UNUSED(net);
    if (m_current_id == 0)
        return;
    if (g_netlist->is_gate_in_netlist(g_netlist->get_gate_by_id(m_current_id)))
        update(m_current_id);
}

void gate_details_widget::handle_net_destination_added(std::shared_ptr<net> net, const u32 dst_gate_id)
{
    Q_UNUSED(net);
    if (m_current_id == dst_gate_id)
        update(m_current_id);
}

void gate_details_widget::handle_net_destination_removed(std::shared_ptr<net> net, const u32 dst_gate_id)
{
    Q_UNUSED(net);
    if (m_current_id == dst_gate_id)
        update(m_current_id);
}

void gate_details_widget::handle_module_removed(std::shared_ptr<module> module)
{
    if (m_current_id == 0)
        return;
    auto g = g_netlist->get_gate_by_id(m_current_id);

    if (module->contains_gate(g))
    {
        update(m_current_id);
    }
}

void gate_details_widget::handle_module_name_changed(std::shared_ptr<module> module)
{
    if (m_current_id == 0)
        return;
    auto g = g_netlist->get_gate_by_id(m_current_id);

    if (module->contains_gate(g))
    {
        update(m_current_id);
    }
}

void gate_details_widget::handle_module_gate_assigned(std::shared_ptr<module> module, u32 associated_data)
{
    Q_UNUSED(module);
    if (m_current_id == associated_data)
    {
        update(m_current_id);
    }
}

void gate_details_widget::handle_module_gate_removed(std::shared_ptr<module> module, u32 associated_data)
{
    Q_UNUSED(module);
    if (!g_netlist->is_gate_in_netlist(g_netlist->get_gate_by_id(associated_data)))
        return;

    if (m_current_id == associated_data)
    {
        update(m_current_id);
    }
}

void gate_details_widget::resizeEvent(QResizeEvent* event)
{
    m_container->setFixedWidth(event->size().width());
}

void gate_details_widget::update(const u32 gate_id)
{
    m_current_id = gate_id;

    if (m_current_id == 0)
        return;

    auto g = g_netlist->get_gate_by_id(gate_id);

    if (!g)
        return;

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
            m_module_item->setData(Qt::UserRole, sub->get_id());
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

    // display all data fields
    m_data_fields->setText("");
    std::string data_str = "";
    auto data_fields     = g->get_data();

    for (const auto& [key, value] : data_fields)
    {
        data_str += "<b>" + std::get<1>(key) + ": </b>" + std::get<1>(value) + "<br>";
    }

    if (!data_str.empty())
    {
        data_str = data_str.substr(0, data_str.size() - 4);
        m_data_fields->setText(QString::fromStdString(data_str));

        if (m_data_fields->text().isEmpty())
        {
            m_data_fields->hide();
        }
        else
        {
            m_data_fields->show();
        }
    }
    else
    {
        m_data_fields->hide();
    }

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

    m_general_table->setHidden(false);
    m_scroll_area->setHidden(false);

    connect(m_tree_widget, &QTreeWidget::itemClicked, this, &gate_details_widget::on_treewidget_item_clicked);
}

void gate_details_widget::on_treewidget_item_clicked(QTreeWidgetItem* item, int column)
{
    auto net_id = item->data(2, Qt::UserRole).toInt();
    if (m_output_pins == item->parent() && column == 2)
    {
        std::shared_ptr<net> clicked_net = g_netlist->get_net_by_id(net_id);

        if (!clicked_net)
            return;

        auto destinations = clicked_net->get_destinations();

        if (destinations.empty() || clicked_net->is_global_output_net())
        {
            g_selection_relay.clear();
            g_selection_relay.m_selected_nets.insert(clicked_net->get_id());
            g_selection_relay.relay_selection_changed(this);
        }
        else if (destinations.size() == 1)
        {
            auto ep = *destinations.begin();
            g_selection_relay.clear();
            g_selection_relay.m_selected_gates.insert(ep.get_gate()->get_id());
            g_selection_relay.m_focus_type = selection_relay::item_type::gate;
            g_selection_relay.m_focus_id   = ep.get_gate()->get_id();
            g_selection_relay.m_subfocus   = selection_relay::subfocus::left;

            auto pins                          = ep.get_gate()->get_input_pins();
            auto index                         = std::distance(pins.begin(), std::find(pins.begin(), pins.end(), ep.get_pin()));
            g_selection_relay.m_subfocus_index = index;

            update(ep.get_gate()->get_id());
            g_selection_relay.relay_selection_changed(this);
        }
        else
        {
            //            auto rect = QApplication::desktop()->availableGeometry(this);
            //            w->move(QPoint(rect.x() + (rect.width() - w->width()) / 2, rect.y() + (rect.height() - w->height()) / 2));
            m_navigation_table->setup(hal::node{hal::node_type::gate, 0}, clicked_net);
            m_navigation_table->move(QCursor::pos());
            m_navigation_table->show();
            m_navigation_table->setFocus();
        }
    }
    else if (m_input_pins == item->parent() && column == 2)
    {
        auto clicked_net = g_netlist->get_net_by_id(net_id);

        if (!clicked_net)
            return;

        g_selection_relay.clear();
        if (clicked_net->get_source().get_gate() == nullptr || clicked_net->is_global_input_net())
        {
            g_selection_relay.m_selected_nets.insert(clicked_net->get_id());
        }
        else
        {
            endpoint ep  = clicked_net->get_source();
            auto gate_id = ep.get_gate()->get_id();
            g_selection_relay.m_selected_gates.insert(gate_id);
            g_selection_relay.m_focus_type = selection_relay::item_type::gate;
            g_selection_relay.m_focus_id   = ep.get_gate()->get_id();
            g_selection_relay.m_subfocus   = selection_relay::subfocus::right;

            auto pins                          = ep.get_gate()->get_output_pins();
            auto index                         = std::distance(pins.begin(), std::find(pins.begin(), pins.end(), ep.get_pin()));
            g_selection_relay.m_subfocus_index = index;
            update(gate_id);
        }
        g_selection_relay.relay_selection_changed(this);
    }
}

//always the right-subfocus!!!!!!(the other way is handled: on_treewidget_item_clicked
void gate_details_widget::handle_navigation_jump_requested(const hal::node origin, const u32 via_net, const QSet<u32>& to_gates)
{
    Q_UNUSED(origin);

    auto n = g_netlist->get_net_by_id(via_net);

    if (to_gates.isEmpty() || !n)
        return;
    for (u32 id : to_gates)
    {
        if (!g_netlist->get_gate_by_id(id))
            return;
    }

    m_navigation_table->hide();
    g_selection_relay.clear();
    g_selection_relay.m_selected_gates = to_gates;
    if (to_gates.size() == 1)
    {
        g_selection_relay.m_focus_type = selection_relay::item_type::gate;
        auto g                         = g_netlist->get_gate_by_id(*to_gates.constBegin());
        g_selection_relay.m_focus_id   = g->get_id();
        g_selection_relay.m_subfocus   = selection_relay::subfocus::left;

        u32 index_cnt = 0;
        for (const auto& pin : g->get_input_pins())
        {
            if (g->get_fan_in_net(pin) == n)
            {
                g_selection_relay.m_subfocus_index = index_cnt;
                break;
            }
            index_cnt++;
        }

        g_selection_relay.relay_selection_changed(this);
    }
    m_navigation_table->hide();

    // TODO ensure gates visible in graph
}

void gate_details_widget::on_general_table_item_double_clicked(const QModelIndex& index)
{
    if (!index.isValid())
        return;

    //cant get the item from the index (static_cast<QTableWidgetItem*>(index.internalPointer()) fails),
    //so ask the item QTableWidgetItem directly
    if (index.row() == m_module_item->row() && index.column() == m_module_item->column())
    {
        g_selection_relay.clear();
        g_selection_relay.m_selected_modules.insert(m_module_item->data(Qt::UserRole).toInt());
        g_selection_relay.relay_selection_changed(this);
    }
}
