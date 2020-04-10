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
#include <QGridLayout>
#include <QPushButton>
#include <QPropertyAnimation>

gate_details_widget::gate_details_widget(QWidget* parent) : QWidget(parent)
{
    //NEW
    // general initializations
    m_current_id = 0;
    m_last_click_time = 0;
    m_key_font = QFont("Iosevka");
    m_key_font.setBold(true);
    m_key_font.setPixelSize(13);

    m_scroll_area = new QScrollArea();
    m_top_lvl_container = new QWidget();
    m_top_lvl_layout = new QVBoxLayout(m_top_lvl_container);
    m_top_lvl_container->setLayout(m_top_lvl_layout);
    m_content_layout = new QVBoxLayout(this);
    m_scroll_area->setWidget(m_top_lvl_container);
    m_scroll_area->setWidgetResizable(true);

    //layout customization
    m_content_layout->setContentsMargins(0,0,0,0);
    m_content_layout->setSpacing(0);
    m_top_lvl_layout->setContentsMargins(0,0,0,0);
    m_top_lvl_layout->setSpacing(0);

    // buttons
    m_general_info_button = new QPushButton("General Information");
    m_general_info_button->setEnabled(false);
    m_input_pins_button = new QPushButton("Input Pins");
    m_output_pins_button = new QPushButton("Output Pins");
    m_data_fields_button = new QPushButton("Data Fields");
    m_boolean_functions_button = new QPushButton("Boolean Functions");

/*    m_general_info_button->setFixedHeight(20);
    QFont tst;
    tst.setPixelSize(10);
    tst.setBold(true);
    m_general_info_button->setFont(tst)*/;

    //(1) general info section
    m_general_table = new QTableWidget(4,2);

    //create static items and iterate over them (minimize code, compiler most likely unrolls the loop)
//    QFont font("Iosevka");//through stylesheets?
//    font.setBold(true);
//    font.setPixelSize(13);

    QList<QTableWidgetItem*> tmp_general_info_list = {new QTableWidgetItem("Name:"), new QTableWidgetItem("Type:"),
                                                      new QTableWidgetItem("ID:"), new QTableWidgetItem("Module:")};
    for(int i = 0; i < tmp_general_info_list.size(); i++)
    {
        auto item = tmp_general_info_list.at(i);
        item->setFlags(Qt::ItemIsEnabled);
        item->setFont(m_key_font);
        m_general_table->setItem(i, 0, item);
    }

//    //create dynamic items that change when gate is changed
    m_name_item = new QTableWidgetItem();
    m_name_item->setFlags(Qt::ItemIsEnabled);
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

    //general table customizations
    m_general_table->horizontalHeader()->setStretchLastSection(true);
    m_general_table->horizontalHeader()->hide();
    m_general_table->verticalHeader()->hide();
//    m_general_table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_general_table->verticalHeader()->setDefaultSectionSize(16);
    m_general_table->resizeColumnToContents(0);
    m_general_table->setShowGrid(false);
    m_general_table->setFocusPolicy(Qt::NoFocus);
    m_general_table->setFrameStyle(QFrame::NoFrame);
    m_general_table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_general_table->setMaximumHeight(m_general_table->verticalHeader()->length());
    m_general_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_general_table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //(2) Input Pin section
    m_input_pins_table = new QTableWidget(1,3);

    // input pin customization
    m_input_pins_table->horizontalHeader()->setStretchLastSection(true);
    m_input_pins_table->horizontalHeader()->hide();
    m_input_pins_table->verticalHeader()->hide();
    m_input_pins_table->verticalHeader()->setDefaultSectionSize(16);
    m_input_pins_table->resizeColumnToContents(0);
    m_input_pins_table->setShowGrid(false);
    m_input_pins_table->setFocusPolicy(Qt::NoFocus);
    m_input_pins_table->setFrameStyle(QFrame::NoFrame);
    m_input_pins_table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_input_pins_table->setMaximumHeight(m_input_pins_table->verticalHeader()->length());
    m_input_pins_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_input_pins_table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //(3) Output Pin section
    m_output_pins_table = new QTableWidget(0,3);

    //output pins customization
    m_output_pins_table->horizontalHeader()->setStretchLastSection(true);
    m_output_pins_table->horizontalHeader()->hide();
    m_output_pins_table->verticalHeader()->hide();
    m_output_pins_table->verticalHeader()->setDefaultSectionSize(16);
    m_output_pins_table->resizeColumnToContents(0);
    m_output_pins_table->setShowGrid(false);
    m_output_pins_table->setFocusPolicy(Qt::NoFocus);
    m_output_pins_table->setFrameStyle(QFrame::NoFrame);
    m_output_pins_table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_output_pins_table->setMaximumHeight(m_output_pins_table->verticalHeader()->length());
    m_output_pins_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_output_pins_table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);


    //(4) Data Fields section
    m_data_fields_table = new QTableWidget(0, 2);
    m_data_fields_table->horizontalHeader()->setStretchLastSection(true);
    m_data_fields_table->horizontalHeader()->hide();
    m_data_fields_table->verticalHeader()->hide();
    m_data_fields_table->verticalHeader()->setDefaultSectionSize(16);
    m_data_fields_table->resizeColumnToContents(0);
    m_data_fields_table->setShowGrid(false);
    m_data_fields_table->setFocusPolicy(Qt::NoFocus);
    m_data_fields_table->setFrameStyle(QFrame::NoFrame);
    m_data_fields_table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_data_fields_table->setMaximumHeight(m_data_fields_table->verticalHeader()->length());
    m_data_fields_table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_data_fields_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //(5) Boolean Function section
    m_boolean_functions_container = new QWidget();
    m_boolean_functions_container_layout = new QVBoxLayout();
    m_boolean_functions_container->setLayout(m_boolean_functions_container_layout);

    //adding things to the layout
    m_top_lvl_layout->addWidget(m_general_info_button);
    m_top_lvl_layout->addWidget(m_general_table);
    m_top_lvl_layout->addSpacerItem(new QSpacerItem(0, 5, QSizePolicy::Expanding, QSizePolicy::Fixed));
    m_top_lvl_layout->addWidget(m_input_pins_button);
    m_top_lvl_layout->addWidget(m_input_pins_table);
    m_top_lvl_layout->addSpacerItem(new QSpacerItem(0, 5, QSizePolicy::Expanding, QSizePolicy::Fixed));
    m_top_lvl_layout->addWidget(m_output_pins_button);
    m_top_lvl_layout->addWidget(m_output_pins_table);
    m_top_lvl_layout->addSpacerItem(new QSpacerItem(0, 5, QSizePolicy::Expanding, QSizePolicy::Fixed));
    m_top_lvl_layout->addWidget(m_data_fields_button);
    m_top_lvl_layout->addWidget(m_data_fields_table);
    m_top_lvl_layout->addSpacerItem(new QSpacerItem(0, 5, QSizePolicy::Expanding, QSizePolicy::Fixed));
    m_top_lvl_layout->addWidget(m_boolean_functions_button);
    m_top_lvl_layout->addWidget(m_boolean_functions_container);

    //necessary to add at the end
    m_top_lvl_layout->addSpacerItem(new QSpacerItem(0,0, QSizePolicy::Expanding, QSizePolicy::Expanding));
    m_content_layout->addWidget(m_scroll_area);

    //setup the navigation_table ("activated" by clicking on an input / output pin in the 2 tables)
    m_navigation_table = new graph_navigation_widget();//set the parent?
    m_navigation_table->setWindowFlags(Qt::CustomizeWindowHint);
    m_navigation_table->hide_when_focus_lost(true);
    m_navigation_table->hide();
    connect(m_navigation_table, &graph_navigation_widget::navigation_requested, this, &gate_details_widget::handle_navigation_jump_requested);

    //some connections (maybe connect to simple toggle_hide_show function of widgets)
    connect(m_general_info_button, &QPushButton::clicked, this, &gate_details_widget::handle_buttons_clicked);
    connect(m_input_pins_button, &QPushButton::clicked, this, &gate_details_widget::handle_buttons_clicked);
    connect(m_output_pins_button, &QPushButton::clicked, this, &gate_details_widget::handle_buttons_clicked);
    connect(m_data_fields_button, &QPushButton::clicked, this, &gate_details_widget::handle_buttons_clicked);
    connect(m_boolean_functions_button, &QPushButton::clicked, this, &gate_details_widget::handle_buttons_clicked);

    connect(m_input_pins_table, &QTableWidget::itemClicked, this, &gate_details_widget::handle_input_pin_item_clicked);
    connect(m_output_pins_table, &QTableWidget::itemClicked, this, &gate_details_widget::handle_output_pin_item_clicked);


    //OLD
