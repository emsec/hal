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
#include <QPushButton>
#include <QMenu>
#include <QClipboard>
#include <QApplication>

net_details_widget::net_details_widget(QWidget* parent) : QWidget(parent)
{
    //general initializations
    m_current_id = 0;
    m_key_font = QFont("Iosevka");
    m_key_font.setBold(true);
    m_key_font.setPixelSize(13);

    m_scroll_area = new QScrollArea();
    m_top_lvl_container = new QWidget();
    m_top_lvl_layout = new QVBoxLayout(m_top_lvl_container);
    m_top_lvl_container->setLayout(m_top_lvl_layout);
    m_scroll_area->setWidget(m_top_lvl_container);
    m_scroll_area->setWidgetResizable(true);
    m_content_layout = new QVBoxLayout(this);

    //layout customization
    m_content_layout->setContentsMargins(0,0,0,0);
    m_content_layout->setSpacing(0);
    m_top_lvl_layout->setContentsMargins(0,0,0,0);
    m_top_lvl_layout->setSpacing(0);

    //intermediate layout for the 3 sections (to add left spacing)
    QHBoxLayout *intermediate_layout_gt = new QHBoxLayout();
    intermediate_layout_gt->setContentsMargins(3,3,0,0);
    intermediate_layout_gt->setSpacing(0);
    QHBoxLayout *intermediate_layout_sources = new QHBoxLayout();
    intermediate_layout_sources->setContentsMargins(3,3,0,0);
    intermediate_layout_sources->setSpacing(10);
    QHBoxLayout *intermediate_layout_destinations = new QHBoxLayout();
    intermediate_layout_destinations->setContentsMargins(3,3,0,0);
    intermediate_layout_destinations->setSpacing(0);

    //buttons
    m_general_info_button = new QPushButton("General Information", this);
    m_general_info_button->setEnabled(false);
    m_source_pins_button = new QPushButton("Source Pins", this);
    m_destination_pins_button = new QPushButton("Destination Pins", this);

    //table initializations
    m_general_table = new QTableWidget(3, 2);
    m_source_pins_table = new QTableWidget(0, 3);
    m_destination_pins_table = new QTableWidget(0, 3);

    QList<QTableWidget*> tmp;
    tmp << m_general_table << m_source_pins_table << m_destination_pins_table;
    for(auto &table : tmp)
    {
        table->horizontalHeader()->hide();
        table->verticalHeader()->hide();
        table->verticalHeader()->setDefaultSectionSize(16);
        table->resizeColumnToContents(0);
        table->setShowGrid(false);
        table->setFocusPolicy(Qt::NoFocus);
        table->setFrameStyle(QFrame::NoFrame);
        table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        table->setMaximumHeight(table->verticalHeader()->length());
        table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        //context menu related:
        table->setContextMenuPolicy(Qt::CustomContextMenu);
    }

    QList<QTableWidgetItem*> tmp_general_info_list = {new QTableWidgetItem("Name:"), new QTableWidgetItem("Type:"),
                                                      new QTableWidgetItem("ID:")};
    for(int i = 0; i < tmp_general_info_list.size(); i++)
    {
        auto item = tmp_general_info_list.at(i);
        item->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
        item->setFont(m_key_font);
        m_general_table->setItem(i, 0, item);
    }

    //create dynamic items that change when gate is changed
    m_name_item = new QTableWidgetItem();
    m_name_item->setFlags(Qt::ItemIsEnabled);
    m_general_table->setItem(0, 1, m_name_item);

    m_type_item = new QTableWidgetItem();
    m_type_item->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
    m_general_table->setItem(1, 1, m_type_item);

    m_id_item = new QTableWidgetItem();
    m_id_item->setFlags(Qt::ItemIsEnabled);
    m_general_table->setItem(2, 1, m_id_item);

    //adding things to intermediate layout (the one thats neccessary for the left spacing)
    intermediate_layout_gt->addWidget(m_general_table);
    intermediate_layout_gt->addSpacerItem(new QSpacerItem(0,0, QSizePolicy::Expanding, QSizePolicy::Fixed));
    intermediate_layout_sources->addWidget(m_source_pins_table);
    intermediate_layout_sources->addSpacerItem(new QSpacerItem(0,0, QSizePolicy::Expanding, QSizePolicy::Fixed));
    intermediate_layout_destinations->addWidget(m_destination_pins_table);
    intermediate_layout_destinations->addSpacerItem(new QSpacerItem(0,0, QSizePolicy::Expanding, QSizePolicy::Fixed));

    //adding things to the main layout
    m_top_lvl_layout->addWidget(m_general_info_button);
    m_top_lvl_layout->addLayout(intermediate_layout_gt);
    m_top_lvl_layout->addSpacerItem(new QSpacerItem(0, 7, QSizePolicy::Expanding, QSizePolicy::Fixed));
    m_top_lvl_layout->addWidget(m_source_pins_button);
    m_top_lvl_layout->addLayout(intermediate_layout_sources);
    m_top_lvl_layout->addSpacerItem(new QSpacerItem(0, 7, QSizePolicy::Expanding, QSizePolicy::Fixed));
    m_top_lvl_layout->addWidget(m_destination_pins_button);
    m_top_lvl_layout->addLayout(intermediate_layout_destinations);

    //necessary to add at the end
    m_top_lvl_layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
    m_content_layout->addWidget(m_scroll_area);

    //connect the buttons
    connect(m_source_pins_button, &QPushButton::clicked, this, &net_details_widget::handle_buttons_clicked);
    connect(m_destination_pins_button, &QPushButton::clicked, this, &net_details_widget::handle_buttons_clicked);

    //connect the tables
    connect(m_source_pins_table, &QTableWidget::itemDoubleClicked, this, &net_details_widget::handle_table_item_clicked);
    connect(m_destination_pins_table, &QTableWidget::itemDoubleClicked, this, &net_details_widget::handle_table_item_clicked);
    connect(m_general_table, &QTableWidget::customContextMenuRequested, this, &net_details_widget::handle_general_table_menu_requeted);
    connect(m_source_pins_table, &QTableWidget::customContextMenuRequested, this, &net_details_widget::handle_sources_table_menu_requeted);
    connect(m_destination_pins_table, &QTableWidget::customContextMenuRequested, this, &net_details_widget::handle_destinations_table_menu_requeted);

    //install eventfilter to change the cursor when hovering over the second colums of the pin tables
    m_destination_pins_table->viewport()->setMouseTracking(true);
    m_destination_pins_table->viewport()->installEventFilter(this);
    m_source_pins_table->viewport()->setMouseTracking(true);
    m_source_pins_table->viewport()->installEventFilter(this);

//    m_content_layout = new QVBoxLayout(this);
//    m_content_layout->setContentsMargins(0, 0, 0, 0);
//    m_content_layout->setSpacing(0);
//    m_content_layout->setAlignment(Qt::AlignTop);

//    m_tree_row_layout = new QHBoxLayout();
//    m_content_layout->setContentsMargins(0, 0, 0, 0);
//    m_content_layout->setSpacing(0);

//    m_general_table = new QTableWidget(this);
//    m_general_table->horizontalHeader()->setStretchLastSection(true);
//    m_general_table->horizontalHeader()->hide();
//    m_general_table->verticalHeader()->hide();
//    m_general_table->setColumnCount(2);
//    m_general_table->setRowCount(3);//removed modules item temporary, may even be final

//    //m_general_table->setStyleSheet("QTableWidget {background-color : rgb(31, 34, 35);}");

//    QFont font("Iosevka");
//    font.setBold(true);
//    font.setPixelSize(13);

//    //m_item_deleted_label = new QLabel(this);
//    //m_item_deleted_label->setText("Currently selected item has been removed. Please consider relayouting the Graph.");
//    //m_item_deleted_label->setWordWrap(true);
//    //m_item_deleted_label->setAlignment(Qt::AlignmentFlag::AlignTop);
//    //m_item_deleted_label->setHidden(true);
//    //m_content_layout->addWidget(m_item_deleted_label);

//    QTableWidgetItem* name_item = new QTableWidgetItem("Name:");
//    name_item->setFlags(Qt::ItemIsEnabled);
//    //    name_item->setTextColor(Qt::red);
//    name_item->setFont(font);
//    m_general_table->setItem(0, 0, name_item);

//    QTableWidgetItem* type_item = new QTableWidgetItem("Type:");
//    type_item->setFlags(Qt::ItemIsEnabled);
//    type_item->setFont(font);
//    m_general_table->setItem(1, 0, type_item);

//    QTableWidgetItem* id_item = new QTableWidgetItem("ID  :");
//    id_item->setFlags(Qt::ItemIsEnabled);
//    id_item->setFont(font);
//    m_general_table->setItem(2, 0, id_item);

//    //QTableWidgetItem* module_item = new QTableWidgetItem("modules:");
//    id_item->setFlags(Qt::ItemIsEnabled);
//    id_item->setFont(font);
//    //m_general_table->setItem(3, 0, module_item);

//    m_name_item = new QTableWidgetItem();
//    m_name_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
//    m_general_table->setItem(0, 1, m_name_item);

//    m_type_item = new QTableWidgetItem();
//    m_type_item->setFlags(Qt::ItemIsEnabled);
//    m_general_table->setItem(1, 1, m_type_item);

//    m_id_item = new QTableWidgetItem();
//    m_id_item->setFlags(Qt::ItemIsEnabled);
//    m_general_table->setItem(2, 1, m_id_item);

//    m_module_item = new QTableWidgetItem();
//    m_module_item->setFlags(Qt::ItemIsEnabled);
//    m_general_table->setItem(3, 1, m_module_item);

//    m_general_table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
//    m_general_table->verticalHeader()->setDefaultSectionSize(16);
//    //    m_general_table->resizeRowsToContents();
//    m_general_table->resizeColumnToContents(0);
//    m_general_table->setShowGrid(false);
//    m_general_table->setFocusPolicy(Qt::NoFocus);
//    m_general_table->setFrameStyle(QFrame::NoFrame);
//    m_general_table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
//    m_general_table->setFixedHeight(m_general_table->verticalHeader()->length());
//    m_content_layout->addWidget(m_general_table);

//    m_container_layout = new QVBoxLayout(this);
//    m_container_layout->setContentsMargins(0, 0, 0, 0);
//    m_container_layout->setSpacing(0);
//    m_container_layout->setAlignment(Qt::AlignTop);

//    m_container = new QWidget(this);
//    m_container->setLayout(m_container_layout);
//    m_container->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

//    m_tree_widget = new QTreeWidget(this);
//    m_tree_widget->setFrameStyle(QFrame::NoFrame);
//    m_tree_widget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
//    m_tree_widget->header()->hide();
//    m_tree_widget->setSelectionMode(QAbstractItemView::NoSelection);
//    m_tree_widget->setFocusPolicy(Qt::NoFocus);
//    m_tree_widget->headerItem()->setText(0, "");
//    m_tree_widget->headerItem()->setText(1, "");
//    m_tree_widget->headerItem()->setText(2, "");
//    //m_tree_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
//    m_tree_widget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    m_tree_widget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    m_tree_widget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
//    m_tree_widget->header()->setStretchLastSection(false);
//    m_tree_widget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

//    m_tree_row_layout->addWidget(m_tree_widget);
//    m_tree_row_layout->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Minimum));
//    m_container_layout->addLayout(m_tree_row_layout);

//    connect(m_tree_widget, &QTreeWidget::itemClicked, this, &net_details_widget::on_treewidget_item_clicked);

//    m_scroll_area = new QScrollArea(this);
//    m_scroll_area->setFrameStyle(QFrame::NoFrame);
//    m_scroll_area->setWidgetResizable(true);
//    m_scroll_area->setWidget(m_container);
//    m_content_layout->addWidget(m_scroll_area);

//    m_source_pin = new QTreeWidgetItem(m_tree_widget);
//    m_source_pin->setExpanded(true);

//    m_destination_pins = new QTreeWidgetItem(m_tree_widget);
//    m_destination_pins->setExpanded(true);


//    connect(&g_netlist_relay, &netlist_relay::net_removed, this, &net_details_widget::handle_net_removed);
//    connect(&g_netlist_relay, &netlist_relay::net_name_changed, this, &net_details_widget::handle_net_name_changed);
//    connect(&g_netlist_relay, &netlist_relay::net_source_added, this, &net_details_widget::handle_net_source_added);
//    connect(&g_netlist_relay, &netlist_relay::net_source_removed, this, &net_details_widget::handle_net_source_removed);
//    connect(&g_netlist_relay, &netlist_relay::net_destination_added, this, &net_details_widget::handle_net_destination_added);
//    connect(&g_netlist_relay, &netlist_relay::net_destination_removed, this, &net_details_widget::handle_net_destination_removed);

//    connect(&g_netlist_relay, &netlist_relay::gate_name_changed, this, &net_details_widget::handle_gate_name_changed);
}

