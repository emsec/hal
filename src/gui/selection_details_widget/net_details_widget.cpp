#include "selection_details_widget/net_details_widget.h"
#include "gui_globals.h"
#include "netlist/gate.h"
#include "netlist/module.h"
#include "netlist/net.h"
#include <QHeaderView>
#include <QLabel>
#include <QScrollArea>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

net_details_widget::net_details_widget(QWidget* parent) : QWidget(parent)
{
    m_content_layout = new QVBoxLayout(this);
    m_content_layout->setContentsMargins(0, 0, 0, 0);
    m_content_layout->setSpacing(0);
    m_content_layout->setAlignment(Qt::AlignTop);

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

    QTableWidgetItem* module_item = new QTableWidgetItem("modules:");
    id_item->setFlags(Qt::ItemIsEnabled);
    id_item->setFont(font);
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

    connect(m_tree_widget, &QTreeWidget::itemExpanded, this, &net_details_widget::handle_tree_size_change);
    connect(m_tree_widget, &QTreeWidget::itemCollapsed, this, &net_details_widget::handle_tree_size_change);

    m_scroll_area = new QScrollArea(this);
    m_scroll_area->setFrameStyle(QFrame::NoFrame);
    m_scroll_area->setWidgetResizable(true);
    m_scroll_area->setWidget(m_container);
    m_content_layout->addWidget(m_scroll_area);

    m_src_pin = new QTreeWidgetItem(m_tree_widget);
    m_src_pin->setExpanded(true);

    m_dst_pins = new QTreeWidgetItem(m_tree_widget);
    m_dst_pins->setExpanded(true);

    connect(&g_netlist_relay, &netlist_relay::net_event, this, &net_details_widget::handle_net_event);
}

net_details_widget::~net_details_widget()
{
    delete m_name_item;
    delete m_type_item;
    delete m_id_item;
    delete m_src_pin;
    delete m_dst_pins;
}

void net_details_widget::handle_net_event(net_event_handler::event ev, std::shared_ptr<net> net, u32 associated_data)
{
    Q_UNUSED(associated_data)

    //check if details widget currently show the affected net
    if (m_current_id == net->get_id())
    {
        if (ev == net_event_handler::event::removed)
        {
            m_general_table->setHidden(true);
            m_scroll_area->setHidden(true);
            m_item_deleted_label->setHidden(false);
        }
        else
        {
            update(net->get_id());
        }
    }
}