//    m_current_id = 0;

//    m_last_click_time = 0;

//    //The most top-level layout for the gate-details-widget
//    m_content_layout = new QVBoxLayout(this);
//    m_content_layout->setContentsMargins(0, 0, 0, 0);
//    m_content_layout->setSpacing(0);

//    //layout for the treewidget (to add additional spacing), is added to the container_layout (and ultimately to the scrollarea)
//    m_tree_row_layout = new QHBoxLayout();

//    // General table at the top with name, id, etc...
//    m_general_table = new QTableWidget(this);
//    m_general_table->horizontalHeader()->setStretchLastSection(true);
//    m_general_table->horizontalHeader()->hide();
//    m_general_table->verticalHeader()->hide();
//    m_general_table->setColumnCount(2);
//    m_general_table->setRowCount(4);

//    QFont font("Iosevka");
//    font.setBold(true);
//    font.setPixelSize(13);

//    // Start of fixed table items
//    QTableWidgetItem* name_item = new QTableWidgetItem("Name:");
//    name_item->setFlags(Qt::ItemIsEnabled);
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

//    QTableWidgetItem* module_item = new QTableWidgetItem("Module: ");
//    module_item->setFlags(Qt::ItemIsEnabled);
//    module_item->setFont(font);
//    m_general_table->setItem(3, 0, module_item);

//    // Start of dynamic table items
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

//    m_boolean_function = new QLabel(this);
//    m_boolean_function->setWordWrap(true);