net_details_widget::~net_details_widget()
{
    delete m_name_item;
    delete m_type_item;
    delete m_id_item;
    //delete m_source_pin;
    //delete m_destination_pins;
}

bool net_details_widget::eventFilter(QObject *watched, QEvent *event)
{
    //need to determine which of the tables is the "owner" of the viewport
    QTableWidget* table = (watched == m_destination_pins_table->viewport()) ? m_destination_pins_table : m_source_pins_table;
    if(event->type() == QEvent::MouseMove)
    {
        QMouseEvent* ev = dynamic_cast<QMouseEvent*>(event);
        QTableWidgetItem* item = table->itemAt(ev->pos());
        if(item)
        {
            if(item->column() == 2)
                setCursor(QCursor(Qt::PointingHandCursor));
            else
                setCursor(QCursor(Qt::ArrowCursor));
        }
        else
            setCursor(QCursor(Qt::ArrowCursor));
    }

    //restore default cursor when leaving the widget (maybe save cursor before entering?)
    if(event->type() == QEvent::Leave)
        setCursor(QCursor(Qt::ArrowCursor));

    return false;
}

void net_details_widget::update(u32 net_id)
{

    m_current_id = net_id;
    auto n = g_netlist->get_net_by_id(net_id);

    if(m_current_id == 0 || !n)
        return;

    // (1) update general info section
    m_name_item->setText(QString::fromStdString(n->get_name()));
    m_id_item->setText(QString::number(m_current_id));

    //get net type
    QString n_type = "Standard";

    if (g_netlist->is_global_input_net(n))
        n_type = "Input";
    else if (g_netlist->is_global_output_net(n))
        n_type = "Output";

    m_type_item->setText(n_type);
    m_general_table->resizeColumnsToContents();
    m_general_table->setFixedWidth(calculate_table_size(m_general_table).width());

    // (2) update sources section
    m_source_pins_table->clearContents();
    m_source_pins_button->setText("Sources (" + QString::number(n->get_sources().size()) + ")");
    m_source_pins_table->setRowCount(n->get_sources().size());
    m_source_pins_table->setMaximumHeight(m_source_pins_table->verticalHeader()->length());
    int index = 0;
    if(!g_netlist->is_global_input_net(n))
    {
        for(const auto &ep_source : n->get_sources())
        {
            QTableWidgetItem* pin_name = new QTableWidgetItem(QString::fromStdString(ep_source.get_pin()));
            QTableWidgetItem* arrow_item = new QTableWidgetItem(QChar(0x2b05));
            QTableWidgetItem* gate_name_item = new QTableWidgetItem(QString::fromStdString(ep_source.get_gate()->get_name()));
            arrow_item->setForeground(QBrush(QColor(114, 140, 0), Qt::SolidPattern));
            pin_name->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
            arrow_item->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
            //arrow_item->setFlags((Qt::ItemFlag)(~Qt::ItemIsSelectable));
            gate_name_item->setFlags(Qt::ItemIsEnabled);
            gate_name_item->setData(Qt::UserRole, ep_source.get_gate()->get_id());

            m_source_pins_table->setItem(index, 0, pin_name);
            m_source_pins_table->setItem(index, 1, arrow_item);
            m_source_pins_table->setItem(index, 2, gate_name_item);
            index++;
        }
    }

    m_source_pins_table->resizeColumnsToContents();
    m_source_pins_table->setFixedWidth(calculate_table_size(m_source_pins_table).width());

    // (3) update destinations section
    m_destination_pins_table->clearContents();
    m_destination_pins_button->setText("Destinations (" + QString::number(n->get_destinations().size()) + ")");
    m_destination_pins_table->setRowCount(n->get_destinations().size());
    m_destination_pins_table->setMaximumHeight(m_destination_pins_table->verticalHeader()->length());
    index = 0;
    if(!g_netlist->is_global_output_net(n))
    {
        for(const auto &ep_destination : n->get_destinations())
        {
            QTableWidgetItem* pin_name = new QTableWidgetItem(QString::fromStdString(ep_destination.get_pin()));
            QTableWidgetItem* arrow_item = new QTableWidgetItem(QChar(0x27a1));
            QTableWidgetItem* gate_name_item = new QTableWidgetItem(QString::fromStdString(ep_destination.get_gate()->get_name()));
            arrow_item->setForeground(QBrush(QColor(114, 140, 0), Qt::SolidPattern));
            pin_name->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
            arrow_item->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
            //arrow_item->setFlags((Qt::ItemFlag)(~Qt::ItemIsSelectable));
            gate_name_item->setFlags(Qt::ItemIsEnabled);
            gate_name_item->setData(Qt::UserRole, ep_destination.get_gate()->get_id());

            m_destination_pins_table->setItem(index, 0, pin_name);
            m_destination_pins_table->setItem(index, 1, arrow_item);
            m_destination_pins_table->setItem(index, 2, gate_name_item);
            index++;
        }
    }
    m_destination_pins_table->resizeColumnsToContents();
    m_destination_pins_table->setFixedWidth(calculate_table_size(m_destination_pins_table).width());

    //to prevent any updating(render) erros that can occur, manually tell the tables to update
    m_general_table->update();
    m_source_pins_table->update();
    m_destination_pins_table->update();

    //    m_type_item->setText(n_type);
//    m_current_id = net_id;

//    if (m_current_id == 0)
//        return;

//    auto n = g_netlist->get_net_by_id(net_id);

//    if(!n)
//        return;

//    //get name
//    m_name_item->setText(QString::fromStdString(n->get_name()));

    //get net type
//    QString n_type = "Standard";

//    if (g_netlist->is_global_input_net(n))
//        n_type = "Input";
//    else if (g_netlist->is_global_output_net(n))
//        n_type = "Output";

//    m_type_item->setText(n_type);

//    //get id
//    m_id_item->setText(QString::number(n->get_id()));

//    //get modules
//    QString module_text = "";
//    m_module_item->setText(module_text);

//    //update source pins
//    for (auto item : m_source_pin->takeChildren())
//        delete item;

//    m_source_pin->setText(0, "");

//    if (!g_netlist->is_global_input_net(n))
//    {
//        auto src_pins = n->get_sources();

//        QString src_text = QString::number(src_pins.size()) + " Source Pins";

//        if (src_pins.size() == 1)
//            src_text.chop(1);

//        m_source_pin->setText(0, src_text);

//        for (auto src_pin : src_pins)
//        {
//            auto src_pin_type = src_pin.get_pin();

//            QTreeWidgetItem* item = new QTreeWidgetItem(m_source_pin);
//            item->setText(0, QString::fromStdString(src_pin_type));
//            item->setText(1, QChar(0x2b05));
//            item->setForeground(1, QBrush(QColor(114, 140, 0), Qt::SolidPattern));
//            item->setText(2, QString::fromStdString(src_pin.get_gate()->get_name()));
//            item->setData(2, Qt::UserRole, src_pin.get_gate()->get_id());
//        }
//    }

//    //update destination pins
//    for (auto item : m_destination_pins->takeChildren())
//        delete item;

//    m_destination_pins->setText(0, "");

//    if (!g_netlist->is_global_output_net(n))
//    {
//        auto dsts_pins = n->get_destinations();

//        QString dst_text = QString::number(dsts_pins.size()) + " Destination Pins";

//        if (dsts_pins.size() == 1)
//            dst_text.chop(1);

//        m_destination_pins->setText(0, dst_text);

//        for (auto dst_pin : dsts_pins)
//        {
//            auto dst_pin_type = dst_pin.get_pin();

//            QTreeWidgetItem* item = new QTreeWidgetItem(m_destination_pins);
//            item->setText(0, QString::fromStdString(dst_pin_type));
//            item->setText(1, QChar(0x27a1));
//            item->setForeground(1, QBrush(QColor(114, 140, 0), Qt::SolidPattern));
//            item->setText(2, QString::fromStdString(dst_pin.get_gate()->get_name()));
//            item->setData(2, Qt::UserRole, dst_pin.get_gate()->get_id());
//        }
//    }

//    m_general_table->setHidden(false);
//    m_scroll_area->setHidden(false);
    //m_item_deleted_label->setHidden(true);

//    m_tree_widget->resizeColumnToContents(0);
//    m_tree_widget->resizeColumnToContents(1);
//    m_tree_widget->resizeColumnToContents(2);

}