void net_details_widget::update(u32 net_id)
{
    m_current_id = net_id;

    auto n = g_netlist->get_net_by_id(net_id);

    //get name
    m_name_item->setText(QString::fromStdString(n->get_name()));

    //get net type
    QString n_type = "Standard";

    if (g_netlist->is_global_input_net(n))
        n_type = "Input";
    else if (g_netlist->is_global_output_net(n))
        n_type = "Output";

    m_type_item->setText(n_type);

    //get id
    m_id_item->setText(QString::number(n->get_id()));

    //get modules
    QString module_text = "";
    m_module_item->setText(module_text);

    //get src pin
    for (auto item : m_src_pin->takeChildren())
        delete item;

    auto src_pin = n->get_src();

    if (src_pin.get_gate() != nullptr)
    {
        auto src_pin_type = src_pin.get_pin_type();

        if (!src_pin_type.empty())
        {
            QTreeWidgetItem* item = new QTreeWidgetItem(m_src_pin);
            m_src_pin->setText(0, "1 Source Pin");
            item->setText(0, QString::fromStdString(src_pin_type));
            item->setText(1, QChar(0x2b05));
            item->setForeground(1, QBrush(QColor(114, 140, 0), Qt::SolidPattern));
            item->setText(2, QString::fromStdString(src_pin.get_gate()->get_name()));
        }
    }
    else
    {
        m_src_pin->setText(0, "No Source Pin");
    }

    //get destination pins
    for (auto item : m_dst_pins->takeChildren())
        delete item;

    m_dst_pins->setText(0, "");

    if (!g_netlist->is_global_output_net(n))
    {
        auto dsts_pins = n->get_dsts();

        QString dst_text = QString::number(dsts_pins.size()) + " Destination Pins";

        if (dsts_pins.size() == 1)
            dst_text.chop(1);

        m_dst_pins->setText(0, dst_text);

        for (auto dst_pin : dsts_pins)
        {
            auto dst_pin_type = dst_pin.get_pin_type();

            QTreeWidgetItem* item = new QTreeWidgetItem(m_dst_pins);
            item->setText(0, QString::fromStdString(dst_pin_type));
            item->setText(1, QChar(0x27a1));
            item->setForeground(1, QBrush(QColor(114, 140, 0), Qt::SolidPattern));
            item->setText(2, QString::fromStdString(dst_pin.get_gate()->get_name()));
        }
    }

    m_general_table->setHidden(false);
    m_scroll_area->setHidden(false);
    m_item_deleted_label->setHidden(true);

    //    for (auto item : m_src_pin->takeChildren())
    //        delete item;

    //    std::set<std::string> src_pins = g_netlist->get_gate_by_id(gate_id)->get_input_pin_types();
    //    if (src_pins.size() == 1)
    //        m_src_pin->setText(0, "1 Source Pin");
    //    else
    //        m_src_pin->setText(0, QString::number(src_pins.size()) + " Source Pins");
    //    for (const auto& pin : src_pins)
    //    {
    //        QTreeWidgetItem* item = new QTreeWidgetItem(m_src_pin);
    //        item->setText(0, QString::fromStdString(pin));
    //        item->setText(1, QChar(0x2b05));
    //        item->setForeground(1, QBrush(QColor(114, 140, 0),Qt::SolidPattern));

    //        auto input_net = g_netlist->get_gate_by_id(gate_id)->get_fan_in_net(pin);
    //        if (input_net == nullptr)
    //            item->setText(2, "unconnected");
    //        else
    //            item->setText(2, QString::fromStdString(input_net->get_name()));
    //    }

    //    for (auto item : m_output_pins->takeChildren())
    //        delete item;

    //    std::set<std::string> output_pins = g_netlist->get_gate_by_id(gate_id)->get_output_pin_types();
    //    if (output_pins.size() == 1)
    //        m_output_pins->setText(0, "1 Output Pin");
    //    else
    //        m_output_pins->setText(0, QString::number(output_pins.size()) + " Output Pins");
    //    for (const auto& pin : output_pins)
    //    {
    //        QTreeWidgetItem* item = new QTreeWidgetItem(m_output_pins);
    //        item->setText(0, QString::fromStdString(pin));
    //        item->setText(1, QChar(0x27a1));
    //        item->setForeground(1, QBrush(QColor(114, 140, 0),Qt::SolidPattern));

    //        auto output_net = g_netlist->get_gate_by_id(gate_id)->get_fan_out_net(pin);
    //        if (output_net == nullptr)
    //            item->setText(2, "unconnected");
    //        else
    //            item->setText(2, QString::fromStdString(output_net->get_name()));
    //    }

    m_tree_widget->resizeColumnToContents(0);
    m_tree_widget->resizeColumnToContents(1);
    m_tree_widget->resizeColumnToContents(2);

    //    //    m_tree_widget->clear();
    //    //    auto tupel = net->get_src();
    //    //    QTreeWidgetItem* src_item = new QTreeWidgetItem(m_tree_widget);
    //    //    gate* gate          = tupel.gate;
    //    //    QString gate_name;
    //    //    QString pin;
    //    //    if (gate)
    //    //    {
    //    //        gate_name = QString::fromStdString(gate->get_name());
    //    //        pin       = "(" + QString::fromStdString(tupel.pin_type) + ")";
    //    //    }
    //    //    else
    //    //    {
    //    //        gate_name = "Nullptr :(";
    //    //        pin       = "";
    //    //    }
    //    //    src_item->setText(0, "1 Source gate (Pin)");
    //    //    QTreeWidgetItem* item = new QTreeWidgetItem(src_item);
    //    //    item->setText(0, gate_name + " " + pin);

    //    //    QTreeWidgetItem* dsts_item = new QTreeWidgetItem(m_tree_widget);
    //    //    for (const auto& tupel2 : net->get_dsts())
    //    //    {
    //    //        gate = tupel2.gate;
    //    //        if (gate)
    //    //        {
    //    //            gate_name = QString::fromStdString(gate->get_name());
    //    //            pin       = "(" + QString::fromStdString(tupel2.pin_type) + ")";
    //    //        }
    //    //        else
    //    //        {
    //    //            gate_name = "Returned Nullptr :(";
    //    //            pin       = "";
    //    //        }
    //    //        QTreeWidgetItem* item2 = new QTreeWidgetItem(dsts_item);
    //    //        item2->setText(0, gate_name + " " + pin);
    //    //    }

    //    //    dsts_item->setText(0, QString::number(net->get_num_of_dsts()) + " Destination gates (Pin)");

    //    QList<QTreeWidgetItem*> list = m_src_item->takeChildren();
    //    for (QTreeWidgetItem* item : list)
    //        delete item;
    //    auto src_tupel = net->get_src();
    //    auto gate      = src_tupel.gate;
    //    QString gate_name;
    //    QString pin;
    //    if (gate)
    //    {
    //        gate_name = QString::fromStdString(gate->get_name());
    //        pin       = "(" + QString::fromStdString(src_tupel.pin_type) + ")";
    //    }
    //    else
    //    {
    //        gate_name = "Nullptr :(";
    //        pin       = "";
    //    }
    //    m_src_item->setText(0, "1 Source gate (Pin)");
    //    QTreeWidgetItem* item = new QTreeWidgetItem(m_src_item);
    //    item->setText(0, gate_name);
    //    //item->setText(1, QChar(0x2b05));
    //    item->setText(1, pin);

    //    list = m_dst_item->takeChildren();
    //    for (QTreeWidgetItem* item2 : list)
    //        delete item2;
    //    m_dst_item->setText(0, QString::number(net->get_num_of_dsts()) + " Destination gates (Pin)");
    //    for (const auto& dst_tupel : net->get_dsts())
    //    {
    //        gate = dst_tupel.gate;
    //        if (gate)
    //        {
    //            gate_name = QString::fromStdString(gate->get_name());
    //            pin       = "(" + QString::fromStdString(dst_tupel.pin_type) + ")";
    //        }
    //        else
    //        {
    //            gate_name = "Returned Nullptr :(";
    //            pin       = "";
    //        }
    //        QTreeWidgetItem* item3 = new QTreeWidgetItem(m_dst_item);
    //        item3->setText(0, gate_name);
    //        //item->setText(1, QChar(0x2b05));
    //        item3->setText(1, pin);
    //    }

    //    m_table_widget->setRowCount(0);
    //    auto net_data = net->get_data();
    //    m_table_widget->setRowCount(net_data.size());
    //    int i = 0;
    //    for (auto value : net_data)
    //    {
    //        m_table_widget->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(std::get<0>(value.first))));
    //        m_table_widget->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(std::get<1>(value.first))));
    //        m_table_widget->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(std::get<0>(value.second))));
    //        m_table_widget->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(std::get<1>(value.second))));
    //        i++;
    //    }

    //    m_tree_widget->adjustSize();
    //    m_tree_height = m_tree_widget->sizeHint().height();
    //    m_tree_widget->setMinimumHeight(m_tree_height);
    //    m_table_height = m_table_widget->verticalHeader()->length();
    //    m_table_widget->setFixedHeight(m_table_height);
    //    m_table_widget->updateGeometry();
    //    m_container->setMinimumHeight(m_tree_height + m_table_height + m_spacing);
    //    m_table_widget->horizontalHeader()->setStretchLastSection(false);
    //    m_table_widget->resizeColumnToContents(3);
    //    m_table_widget->adjustSize();
    //    m_tree_widget->resizeColumnToContents(0);
    //    m_container->setMinimumWidth(m_table_widget->horizontalHeader()->length());
    //    m_table_widget->horizontalHeader()->setStretchLastSection(true);
}

void net_details_widget::handle_tree_size_change(QTreeWidgetItem* item)
{
    Q_UNUSED(item)

    //    m_tree_widget->adjustSize();
    //    m_tree_height = m_tree_widget->sizeHint().height();
    //    m_tree_widget->setMinimumHeight(m_tree_height);
    //    m_container->setMinimumHeight(m_tree_height + m_table_height + m_spacing);
    //    m_tree_widget->updateGeometry();
}

void net_details_widget::handle_item_expanded(QTreeWidgetItem* item)
{
    Q_UNUSED(item)
}

void net_details_widget::handle_item_collapsed(QTreeWidgetItem* item)
{
    Q_UNUSED(item)
}