//    m_data_fields = new QLabel(this);
//    m_data_fields->setWordWrap(true);

//    m_container = new QWidget(this);
//    m_container->setLayout(m_container_layout);
//    m_container->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

//    // TreeWidget for input/output pins
//    m_tree_widget = new QTreeWidget(this);
//    m_tree_widget->setFrameStyle(QFrame::NoFrame);
//    m_tree_widget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
//    m_tree_widget->header()->hide();
//    m_tree_widget->setSelectionMode(QAbstractItemView::NoSelection);
//    m_tree_widget->setFocusPolicy(Qt::NoFocus);
//    m_tree_widget->headerItem()->setText(0, "");
//    m_tree_widget->headerItem()->setText(1, "");
//    m_tree_widget->headerItem()->setText(2, "");
//    m_tree_widget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    m_tree_widget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

//    m_tree_widget->header()->setStretchLastSection(false);
//    //by setting this you do not need to call resizecolumntocontents every time after updating
//    m_tree_widget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
//    m_tree_widget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

//    m_tree_row_layout->addWidget(m_tree_widget);
//    m_tree_row_layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum));
//    m_tree_row_layout->addSpacing(0);

//    //add items to the container layout that is later added to the content layout(ultimately the scrollarea)
//    m_container_layout->addLayout(m_tree_row_layout);
//    m_container_layout->addWidget(m_data_fields);
//    //Add an additional layout to the container-layout to manage the boolean functions (//TEST nested-layouts and word wrapping)
//    m_boolean_functions_layout = new QVBoxLayout(this);
//    m_boolean_functions_layout->setSpacing(5);
//    //m_boolean_function->setText("<b>Boolean Function(s):</b");
//    m_container_layout->addWidget(m_boolean_function);
////    QWidget* spacer = new QWidget(this);
////    spacer->setMinimumHeight(10);
////    spacer->setMaximumHeight(10);
////    m_container_layout->addWidget(spacer);
//    m_container_layout->addLayout(m_boolean_functions_layout);
//    //m_container_layout->addWidget(m_boolean_function);

//    connect(m_tree_widget, &QTreeWidget::itemClicked, this, &gate_details_widget::on_treewidget_item_clicked);

//    //set parent and flag so it gets shown in its own window
//    m_navigation_table = new graph_navigation_widget();
//    m_navigation_table->setWindowFlags(Qt::CustomizeWindowHint);
//    m_navigation_table->hide_when_focus_lost(true);
//    m_navigation_table->hide();
//    connect(m_navigation_table, &graph_navigation_widget::navigation_requested, this, &gate_details_widget::handle_navigation_jump_requested);

//    m_scroll_area = new QScrollArea(this);
//    m_scroll_area->setFrameStyle(QFrame::NoFrame);
//    m_scroll_area->setWidgetResizable(true);
//    m_scroll_area->setWidget(m_container);
//    m_content_layout->addWidget(m_scroll_area);

//    m_input_pins = new QTreeWidgetItem(m_tree_widget);
//    m_input_pins->setExpanded(true);

//    m_output_pins = new QTreeWidgetItem(m_tree_widget);
//    m_output_pins->setExpanded(true);

//    connect(m_general_table, &QTableWidget::doubleClicked, this, &gate_details_widget::on_general_table_item_double_clicked);

//    //handle netlist modifications regarding gates
//    connect(&g_netlist_relay, &netlist_relay::gate_name_changed, this, &gate_details_widget::handle_gate_name_changed);
//    connect(&g_netlist_relay, &netlist_relay::gate_removed, this, &gate_details_widget::handle_gate_removed);

//    //handle netlist modifications regarding modules
//    connect(&g_netlist_relay, &netlist_relay::module_name_changed, this, &gate_details_widget::handle_module_name_changed);
//    connect(&g_netlist_relay, &netlist_relay::module_removed, this, &gate_details_widget::handle_module_removed);
//    connect(&g_netlist_relay, &netlist_relay::module_gate_assigned, this, &gate_details_widget::handle_module_gate_assigned);
//    connect(&g_netlist_relay, &netlist_relay::module_gate_removed, this, &gate_details_widget::handle_module_gate_removed);

//    //handle netlist modifications reagarding nets
//    connect(&g_netlist_relay, &netlist_relay::net_name_changed, this, &gate_details_widget::handle_net_name_changed);
//    connect(&g_netlist_relay, &netlist_relay::net_source_added, this, &gate_details_widget::handle_net_source_added);
//    connect(&g_netlist_relay, &netlist_relay::net_source_removed, this, &gate_details_widget::handle_net_source_removed);
//    connect(&g_netlist_relay, &netlist_relay::net_destination_added, this, &gate_details_widget::handle_net_destination_added);
//    connect(&g_netlist_relay, &netlist_relay::net_destination_removed, this, &gate_details_widget::handle_net_destination_removed);
}