void net_details_widget::handle_item_expanded(QTreeWidgetItem* item)
{
    Q_UNUSED(item)
}

void net_details_widget::handle_item_collapsed(QTreeWidgetItem* item)
{
    Q_UNUSED(item)
}

void net_details_widget::on_treewidget_item_clicked(QTreeWidgetItem* item, int column)
{
//    auto gate_id = item->data(2, Qt::UserRole).toInt();
//    auto pin     = item->text(0).toStdString();
//    if (m_destination_pins == item->parent() && column == 2)
//    {
//        std::shared_ptr<gate> clicked_gate = g_netlist->get_gate_by_id(gate_id);

//        if (!clicked_gate)
//            return;

//        g_selection_relay.clear();
//        g_selection_relay.m_selected_gates.insert(clicked_gate->get_id());
//        g_selection_relay.m_focus_type = selection_relay::item_type::gate;
//        g_selection_relay.m_focus_id   = clicked_gate->get_id();
//        g_selection_relay.m_subfocus   = selection_relay::subfocus::left;

//        auto pins                          = clicked_gate->get_input_pins();
//        auto index                         = std::distance(pins.begin(), std::find(pins.begin(), pins.end(), pin));
//        g_selection_relay.m_subfocus_index = index;

//        g_selection_relay.relay_selection_changed(this);
//    }
//    else if (m_source_pin == item->parent() && column == 2)
//    {
//        std::shared_ptr<gate> clicked_gate = g_netlist->get_gate_by_id(gate_id);

//        if (!clicked_gate)
//            return;

//        g_selection_relay.clear();
//        g_selection_relay.m_selected_gates.insert(clicked_gate->get_id());
//        g_selection_relay.m_focus_type = selection_relay::item_type::gate;
//        g_selection_relay.m_focus_id   = clicked_gate->get_id();
//        g_selection_relay.m_subfocus   = selection_relay::subfocus::right;

//        auto pins                          = clicked_gate->get_output_pins();
//        auto index                         = std::distance(pins.begin(), std::find(pins.begin(), pins.end(), pin));
//        g_selection_relay.m_subfocus_index = index;

//        g_selection_relay.relay_selection_changed(this);
//    }
}

void net_details_widget::handle_net_removed(const std::shared_ptr<net> n)
{
//    if(m_current_id == n->get_id())
//    {
//        m_general_table->setHidden(true);
//        m_scroll_area->setHidden(true);
//    }
}

void net_details_widget::handle_net_name_changed(const std::shared_ptr<net> n)
{
    if(m_current_id == n->get_id())
        m_name_item->setText(QString::fromStdString(n->get_name()));

}

void net_details_widget::handle_net_source_added(const std::shared_ptr<net> n, const u32 src_gate_id)
{
    Q_UNUSED(src_gate_id);

    if(m_current_id == n->get_id())
        update(m_current_id);
}

void net_details_widget::handle_net_source_removed(const std::shared_ptr<net> n, const u32 src_gate_id)
{
    Q_UNUSED(src_gate_id);

    if(m_current_id == n->get_id())
        update(m_current_id);
}

void net_details_widget::handle_net_destination_added(const std::shared_ptr<net> n, const u32 dst_gate_id)
{
    Q_UNUSED(dst_gate_id);

    if(m_current_id == n->get_id())
        update(m_current_id);
}

void net_details_widget::handle_net_destination_removed(const std::shared_ptr<net> n, const u32 dst_gate_id)
{
    Q_UNUSED(dst_gate_id);

    if(m_current_id == n->get_id())
        update(m_current_id);
}