gate_details_widget::~gate_details_widget()
{
//    delete m_name_item;
//    delete m_id_item;
//    delete m_type_item;
//    delete m_module_item;
//    delete m_input_pins;
//    delete m_output_pins;
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

    //check if currently shown gate is a src of renamed net
    for (auto& e : net->get_sources())
    {
        if (m_current_id == e.get_gate()->get_id())
        {
            update_needed = true;
            break;
        }
    }

    //check if currently shown gate is a dst of renamed net
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

void gate_details_widget::handle_net_source_added(std::shared_ptr<net> net, const u32 src_gate_id)
{
    Q_UNUSED(net);
    if (m_current_id == src_gate_id)
        update(m_current_id);
}

void gate_details_widget::handle_net_source_removed(std::shared_ptr<net> net, const u32 src_gate_id)
{
    Q_UNUSED(net);
    if (m_current_id == src_gate_id)
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

void gate_details_widget::handle_buttons_clicked()
{
    //function that (perhaps) is changed by a toggle-slot of the widget
    QPushButton* btn = dynamic_cast<QPushButton*>(sender());
    if(!btn)
        return;

    int index = m_top_lvl_layout->indexOf(btn);
    QWidget* widget = dynamic_cast<QWidget*>(m_top_lvl_layout->itemAt(index+1)->widget());
    if(!widget)
        return;
    if(widget->isHidden()){
        widget->show();

        if(index != m_top_lvl_layout->indexOf(m_boolean_functions_button)){
            m_top_lvl_layout->itemAt(index+2)->spacerItem()->changeSize(0,10, QSizePolicy::Expanding, QSizePolicy::Fixed);
            m_top_lvl_layout->invalidate();
        }
//        QPropertyAnimation* anim = new QPropertyAnimation(widget, "maximumHeight");
//        anim->setDuration(250);
//        anim->setStartValue(0);
//        anim->setEndValue(widget->verticalHeader()->length());
//        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
    else{
        widget->hide();

        if(index != m_top_lvl_layout->indexOf(m_boolean_functions_button)){
            m_top_lvl_layout->itemAt(index+2)->spacerItem()->changeSize(0,5, QSizePolicy::Expanding, QSizePolicy::Fixed);
            m_top_lvl_layout->invalidate();
        }
//        QPropertyAnimation* anim = new QPropertyAnimation(widget, "maximumHeight");
//        anim->setDuration(250);
//        anim->setStartValue(widget->verticalHeader()->length());
//        anim->setEndValue(0);
//        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void gate_details_widget::handle_input_pin_item_clicked(QTableWidgetItem *item)
{
    if(item->column() != 2)
        return;

    int net_id = item->data(Qt::UserRole).toInt();

    auto clicked_net = g_netlist->get_net_by_id(net_id);

    if(!clicked_net)
        return;

    auto sources = clicked_net->get_sources();

    if(sources.empty() || clicked_net->is_global_input_net())
    {
        g_selection_relay.clear();
        g_selection_relay.m_selected_nets.insert(net_id);
        g_selection_relay.relay_selection_changed(this);
    }
    else if(sources.size() == 1)
    {
        auto ep = *sources.begin();
        g_selection_relay.clear();
        g_selection_relay.m_selected_gates.insert(ep.get_gate()->get_id());
        g_selection_relay.m_focus_type = selection_relay::item_type::gate;
        g_selection_relay.m_focus_id   = ep.get_gate()->get_id();
        g_selection_relay.m_subfocus   = selection_relay::subfocus::right;

        auto pins                          = ep.get_gate()->get_output_pins();
        auto index                         = std::distance(pins.begin(), std::find(pins.begin(), pins.end(), ep.get_pin()));
        g_selection_relay.m_subfocus_index = index;

        update(ep.get_gate()->get_id());
        g_selection_relay.relay_selection_changed(this);
    }
    else
    {
        m_navigation_table->setup(hal::node{hal::node_type::gate, 0}, clicked_net, false);
        m_navigation_table->move(QCursor::pos());
        m_navigation_table->show();
        m_navigation_table->setFocus();
    }
}

void gate_details_widget::handle_output_pin_item_clicked(QTableWidgetItem *item)
{
    if(item->column() != 2)
        return;

    int net_id = item->data(Qt::UserRole).toInt();
    std::shared_ptr<net> clicked_net = g_netlist->get_net_by_id(net_id);

    if(!clicked_net)
        return;

    auto destinations = clicked_net->get_destinations();
    if(destinations.empty() || clicked_net->is_global_output_net())
    {
        g_selection_relay.clear();
        g_selection_relay.m_selected_nets.insert(net_id);
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
        m_navigation_table->setup(hal::node{hal::node_type::gate, 0}, clicked_net, true);
        m_navigation_table->move(QCursor::pos());
        m_navigation_table->show();
        m_navigation_table->setFocus();
    }

}

QSize gate_details_widget::calculate_table_size(QTableWidget *table)
{
    int w = table->verticalHeader()->width() + 4; // +4 seems to be needed
    for (int i = 0; i < table->columnCount(); i++)
       w += table->columnWidth(i); // seems to include gridline (on my machine)
    int h = table->horizontalHeader()->height() + 4;
    for (int i = 0; i < table->rowCount(); i++)
       h += table->rowHeight(i);
    return QSize(w, h);

}

void gate_details_widget::update_boolean_function()
{
    auto g = g_netlist->get_gate_by_id(m_current_id);

//    m_boolean_function->setText("");

//    std::string description = "";
//    for (const auto& it : g->get_boolean_functions())
//        description += " <b>Boolean Function</b>: " + it.first + " = " + it.second.to_string() + "<br>";

//    if (description.empty())
//    {
//        g->set_data("gui", "boolean_function", "string", "-");
//    }
//    else
//    {
//        description = description.substr(0, description.size() - 4);
//        m_boolean_function->setText(QString::fromStdString(description));
//    }

//    if (m_boolean_function->text().isEmpty())
//        m_boolean_function->hide();
//    else
//        m_boolean_function->show();

    //delete items in layout:
//    while(m_boolean_functions_layout->itemAt(0) != 0)
//    {
//        QLayoutItem* i = m_boolean_functions_layout->takeAt(0);
//        delete i->widget();
//        delete i;
//    }

//    if(g->get_boolean_functions().size() == 0)
//        m_boolean_function->setText("<b>Boolean Function(s): -<b>");
//    else
//        m_boolean_function->setText("<b>Boolean Function(s):" + QString::number(g->get_boolean_functions().size()) + "</b");

//    int index = 0;
//    for(const auto& it : g->get_boolean_functions())
//    {
//        QPushButton* button = new QPushButton("Hide/Show");
//        button->setMaximumWidth(100);
//        button->setMinimumHeight(22);
//        button->setMaximumHeight(22);
//        toggleable_label* fnct = new toggleable_label(QString::fromStdString(it.first) + " = " + QString::fromStdString(it.second.to_string()));
//        fnct->setWordWrap(true);
//        connect(button, &QPushButton::clicked, fnct, &toggleable_label::toggle_visible);
//        QWidget* spacer = new QWidget();
//        spacer->setMinimumHeight(10);
//        spacer->setMaximumHeight(10);
//        m_boolean_functions_layout->addWidget(button);
//        m_boolean_functions_layout->addWidget(fnct);
//        m_boolean_functions_layout->addWidget(spacer);
//        index+=3;
//    }

//    QPushButton* button = new QPushButton("HIDE/SHOW");
//    button->setMaximumWidth(100);
//    button->setMinimumHeight(22);
//    button->setMaximumHeight(22);
//    //button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
//    QLabel* bruh = new QLabel("HUHU");
//    toggleable_label* span = new toggleable_label("SPAAAAAN AAAAAAa AAAAAAAA AAAAA bbbbb bbbb bbbbbbb bbbbbbb bbb");
//    span->setWordWrap(true);
//    connect(button, &QPushButton::clicked, span, &toggleable_label::toggle_visible);
//    m_boolean_functions_layout->addWidget(button, 0,0);
//    m_boolean_functions_layout->addWidget(bruh, 0, 1);
//    m_boolean_functions_layout->addWidget(span, 1, 0, 1, 2);
//    QWidget* spacer = new QWidget();
//    spacer->setMinimumHeight(10);
//    spacer->setMaximumHeight(10);
//    m_boolean_functions_layout->addWidget(spacer, 2, 0, 1, 2);

//    QPushButton* button2 = new QPushButton("HIDE/SHOW");
//    button->setMaximumWidth(100);
//    //button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
//    QLabel* bruh2 = new QLabel("HUHU");
//    QLabel* span2 = new QLabel("SPAAAAAN AAAAAAa AAAAAAAA AAAAA bbbbb bbbb bbbbbbb bbbbbbb bbb");
//    span2->setWordWrap(true);
//    m_boolean_functions_layout->addWidget(button2, 3,0);
//    m_boolean_functions_layout->addWidget(bruh2, 3, 1);
//    m_boolean_functions_layout->addWidget(span2, 4, 0, 1, 2);
//    m_boolean_functions_layout->setColumnStretch(0, m_boolean_functions_layout->columnMinimumWidth(0));

      // ###### Version with TreeWidget
//    QTreeWidget* tmp = new QTreeWidget(this);
//    tmp->setFrameStyle(QFrame::NoFrame);
//    tmp->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
//    tmp->header()->hide();
//    tmp->setSelectionMode(QAbstractItemView::NoSelection);
//    tmp->setFocusPolicy(Qt::NoFocus);
//    tmp->headerItem()->setText(0, "");
//    tmp->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    tmp->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

//    QTreeWidgetItem* item = new QTreeWidgetItem(tmp);
//    item->setText(0, "HAHAHASSAEDAS");
//    QTreeWidgetItem* child = new QTreeWidgetItem(item);
//    QLabel* txt = new QLabel("asdas fdsfsdfsd asdasd gdfgfdsf asdas gfgsfg asdad gdfsdf dfgdfgfsdg asdasd sdgfsdfsdf fssd");
//    txt->setWordWrap(true);
//    tmp->setItemWidget(child, 0, txt);

//    m_boolean_functions_layout->addWidget(tmp);

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
    //m_container->setFixedWidth(event->size().width());
    m_boolean_functions_container->setFixedWidth(event->size().width());
}

void gate_details_widget::update2(const u32 gate_id)
{
//    m_current_id = gate_id;

//    if (m_current_id == 0)
//        return;

//    auto g = g_netlist->get_gate_by_id(gate_id);

//    if (!g)
//        return;

//    m_name_item->setText(QString::fromStdString(g->get_name()));
//    m_type_item->setText(QString::fromStdString(g->get_type()->get_name()));
//    m_id_item->setText(QString::number(g->get_id()));

//    //get modules
//    QString module_text = "NONE";
//    for (const auto sub : g_netlist->get_modules())
//    {
//        if (sub->contains_gate(g))
//        {
//            module_text = QString::fromStdString(sub->get_name()) + "[" + QString::number(sub->get_id()) + "]";
//            m_module_item->setData(Qt::UserRole, sub->get_id());
//        }
//    }
//    m_module_item->setText(module_text);

    // display Boolean function (if present)
//    m_boolean_function->setText("");

//    std::string description = "";
//    for (const auto& it : g->get_boolean_functions())
//        description += " <b>Boolean Function</b>: " + it.first + " = " + it.second.to_string() + "<br>";

//    if (description.empty())
//    {
//        g->set_data("gui", "boolean_function", "string", "-");
//    }
//    else
//    {
//        description = description.substr(0, description.size() - 4);
//        m_boolean_function->setText(QString::fromStdString(description));
//    }

//    if (m_boolean_function->text().isEmpty())
//        m_boolean_function->hide();
//    else
//        m_boolean_function->show();
    update_boolean_function();

    // display all data fields
//    m_data_fields->setText("");
//    std::string data_str = "";
//    auto data_fields     = g->get_data();

//    for (const auto& [key, value] : data_fields)
//    {
//        data_str += "<b>" + std::get<1>(key) + ": </b>" + std::get<1>(value) + "<br>";
//    }

//    if (!data_str.empty())
//    {
//        data_str = data_str.substr(0, data_str.size() - 4);
//        m_data_fields->setText(QString::fromStdString(data_str));

//        if (m_data_fields->text().isEmpty())
//        {
//            m_data_fields->hide();
//        }
//        else
//        {
//            m_data_fields->show();
//        }
//    }
//    else
//    {
//        m_data_fields->hide();
//    }

//    disconnect(m_tree_widget, &QTreeWidget::itemClicked, this, 0);

//    for (auto item : m_input_pins->takeChildren())
//        delete item;

//    auto input_pins = g_netlist->get_gate_by_id(gate_id)->get_input_pins();
//    if (input_pins.size() == 1)
//        m_input_pins->setText(0, "1 Input Pin");
//    else
//        m_input_pins->setText(0, QString::number(input_pins.size()) + " Input Pins");
//    for (const auto& pin : input_pins)
//    {
//        QTreeWidgetItem* item = new QTreeWidgetItem(m_input_pins);
//        item->setText(0, QString::fromStdString(pin));
//        item->setText(1, QChar(0x2b05));
//        item->setForeground(1, QBrush(QColor(114, 140, 0), Qt::SolidPattern));

//        auto input_net = g_netlist->get_gate_by_id(gate_id)->get_fan_in_net(pin);
//        if (input_net == nullptr)
//            item->setText(2, "unconnected");
//        else
//        {
//            item->setText(2, QString::fromStdString(input_net->get_name()));
//            item->setData(2, Qt::UserRole, input_net->get_id());
//        }
//    }

//    for (auto item : m_output_pins->takeChildren())
//        delete item;

//    auto output_pins = g_netlist->get_gate_by_id(gate_id)->get_output_pins();
//    if (output_pins.size() == 1)
//        m_output_pins->setText(0, "1 Output Pin");
//    else
//        m_output_pins->setText(0, QString::number(output_pins.size()) + " Output Pins");
//    for (const auto& pin : output_pins)
//    {
//        QTreeWidgetItem* item = new QTreeWidgetItem(m_output_pins);
//        item->setText(0, QString::fromStdString(pin));
//        item->setText(1, QChar(0x27a1));
//        item->setForeground(1, QBrush(QColor(114, 140, 0), Qt::SolidPattern));

//        auto output_net = g_netlist->get_gate_by_id(gate_id)->get_fan_out_net(pin);
//        if (output_net == nullptr)
//            item->setText(2, "unconnected");
//        else
//        {
//            item->setText(2, QString::fromStdString(output_net->get_name()));
//            item->setData(2, Qt::UserRole, output_net->get_id());
//        }
//    }

//    m_general_table->setHidden(false);
//    m_scroll_area->setHidden(false);

//    connect(m_tree_widget, &QTreeWidget::itemClicked, this, &gate_details_widget::on_treewidget_item_clicked);
}

void gate_details_widget::update(const u32 gate_id)
{
    auto g = g_netlist->get_gate_by_id(gate_id);
    m_current_id = gate_id;

    if(!g || m_current_id == 0)
        return;

    //update (1)general info section
    m_name_item->setText(QString::fromStdString(g->get_name()));
    m_type_item->setText(QString::fromStdString(g->get_type()->get_name()));
    m_id_item->setText(QString::number(m_current_id));

    QString module_text = "";
    for (const auto sub : g_netlist->get_modules())
    {
        if (sub->contains_gate(g))
        {
            module_text = QString::fromStdString(sub->get_name()) + "[" + QString::number(sub->get_id()) + "]";
            m_module_item->setData(Qt::UserRole, sub->get_id());
        }
    }
    m_module_item->setText(module_text);
    m_general_table->resizeColumnsToContents();
    m_general_table->setFixedWidth(calculate_table_size(m_general_table).width());

    //update (2)input-pin section (put the sections code in extra functions?)
    m_input_pins_table->clearContents();
    m_input_pins_button->setText(QString::fromStdString("Input Pins (") + QString::number(g->get_input_pins().size()) + QString::fromStdString(")"));
    m_input_pins_table->setRowCount(g->get_input_pins().size());
    //m_input_pins_table->setFixedHeight(m_input_pins_table->verticalHeader()->length());
    m_input_pins_table->setMaximumHeight(m_input_pins_table->verticalHeader()->length());
    int index = 0;
    for(const auto &pin : g->get_input_pins())
    {
        QTableWidgetItem* pin_name = new QTableWidgetItem(QString::fromStdString(pin));
        QTableWidgetItem* arrow_item = new QTableWidgetItem(QChar(0x2b05));
        QTableWidgetItem* net_item = new QTableWidgetItem();
        arrow_item->setForeground(QBrush(QColor(114, 140, 0), Qt::SolidPattern));
        pin_name->setFlags(Qt::ItemIsEnabled);
        arrow_item->setFlags(Qt::ItemIsEnabled);
        net_item->setFlags(Qt::ItemIsEnabled);

        auto input_net = g_netlist->get_gate_by_id(gate_id)->get_fan_in_net(pin);
        if(input_net)
        {
            net_item->setText(QString::fromStdString(input_net->get_name()));
            net_item->setData(Qt::UserRole, input_net->get_id());
        }
        else
            net_item->setText("unconnected");


        m_input_pins_table->setItem(index, 0, pin_name);
        m_input_pins_table->setItem(index, 1, arrow_item);
        m_input_pins_table->setItem(index, 2, net_item);
        index++;
    }
    m_input_pins_table->resizeColumnsToContents();
    m_input_pins_table->setFixedWidth(calculate_table_size(m_input_pins_table).width());

    //update(3) output pins section
    m_output_pins_table->clearContents();
    m_output_pins_button->setText(QString::fromStdString("Output Pins (") + QString::number(g->get_output_pins().size()) + QString::fromStdString(")"));
    m_output_pins_table->setRowCount(g->get_output_pins().size());
    m_output_pins_table->setMaximumHeight(m_output_pins_table->verticalHeader()->length());
    index = 0;
    for(const auto &pin : g->get_output_pins())
    {
        QTableWidgetItem* pin_name = new QTableWidgetItem(QString::fromStdString(pin));
        QTableWidgetItem* arrow_item = new QTableWidgetItem(QChar(0x27a1));
        QTableWidgetItem* net_item = new QTableWidgetItem();
        arrow_item->setForeground(QBrush(QColor(114, 140, 0), Qt::SolidPattern));//stylesheet?
        pin_name->setFlags(Qt::ItemIsEnabled);
        arrow_item->setFlags(Qt::ItemIsEnabled);
        net_item->setFlags(Qt::ItemIsEnabled);

        auto output_net = g_netlist->get_gate_by_id(gate_id)->get_fan_out_net(pin);
        if(output_net)
        {
            net_item->setText(QString::fromStdString(output_net->get_name()));
            net_item->setData(Qt::UserRole, output_net->get_id());
        }
        else
            net_item->setText("unconnected");

        m_output_pins_table->setItem(index, 0, pin_name);
        m_output_pins_table->setItem(index, 1, arrow_item);
        m_output_pins_table->setItem(index, 2, net_item);
        index++;
    }
    m_output_pins_table->resizeColumnsToContents();
    m_output_pins_table->setFixedWidth(calculate_table_size(m_output_pins_table).width());

    //update(4) data fields section
    m_data_fields_button->setText(QString::fromStdString("Data Fields (") + QString::number(g->get_data().size()) + QString::fromStdString(")"));
    m_data_fields_table->clearContents();
    m_data_fields_table->setRowCount(g->get_data().size());
    //m_data_fields_table->setFixedHeight(m_data_fields_table->verticalHeader()->length());
    m_data_fields_table->setMaximumHeight(m_data_fields_table->verticalHeader()->length());
    index = 0;
    for(const auto& [key, value] : g->get_data())
    {
        QTableWidgetItem* key_item = new QTableWidgetItem(QString::fromStdString(std::get<1>(key)) + QString(":"));
        key_item->setFont(m_key_font);
        key_item->setFlags(Qt::ItemIsEnabled);
        QTableWidgetItem* value_item = new QTableWidgetItem(QString::fromStdString(std::get<1>(value)));
        value_item->setFlags(Qt::ItemIsEnabled);

        m_data_fields_table->setItem(index, 0, key_item);
        m_data_fields_table->setItem(index, 1, value_item);
        index++;
    }
    m_data_fields_table->resizeColumnsToContents();
    m_data_fields_table->setFixedWidth(calculate_table_size(m_data_fields_table).width());


    //update(5) boolean functions section
    //clear container layout
    while(m_boolean_functions_container_layout->itemAt(0) != 0)
    {
        QLayoutItem* i = m_boolean_functions_container_layout->takeAt(0);
        delete i->widget();
        delete i;
    }

    m_boolean_functions_button->setText(QString("Boolean Functions (") + QString::number(g->get_boolean_functions().size()) + QString(")"));
    QFrame* last_line = nullptr; //unexpected behaviour below otherwise
    for(const auto& it : g->get_boolean_functions())
    {
        QLabel* fnct = new QLabel(QString::fromStdString(it.first) + " = " + QString::fromStdString(it.second.to_string()));
        fnct->setWordWrap(true);
        m_boolean_functions_container_layout->addWidget(fnct);
        QFrame* line = new QFrame;
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        line->setStyleSheet("QFrame{background-color: gray;}"); //tmp test
        last_line = line;
        m_boolean_functions_container_layout->addWidget(line);
    }

    if(last_line){
        m_boolean_functions_container_layout->removeWidget(last_line);
        delete last_line;
    }
}

void gate_details_widget::on_treewidget_item_clicked(QTreeWidgetItem* item, int column)
{
//    auto net_id = item->data(2, Qt::UserRole).toInt();
//    if (m_output_pins == item->parent() && column == 2)
//    {
//        std::shared_ptr<net> clicked_net = g_netlist->get_net_by_id(net_id);

//        if (!clicked_net)
//            return;

//        auto destinations = clicked_net->get_destinations();

//        if (destinations.empty() || clicked_net->is_global_output_net())
//        {
//            g_selection_relay.clear();
//            g_selection_relay.m_selected_nets.insert(clicked_net->get_id());
//            g_selection_relay.relay_selection_changed(this);
//        }
//        else if (destinations.size() == 1)
//        {
//            auto ep = *destinations.begin();
//            g_selection_relay.clear();
//            g_selection_relay.m_selected_gates.insert(ep.get_gate()->get_id());
//            g_selection_relay.m_focus_type = selection_relay::item_type::gate;
//            g_selection_relay.m_focus_id   = ep.get_gate()->get_id();
//            g_selection_relay.m_subfocus   = selection_relay::subfocus::left;

//            auto pins                          = ep.get_gate()->get_input_pins();
//            auto index                         = std::distance(pins.begin(), std::find(pins.begin(), pins.end(), ep.get_pin()));
//            g_selection_relay.m_subfocus_index = index;

//            update(ep.get_gate()->get_id());
//            g_selection_relay.relay_selection_changed(this);
//        }
//        else
//        {
//            //            auto rect = QApplication::desktop()->availableGeometry(this);
//            //            w->move(QPoint(rect.x() + (rect.width() - w->width()) / 2, rect.y() + (rect.height() - w->height()) / 2));
//            m_navigation_table->setup(hal::node{hal::node_type::gate, 0}, clicked_net, true);
//            m_navigation_table->move(QCursor::pos());
//            m_navigation_table->show();
//            m_navigation_table->setFocus();
//        }
//    }
//    else if (m_input_pins == item->parent() && column == 2)
//    {
//        auto clicked_net = g_netlist->get_net_by_id(net_id);

//        if (!clicked_net)
//            return;

//        auto sources = clicked_net->get_sources();

//        if (sources.empty() || clicked_net->is_global_input_net())
//        {
//            g_selection_relay.clear();
//            g_selection_relay.m_selected_nets.insert(clicked_net->get_id());
//            g_selection_relay.relay_selection_changed(this);
//        }
//        else if (sources.size() == 1)
//        {
//            auto ep = *sources.begin();
//            g_selection_relay.clear();
//            g_selection_relay.m_selected_gates.insert(ep.get_gate()->get_id());
//            g_selection_relay.m_focus_type = selection_relay::item_type::gate;
//            g_selection_relay.m_focus_id   = ep.get_gate()->get_id();
//            g_selection_relay.m_subfocus   = selection_relay::subfocus::right;

//            auto pins                          = ep.get_gate()->get_output_pins();
//            auto index                         = std::distance(pins.begin(), std::find(pins.begin(), pins.end(), ep.get_pin()));
//            g_selection_relay.m_subfocus_index = index;

//            update(ep.get_gate()->get_id());
//            g_selection_relay.relay_selection_changed(this);
//        }
//        else
//        {
//            //            auto rect = QApplication::desktop()->availableGeometry(this);
//            //            w->move(QPoint(rect.x() + (rect.width() - w->width()) / 2, rect.y() + (rect.height() - w->height()) / 2));
//            m_navigation_table->setup(hal::node{hal::node_type::gate, 0}, clicked_net, false);
//            m_navigation_table->move(QCursor::pos());
//            m_navigation_table->show();
//            m_navigation_table->setFocus();
//        }
//    }
}

//always the right-subfocus!!!!!!(the other way is handled: on_treewidget_item_clicked)
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