void net_details_widget::handle_gate_name_changed(const std::shared_ptr<gate> g)
{
    Q_UNUSED(g)

    if (m_current_id == 0)
        return;

    bool update_needed = false;

    //current net
    auto n = g_netlist->get_net_by_id(m_current_id);

    //check if current net is in netlist (m_current_id is unassigned if netlist details widget hasn't been shown once)
    if(!g_netlist->is_net_in_netlist(n))
        return;

    //check if renamed gate is a src of the currently shown net
    for(auto e : n->get_sources())
    {
        if(e.get_gate()->get_id() == m_current_id)
        {
            update_needed = true;
            break;
        }
    }

    //check if renamed gate is a dst of the currently shown net
    if(!update_needed)
    {
        for(auto e : n->get_destinations())
        {
            if(e.get_gate()->get_id() == m_current_id)
            {
                update_needed = true;
                break;
            }
        }
    }

    if(update_needed)
        update(m_current_id);
}

void net_details_widget::handle_buttons_clicked()
{
    QPushButton* btn = dynamic_cast<QPushButton*>(sender());
    if(!btn)
        return;

    int index = m_top_lvl_layout->indexOf(btn);
    QWidget* widget;
    widget = m_top_lvl_layout->itemAt(index+1)->layout()->itemAt(0)->widget();

    if(!widget)
        return;

    if(widget->isHidden()){
        widget->show();
     }
    else{
        widget->hide();
     }

}

void net_details_widget::handle_table_item_clicked(QTableWidgetItem* item)
{
    if(item->column() != 2)
        return;

    QTableWidget* sender_table = dynamic_cast<QTableWidget*>(sender());

    selection_relay::subfocus focus = (sender_table == m_source_pins_table) ? selection_relay::subfocus::right : selection_relay::subfocus::left;
    auto gate_id = item->data(Qt::UserRole).toInt();
    auto pin = sender_table->item(item->row(), 0)->text().toStdString();

    auto clicked_gate = g_netlist->get_gate_by_id(gate_id);
    if(!clicked_gate)
        return;

    g_selection_relay.clear();
    g_selection_relay.m_selected_gates.insert(gate_id);
    g_selection_relay.m_focus_type = selection_relay::item_type::gate;
    g_selection_relay.m_focus_id = gate_id;
    g_selection_relay.m_subfocus = focus;

    auto pins = (sender_table == m_source_pins_table) ? clicked_gate->get_output_pins() : clicked_gate->get_input_pins();
    auto index = std::distance(pins.begin(), std::find(pins.begin(), pins.end(), pin));
    g_selection_relay.m_subfocus_index = index;

    g_selection_relay.relay_selection_changed(this);
}

void net_details_widget::handle_general_table_menu_requeted(const QPoint &pos)
{
    if(m_general_table->itemAt(pos)->column() != 1)
        return;

    QMenu menu;
    QString description;
    QString python_command = "netlist.get_net_by_id(" + QString::number(m_current_id) + ").";
    switch(m_general_table->itemAt(pos)->row())
    {
        case 0: python_command += "get_name()"; description = "Extract name as python code (copy to clipboard)"; break;
        case 1: break; //there is no "explicit" type.... need to check with functions
        case 2: python_command += "get_id()"; description = "Extract id as python code (copy to clipboard)"; break;
    }

    if(m_general_table->itemAt(pos)->row() == 1 || m_general_table->itemAt(pos)->row() == 3)
        return;

    menu.addAction(QIcon(":/icons/python"), description, [python_command](){
        QApplication::clipboard()->setText(python_command);
    });

    menu.move(dynamic_cast<QWidget*>(sender())->mapToGlobal(pos));
    menu.exec();
}

void net_details_widget::handle_sources_table_menu_requeted(const QPoint &pos)
{
    if(m_source_pins_table->itemAt(pos)->column() != 2)
        return;

    QMenu menu;
    menu.addAction("Jump to source gate", [this, pos](){
        handle_table_item_clicked(m_source_pins_table->itemAt(pos));
    });
    menu.addAction(QIcon(":/icons/python"),"Extract gate as python code (copy to clipboard)", [this, pos](){
        QApplication::clipboard()->setText("netlist.get_gate_by_id(" + m_source_pins_table->itemAt(pos)->data(Qt::UserRole).toString() + ")");
    });

    menu.move(dynamic_cast<QWidget*>(sender())->mapToGlobal(pos));
    menu.exec();
}

void net_details_widget::handle_destinations_table_menu_requeted(const QPoint &pos)
{
    if(m_destination_pins_table->itemAt(pos)->column() != 2)
        return;

    QMenu menu;
    menu.addAction("Jump to destination gate", [this, pos](){
        handle_table_item_clicked(m_destination_pins_table->itemAt(pos));
    });
    menu.addAction(QIcon(":/icons/python"), "Extract gate as python code (copy to clipboard)", [this, pos](){
        QApplication::clipboard()->setText("netlist.get_gate_by_id(" + m_destination_pins_table->itemAt(pos)->data(Qt::UserRole).toString() + ")");
    });

    menu.move(dynamic_cast<QWidget*>(sender())->mapToGlobal(pos));
    menu.exec();
}

QSize net_details_widget::calculate_table_size(QTableWidget *table)
{
    //necessary to test if the table is empty, otherwise (due to the resizeColumnsToContents function)
    //is the tables width far too big, so just return 0 as the size
    if(!table->rowCount())
        return QSize(0,0);

    int w = table->verticalHeader()->width() + 4; // +4 seems to be needed
    for (int i = 0; i < table->columnCount(); i++)
       w += table->columnWidth(i); // seems to include gridline
    int h = table->horizontalHeader()->height() + 4;
    for (int i = 0; i < table->rowCount(); i++)
       h += table->rowHeight(i);
    return QSize(w+5, h);

}
