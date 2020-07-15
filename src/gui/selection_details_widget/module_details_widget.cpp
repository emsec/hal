#include "selection_details_widget/module_details_widget.h"

#include "graph_widget/graph_navigation_widget.h"
#include "input_dialog/input_dialog.h"
#include "gui_globals.h"
#include "netlist/gate.h"
#include "netlist/module.h"
#include "netlist/net.h"

#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QMenu>
#include <QMouseEvent>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QTableWidget>
#include <QVBoxLayout>

namespace hal
{
    ModuleDetailsWidget::ModuleDetailsWidget(QWidget* parent) : QWidget(parent)
    {
        init_settings();
        m_current_id = 0;
        m_key_font = QFont("Iosevka");
        m_key_font.setBold(true);
        m_key_font.setPixelSize(13);

        m_scroll_area       = new QScrollArea();
        m_top_lvl_container = new QWidget();
        m_top_lvl_layout    = new QVBoxLayout(m_top_lvl_container);
        m_top_lvl_container->setLayout(m_top_lvl_layout);
        m_content_layout = new QVBoxLayout(this);
        m_scroll_area->setWidget(m_top_lvl_container);
        m_scroll_area->setWidgetResizable(true);

        m_content_layout->setContentsMargins(0, 0, 0, 0);
        m_content_layout->setSpacing(0);
        m_top_lvl_layout->setContentsMargins(0, 0, 0, 0);
        m_top_lvl_layout->setSpacing(0);

        QHBoxLayout* intermediate_layout_gt = new QHBoxLayout();
        intermediate_layout_gt->setContentsMargins(3, 3, 0, 0);
        intermediate_layout_gt->setSpacing(0);
        QHBoxLayout* intermediate_layout_ip = new QHBoxLayout();
        intermediate_layout_ip->setContentsMargins(3, 3, 0, 0);
        intermediate_layout_ip->setSpacing(10);
        QHBoxLayout* intermediate_layout_op = new QHBoxLayout();
        intermediate_layout_op->setContentsMargins(3, 3, 0, 0);
        intermediate_layout_op->setSpacing(0);
        QHBoxLayout* intermediate_layout_df = new QHBoxLayout();
        intermediate_layout_df->setContentsMargins(3, 3, 0, 0);
        intermediate_layout_df->setSpacing(0);

        m_general_info_button = new QPushButton("General Information", this);
        m_general_info_button->setEnabled(false);
        m_input_ports_button  = new QPushButton("Input Ports", this);
        m_output_ports_button = new QPushButton("Output Ports", this);
        m_data_fields_button = new QPushButton("Data Fields", this);

        m_general_table      = new QTableWidget(6, 2);
        m_input_ports_table  = new QTableWidget(0, 3);
        m_output_ports_table = new QTableWidget(0, 3);
        m_data_fields_table = new QTableWidget(0, 2);

        QList<QTableWidget*> tmp_tables = {m_general_table, m_input_ports_table, m_output_ports_table, m_data_fields_table};

        QList<QTableWidgetItem*> tmp_general_table_static_items = {new QTableWidgetItem("Name:"),
                                                                   new QTableWidgetItem("Id:"),
                                                                   new QTableWidgetItem("Type:"),
                                                                   new QTableWidgetItem("Gates:"),
                                                                   new QTableWidgetItem("Submodules:"),
                                                                   new QTableWidgetItem("Nets:")};

        QList<QTableWidgetItem*> tmp_general_table_dynamic_items = {m_name_item                 = new QTableWidgetItem(),
                                                                    m_id_item                   = new QTableWidgetItem(),
                                                                    m_type_item                 = new QTableWidgetItem(),
                                                                    m_number_of_gates_item      = new QTableWidgetItem(),
                                                                    m_number_of_submodules_item = new QTableWidgetItem(),
                                                                    m_number_of_nets_item       = new QTableWidgetItem()};

        for (const auto& table : tmp_tables)
            style_table(table);

        for (const auto& item : tmp_general_table_static_items)
            add_general_table_static_item(item);

        for (const auto& item : tmp_general_table_dynamic_items)
            add_general_table_dynamic_item(item);

        //first 3 items of the general table are interactive, instead of checking id the function above
        //just declare it here
        m_name_item->setFlags(Qt::ItemIsEnabled);
        m_id_item->setFlags(Qt::ItemIsEnabled);
        m_type_item->setFlags(Qt::ItemIsEnabled);

        intermediate_layout_gt->addWidget(m_general_table);
        intermediate_layout_gt->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
        intermediate_layout_ip->addWidget(m_input_ports_table);
        intermediate_layout_ip->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
        intermediate_layout_op->addWidget(m_output_ports_table);
        intermediate_layout_op->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
        intermediate_layout_df->addWidget(m_data_fields_table);
        intermediate_layout_df->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

        m_top_lvl_layout->addWidget(m_general_info_button);
        m_top_lvl_layout->addLayout(intermediate_layout_gt);
        m_top_lvl_layout->addSpacerItem(new QSpacerItem(0, 7, QSizePolicy::Expanding, QSizePolicy::Fixed));
        m_top_lvl_layout->addWidget(m_input_ports_button);
        m_top_lvl_layout->addLayout(intermediate_layout_ip);
        m_top_lvl_layout->addSpacerItem(new QSpacerItem(0, 7, QSizePolicy::Expanding, QSizePolicy::Fixed));
        m_top_lvl_layout->addWidget(m_output_ports_button);
        m_top_lvl_layout->addLayout(intermediate_layout_op);
        m_top_lvl_layout->addSpacerItem(new QSpacerItem(0, 7, QSizePolicy::Expanding, QSizePolicy::Fixed));
        m_top_lvl_layout->addWidget(m_data_fields_button);
        m_top_lvl_layout->addLayout(intermediate_layout_df);

        m_top_lvl_layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
        m_content_layout->addWidget(m_scroll_area);

        m_util_list << m_input_ports_button << m_output_ports_button << m_data_fields_button;

        //setup the navigation_table ("activated" by clicking on an input / output pin in the 2 tables)
        //delete the table manually so its not necessarry to add a property for the stylesheet(otherwise this table is styled like the others)
        m_navigation_table = new GraphNavigationWidget();
        m_navigation_table->setWindowFlags(Qt::CustomizeWindowHint);
        m_navigation_table->hide_when_focus_lost(true);
        m_navigation_table->hide();

        connect(m_navigation_table, &GraphNavigationWidget::navigation_requested, this, &ModuleDetailsWidget::handle_navigation_jump_requested);

        //connect(m_general_info_button, &QPushButton::clicked, this, &ModuleDetailsWidget::handle_buttons_clicked);
        connect(m_input_ports_button, &QPushButton::clicked, this, &ModuleDetailsWidget::handle_buttons_clicked);
        connect(m_output_ports_button, &QPushButton::clicked, this, &ModuleDetailsWidget::handle_buttons_clicked);
        connect(m_data_fields_button, &QPushButton::clicked, this, &ModuleDetailsWidget::handle_buttons_clicked);

        connect(&g_netlist_relay, &NetlistRelay::netlist_marked_global_input, this, &ModuleDetailsWidget::handle_netlist_marked_global_input);
        connect(&g_netlist_relay, &NetlistRelay::netlist_marked_global_output, this, &ModuleDetailsWidget::handle_netlist_marked_global_output);
        connect(&g_netlist_relay, &NetlistRelay::netlist_marked_global_inout, this, &ModuleDetailsWidget::handle_netlist_marked_global_inout);
        connect(&g_netlist_relay, &NetlistRelay::netlist_unmarked_global_input, this, &ModuleDetailsWidget::handle_netlist_unmarked_global_input);
        connect(&g_netlist_relay, &NetlistRelay::netlist_unmarked_global_output, this, &ModuleDetailsWidget::handle_netlist_unmarked_global_output);
        connect(&g_netlist_relay, &NetlistRelay::netlist_unmarked_global_inout, this, &ModuleDetailsWidget::handle_netlist_unmarked_global_inout);

        connect(&g_netlist_relay, &NetlistRelay::module_name_changed, this, &ModuleDetailsWidget::handle_module_name_changed);
        connect(&g_netlist_relay, &NetlistRelay::module_submodule_added, this, &ModuleDetailsWidget::handle_submodule_added);
        connect(&g_netlist_relay, &NetlistRelay::module_submodule_removed, this, &ModuleDetailsWidget::handle_submodule_removed);
        connect(&g_netlist_relay, &NetlistRelay::module_gate_assigned, this, &ModuleDetailsWidget::handle_module_gate_assigned);
        connect(&g_netlist_relay, &NetlistRelay::module_gate_removed, this, &ModuleDetailsWidget::handle_module_gate_removed);
        connect(&g_netlist_relay, &NetlistRelay::module_input_port_name_changed, this, &ModuleDetailsWidget::handle_module_input_port_name_changed);
        connect(&g_netlist_relay, &NetlistRelay::module_output_port_name_changed, this, &ModuleDetailsWidget::handle_module_output_port_name_changed);
        connect(&g_netlist_relay, &NetlistRelay::module_type_changed, this, &ModuleDetailsWidget::handle_module_type_changed);

        connect(&g_netlist_relay, &NetlistRelay::net_name_changed, this, &ModuleDetailsWidget::handle_net_name_changed);
        connect(&g_netlist_relay, &NetlistRelay::net_source_added, this, &ModuleDetailsWidget::handle_net_source_added);
        connect(&g_netlist_relay, &NetlistRelay::net_source_removed, this, &ModuleDetailsWidget::handle_net_source_removed);
        connect(&g_netlist_relay, &NetlistRelay::net_destination_added, this, &ModuleDetailsWidget::handle_net_destination_added);
        connect(&g_netlist_relay, &NetlistRelay::net_destination_removed, this, &ModuleDetailsWidget::handle_net_destination_removed);

        connect(m_general_table, &QTableWidget::customContextMenuRequested, this, &ModuleDetailsWidget::handle_general_table_menu_requested);
        connect(m_input_ports_table, &QTableWidget::customContextMenuRequested, this, &ModuleDetailsWidget::handle_input_ports_table_menu_requested);
        connect(m_output_ports_table, &QTableWidget::customContextMenuRequested, this, &ModuleDetailsWidget::handle_output_ports_table_menu_requested);
        connect(m_input_ports_table, &QTableWidget::itemDoubleClicked, this, &ModuleDetailsWidget::handle_input_net_item_clicked);
        connect(m_output_ports_table, &QTableWidget::itemDoubleClicked, this, &ModuleDetailsWidget::handle_output_net_item_clicked);

        //settings
        connect(&g_settings_relay, &SettingsRelay::setting_changed, this, &ModuleDetailsWidget::handle_global_settings_changed);

        //eventfilters
        m_input_ports_table->viewport()->setMouseTracking(true);
        m_input_ports_table->viewport()->installEventFilter(this);
        m_output_ports_table->viewport()->setMouseTracking(true);
        m_output_ports_table->viewport()->installEventFilter(this);
    }

    ModuleDetailsWidget::~ModuleDetailsWidget()
    {
        delete m_navigation_table;
    }

    bool ModuleDetailsWidget::eventFilter(QObject* watched, QEvent* event)
    {
        if (event->type() == QEvent::MouseMove)
        {
            QTableWidget* table    = (watched == m_input_ports_table->viewport()) ? m_input_ports_table : m_output_ports_table;
            QMouseEvent* ev        = dynamic_cast<QMouseEvent*>(event);
            QTableWidgetItem* item = table->itemAt(ev->pos());
            if (item)
            {
                if (item->column() == 2)
                    setCursor(QCursor(Qt::PointingHandCursor));
                else
                    setCursor(QCursor(Qt::ArrowCursor));
            }
            else
                setCursor(QCursor(Qt::ArrowCursor));
        }

        //restore default cursor when leaving any watched widget (maybe save cursor before entering?)
        if (event->type() == QEvent::Leave)
            setCursor(QCursor(Qt::ArrowCursor));

        return false;
    }

    void ModuleDetailsWidget::update(const u32 module_id)
    {
        m_current_id = module_id;

        if (m_current_id == 0)
            return;

        auto m = g_netlist->get_module_by_id(module_id);

        if (!m)
            return;

        show_all_sections();

        //update table with general information
        m_name_item->setText(QString::fromStdString(m->get_name()));
        m_id_item->setText(QString::number(m_current_id));
        m_number_of_submodules_item->setText(QString::number(m->get_submodules(nullptr, true).size()));
        m_number_of_nets_item->setText(QString::number(m->get_internal_nets().size()));

        QString type_text = QString::fromStdString(m->get_type());

        if (type_text.isEmpty())
            type_text = "None";

        m_type_item->setText(type_text);

        int total_number_of_gates           = m->get_gates(nullptr, true).size();
        int direct_member_number_of_gates   = m->get_gates(nullptr, false).size();
        int indirect_member_number_of_gates = 0;

        for (const auto& module : m->get_submodules())
            indirect_member_number_of_gates += module->get_gates(nullptr, true).size();

        QString number_of_gates_text = QString::number(total_number_of_gates);

        if (indirect_member_number_of_gates > 0)
            number_of_gates_text += " in total, " + QString::number(direct_member_number_of_gates) + " as direct members and " + QString::number(indirect_member_number_of_gates) + " in submodules";

        m_number_of_gates_item->setText(number_of_gates_text);

        m_general_table->resizeColumnsToContents();
        m_general_table->setFixedWidth(calculate_table_size(m_general_table).width());
        m_general_table->update();

        //update table with input ports
        m_input_ports_table->clearContents();
        m_input_ports_button->setText(QString::fromStdString("Input Ports (") + QString::number(m->get_input_nets().size()) + QString::fromStdString(")"));

        m_input_ports_table->setRowCount(m->get_input_nets().size());
        m_input_ports_table->setMaximumHeight(m_input_ports_table->verticalHeader()->length());
        m_input_ports_table->setMinimumHeight(m_input_ports_table->verticalHeader()->length());

        int index = 0;
        for (const auto& net : m->get_input_nets())
        {
            QTableWidgetItem* port_name  = new QTableWidgetItem(QString::fromStdString(m->get_input_port_name(net)));
            QTableWidgetItem* arrow_item = new QTableWidgetItem(QChar(0x2b05));
            QTableWidgetItem* net_item   = new QTableWidgetItem(QString::fromStdString(net->get_name()));

            arrow_item->setForeground(QBrush(QColor(114, 140, 0), Qt::SolidPattern));
            port_name->setFlags(Qt::ItemIsEnabled);
            arrow_item->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
            net_item->setFlags(Qt::ItemIsEnabled);
            net_item->setData(Qt::UserRole, net->get_id());

            m_input_ports_table->setItem(index, 0, port_name);
            m_input_ports_table->setItem(index, 1, arrow_item);
            m_input_ports_table->setItem(index, 2, net_item);

            index++;
        }

        m_input_ports_table->resizeColumnsToContents();
        m_input_ports_table->setFixedWidth(calculate_table_size(m_input_ports_table).width());

        //update table with output ports
        m_output_ports_table->clearContents();
        m_output_ports_button->setText(QString::fromStdString("Output Ports (") + QString::number(m->get_output_nets().size()) + QString::fromStdString(")"));

        m_output_ports_table->setRowCount(m->get_output_nets().size());
        m_output_ports_table->setMaximumHeight(m_output_ports_table->verticalHeader()->length());
        m_output_ports_table->setMinimumHeight(m_output_ports_table->verticalHeader()->length());

        index = 0;
        for (const auto& net : m->get_output_nets())
        {
            QTableWidgetItem* port_name  = new QTableWidgetItem(QString::fromStdString(m->get_output_port_name(net)));
            QTableWidgetItem* arrow_item = new QTableWidgetItem(QChar(0x27a1));
            QTableWidgetItem* net_item   = new QTableWidgetItem(QString::fromStdString(net->get_name()));

            arrow_item->setForeground(QBrush(QColor(114, 140, 0), Qt::SolidPattern));
            port_name->setFlags(Qt::ItemIsEnabled);
            arrow_item->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
            net_item->setFlags(Qt::ItemIsEnabled);
            net_item->setData(Qt::UserRole, net->get_id());

            m_output_ports_table->setItem(index, 0, port_name);
            m_output_ports_table->setItem(index, 1, arrow_item);
            m_output_ports_table->setItem(index, 2, net_item);

            index++;
        }

        m_output_ports_table->resizeColumnsToContents();
        m_output_ports_table->setFixedWidth(calculate_table_size(m_output_ports_table).width());

        //update data fields table
        m_data_fields_table->clearContents();
        m_data_fields_button->setText("Data Fields (" + QString::number(m->get_data().size()) + ")");
        m_data_fields_table->setRowCount(m->get_data().size());
        m_data_fields_table->setMaximumHeight(m_data_fields_table->verticalHeader()->length());
        index = 0;
        for(const auto& [key, value] : m->get_data())
        {
            QTableWidgetItem* key_item = new QTableWidgetItem(QString::fromStdString(std::get<1>(key)) + QString(":"));
            key_item->setFont(m_key_font);
            key_item->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
            key_item->setData(Qt::UserRole, QString::fromStdString(std::get<0>(key)));
            QTableWidgetItem* value_item = new QTableWidgetItem(QString::fromStdString(std::get<1>(value)));
            value_item->setFlags(Qt::ItemIsEnabled);

            m_data_fields_table->setItem(index, 0, key_item);
            m_data_fields_table->setItem(index, 1, value_item);
            index++;
        }
        m_data_fields_table->resizeColumnsToContents();
        m_data_fields_table->setFixedWidth(calculate_table_size(m_data_fields_table).width());

        if(m_hide_empty_sections)
            hide_empty_sections();
    }

    void ModuleDetailsWidget::handle_netlist_marked_global_input(std::shared_ptr<Netlist> netlist, u32 associated_data)
    {
        Q_UNUSED(netlist)

        if (m_current_id == 0)
            return;

        auto module = g_netlist->get_module_by_id(m_current_id);
        auto gates  = module->get_gates(nullptr, true);
        auto net    = g_netlist->get_net_by_id(associated_data);

        for (const auto& gate : gates)
        {
            auto in_nets  = gate->get_fan_in_nets();
            auto out_nets = gate->get_fan_out_nets();

            if (in_nets.find(net) != in_nets.end() || out_nets.find(net) != out_nets.end())
            {
                update(m_current_id);
                return;
            }
        }
    }

    void ModuleDetailsWidget::handle_netlist_marked_global_output(std::shared_ptr<Netlist> netlist, u32 associated_data)
    {
        Q_UNUSED(netlist)

        if (m_current_id == 0)
            return;

        auto module = g_netlist->get_module_by_id(m_current_id);
        auto gates  = module->get_gates(nullptr, true);
        auto net    = g_netlist->get_net_by_id(associated_data);

        for (const auto& gate : gates)
        {
            auto in_nets  = gate->get_fan_in_nets();
            auto out_nets = gate->get_fan_out_nets();

            if (in_nets.find(net) != in_nets.end() || out_nets.find(net) != out_nets.end())
            {
                update(m_current_id);
                return;
            }
        }
    }

    void ModuleDetailsWidget::handle_netlist_marked_global_inout(std::shared_ptr<Netlist> netlist, u32 associated_data)
    {
        Q_UNUSED(netlist)

        if (m_current_id == 0)
            return;

        auto module = g_netlist->get_module_by_id(m_current_id);
        auto gates  = module->get_gates(nullptr, true);
        auto net    = g_netlist->get_net_by_id(associated_data);

        for (const auto& gate : gates)
        {
            auto in_nets  = gate->get_fan_in_nets();
            auto out_nets = gate->get_fan_out_nets();

            if (in_nets.find(net) != in_nets.end() || out_nets.find(net) != out_nets.end())
            {
                update(m_current_id);
                return;
            }
        }
    }

    void ModuleDetailsWidget::handle_netlist_unmarked_global_input(std::shared_ptr<Netlist> netlist, u32 associated_data)
    {
        Q_UNUSED(netlist)

        if (m_current_id == 0)
            return;

        auto module = g_netlist->get_module_by_id(m_current_id);
        auto gates  = module->get_gates(nullptr, true);
        auto net    = g_netlist->get_net_by_id(associated_data);

        for (const auto& gate : gates)
        {
            auto in_nets  = gate->get_fan_in_nets();
            auto out_nets = gate->get_fan_out_nets();

            if (in_nets.find(net) != in_nets.end() || out_nets.find(net) != out_nets.end())
            {
                update(m_current_id);
                return;
            }
        }
    }

    void ModuleDetailsWidget::handle_netlist_unmarked_global_output(std::shared_ptr<Netlist> netlist, u32 associated_data)
    {
        Q_UNUSED(netlist)

        if (m_current_id == 0)
            return;

        auto module = g_netlist->get_module_by_id(m_current_id);
        auto gates  = module->get_gates(nullptr, true);
        auto net    = g_netlist->get_net_by_id(associated_data);

        for (const auto& gate : gates)
        {
            auto in_nets  = gate->get_fan_in_nets();
            auto out_nets = gate->get_fan_out_nets();

            if (in_nets.find(net) != in_nets.end() || out_nets.find(net) != out_nets.end())
            {
                update(m_current_id);
                return;
            }
        }
    }

    void ModuleDetailsWidget::handle_netlist_unmarked_global_inout(std::shared_ptr<Netlist> netlist, u32 associated_data)
    {
        Q_UNUSED(netlist)

        if (m_current_id == 0)
            return;

        auto module = g_netlist->get_module_by_id(m_current_id);
        auto gates  = module->get_gates(nullptr, true);
        auto net    = g_netlist->get_net_by_id(associated_data);

        for (const auto& gate : gates)
        {
            auto in_nets  = gate->get_fan_in_nets();
            auto out_nets = gate->get_fan_out_nets();

            if (in_nets.find(net) != in_nets.end() || out_nets.find(net) != out_nets.end())
            {
                update(m_current_id);
                return;
            }
        }
    }

    void ModuleDetailsWidget::handle_module_name_changed(std::shared_ptr<Module> module)
    {
        if (m_current_id == module->get_id())
            update(m_current_id);
    }

    void ModuleDetailsWidget::handle_submodule_added(std::shared_ptr<Module> module, u32 associated_data)
    {
        Q_UNUSED(associated_data);

        if (m_current_id == 0)
            return;

        auto current_module = g_netlist->get_module_by_id(m_current_id);

        if (m_current_id == module->get_id() || current_module->contains_module(module, true))
            update(m_current_id);
    }

    void ModuleDetailsWidget::handle_submodule_removed(std::shared_ptr<Module> module, u32 associated_data)
    {
        Q_UNUSED(associated_data);

        if (m_current_id == 0)
            return;

        auto current_module = g_netlist->get_module_by_id(m_current_id);

        if (m_current_id == module->get_id() || current_module->contains_module(module, true))
            update(m_current_id);
    }

    void ModuleDetailsWidget::handle_module_gate_assigned(std::shared_ptr<Module> module, u32 associated_data)
    {
        Q_UNUSED(associated_data);

        if (m_current_id == 0)
            return;

        auto current_module = g_netlist->get_module_by_id(m_current_id);

        if (m_current_id == module->get_id() || current_module->contains_module(module, true))
            update(m_current_id);
    }

    void ModuleDetailsWidget::handle_module_gate_removed(std::shared_ptr<Module> module, u32 associated_data)
    {
        Q_UNUSED(associated_data);

        if (m_current_id == 0)
            return;

        auto current_module = g_netlist->get_module_by_id(m_current_id);

        if (m_current_id == module->get_id() || current_module->contains_module(module, true))
            update(m_current_id);
    }

    void ModuleDetailsWidget::handle_module_input_port_name_changed(std::shared_ptr<Module> module, u32 associated_data)
    {
        Q_UNUSED(associated_data);

        if (m_current_id == module->get_id())
            update(m_current_id);
    }

    void ModuleDetailsWidget::handle_module_output_port_name_changed(std::shared_ptr<Module> module, u32 associated_data)
    {
        Q_UNUSED(associated_data);

        if (m_current_id == module->get_id())
            update(m_current_id);
    }

    void ModuleDetailsWidget::handle_module_type_changed(std::shared_ptr<Module> module)
    {
        if (m_current_id == module->get_id())
            update(m_current_id);
    }

    void ModuleDetailsWidget::handle_net_name_changed(std::shared_ptr<Net> net)
    {
        if (m_current_id == 0)
            return;

        auto module      = g_netlist->get_module_by_id(m_current_id);
        auto input_nets  = module->get_input_nets();
        auto output_nets = module->get_output_nets();

        if (input_nets.find(net) != input_nets.end() || output_nets.find(net) != output_nets.end())
            update(m_current_id);
    }

    void ModuleDetailsWidget::handle_net_source_added(std::shared_ptr<Net> net, const u32 src_gate_id)
    {
        Q_UNUSED(net)

        if (m_current_id == 0)
            return;

        auto module = g_netlist->get_module_by_id(m_current_id);
        auto gate   = g_netlist->get_gate_by_id(src_gate_id);

        if (module->contains_gate(gate, true))
            update(m_current_id);
    }

    void ModuleDetailsWidget::handle_net_source_removed(std::shared_ptr<Net> net, const u32 src_gate_id)
    {
        Q_UNUSED(net)

        if (m_current_id == 0)
            return;

        auto module = g_netlist->get_module_by_id(m_current_id);
        auto gate   = g_netlist->get_gate_by_id(src_gate_id);

        if (module->contains_gate(gate, true))
            update(m_current_id);
    }

    void ModuleDetailsWidget::handle_net_destination_added(std::shared_ptr<Net> net, const u32 dst_gate_id)
    {
        Q_UNUSED(net)

        if (m_current_id == 0)
            return;

        auto module = g_netlist->get_module_by_id(m_current_id);
        auto gate   = g_netlist->get_gate_by_id(dst_gate_id);

        if (module->contains_gate(gate, true))
            update(m_current_id);
    }

    void ModuleDetailsWidget::handle_net_destination_removed(std::shared_ptr<Net> net, const u32 dst_gate_id)
    {
        Q_UNUSED(net)

        if (m_current_id == 0)
            return;

        auto module = g_netlist->get_module_by_id(m_current_id);
        auto gate   = g_netlist->get_gate_by_id(dst_gate_id);

        if (module->contains_gate(gate, true))
            update(m_current_id);
    }

    void ModuleDetailsWidget::handle_buttons_clicked()
    {
        QPushButton* btn = dynamic_cast<QPushButton*>(sender());

        if (!btn)
            return;

        int index = m_top_lvl_layout->indexOf(btn);

        QWidget* widget;
        widget = m_top_lvl_layout->itemAt(index + 1)->layout()->itemAt(0)->widget();

        if (!widget)
            return;

        if (widget->isHidden())
            widget->show();
        else
            widget->hide();
    }

    void ModuleDetailsWidget::add_general_table_static_item(QTableWidgetItem* item)
    {
        static int row_index = 0;

        item->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
        item->setFont(m_key_font);
        m_general_table->setItem(row_index, 0, item);

        row_index++;
    }

    void ModuleDetailsWidget::add_general_table_dynamic_item(QTableWidgetItem* item)
    {
        static int row_index = 0;

        item->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
        m_general_table->setItem(row_index, 1, item);

        row_index++;
    }

    QSize ModuleDetailsWidget::calculate_table_size(QTableWidget* table)
    {
        //necessary to test if the table is empty, otherwise (due to the resizeColumnsToContents function)
        //is the tables width far too big, so just return 0 as the size
        if (!table->rowCount())
            return QSize(0, 0);

        int w = table->verticalHeader()->width() + 4;    // +4 seems to be needed

        for (int i = 0; i < table->columnCount(); i++)
            w += table->columnWidth(i);    // seems to include gridline

        int h = table->horizontalHeader()->height() + 4;

        for (int i = 0; i < table->rowCount(); i++)
            h += table->rowHeight(i);

        return QSize(w + 5, h);
    }

    void ModuleDetailsWidget::style_table(QTableWidget* table)
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
        //not really a style, yet it applys to all tables
        table->setContextMenuPolicy(Qt::CustomContextMenu);
    }

    void ModuleDetailsWidget::handle_general_table_menu_requested(const QPoint& pos)
    {
        auto curr_item = m_general_table->itemAt(pos);

        if (!curr_item || curr_item->column() != 1 || curr_item->row() >= 3)
            return;

        QMenu menu;
        QString description;
        QString python_command = "netlist.get_module_by_id(" + QString::number(m_current_id) + ").";
        QString raw_string = curr_item->text(), raw_desc = "";
        switch (curr_item->row())
        {
            case 0:
                python_command += "get_name()";
                description = "Extract name as python code (copy to clipboard)";
                raw_desc    = "Extract raw name (copy to clipboard)";
                break;
            case 1:
                python_command += "get_id()";
                description = "Extract id as python code (copy to clipboard)";
                raw_desc    = "Extract raw id (copy to clipboard)";
                break;
            case 2:
                python_command += "get_type()";
                description = "Extract type as python code (copy to clipboard)";
                raw_desc    = "Extract raw type (copy to clipboard)";
                break;
            default:
                break;    //cases 3-5 are currently not in use
        }

        if(curr_item->row() == 0)
        {
            menu.addAction("Change name", [this, curr_item](){
                InputDialog ipd("Change name", "New name", curr_item->text());
                if(ipd.exec() == QDialog::Accepted)
                {
                    g_netlist->get_module_by_id(m_current_id)->set_name(ipd.text_value().toStdString());
                    update(m_current_id);
                }
            });
        }

        if(curr_item->row() == 2)
        {
            menu.addAction("Change type", [this, curr_item](){
                InputDialog ipd("Change type", "New type", curr_item->text());
                if(ipd.exec() == QDialog::Accepted)
                {
                    g_netlist->get_module_by_id(m_current_id)->set_type(ipd.text_value().toStdString());
                    update(m_current_id);
                }
            });
        }

        menu.addAction(raw_desc, [raw_string]() { QApplication::clipboard()->setText(raw_string); });

        menu.addAction(QIcon(":/icons/python"), description, [python_command]() { QApplication::clipboard()->setText(python_command); });

        menu.move(dynamic_cast<QWidget*>(sender())->mapToGlobal(pos));
        menu.exec();
    }

    void ModuleDetailsWidget::handle_input_ports_table_menu_requested(const QPoint& pos)
    {
        auto curr_item = m_input_ports_table->itemAt(pos);

        if (!curr_item || curr_item->column() == 1)
            return;

        QMenu menu;
        if(curr_item->column() == 2)
        {
            auto clicked_net = g_netlist->get_net_by_id(curr_item->data(Qt::UserRole).toInt());
            if (!g_netlist->is_global_input_net(clicked_net))
            {
                menu.addAction("Jump to source gate", [this, curr_item]() { handle_input_net_item_clicked(curr_item); });
            }

            menu.addAction(QIcon(":/icons/python"), "Extract net as python code (copy to clipboard)", [this, curr_item]() {
                QApplication::clipboard()->setText("netlist.get_net_by_id(" + curr_item->data(Qt::UserRole).toString() + ")");
            });

            menu.addAction(QIcon(":/icons/python"), "Extract sources as python code (copy to clipboard)", [this, curr_item]() {
                QApplication::clipboard()->setText("netlist.get_net_by_id(" + curr_item->data(Qt::UserRole).toString() + ").get_sources()");
            });
        }
        else
        {
            menu.addAction("Change input port name", [this, curr_item](){
                InputDialog ipd("Change port name", "New port name", curr_item->text());
                if(ipd.exec() == QDialog::Accepted)
                {
                    auto corresponding_net = g_netlist->get_net_by_id(m_input_ports_table->item(curr_item->row(), 2)->data(Qt::UserRole).toInt());
                    if(!corresponding_net)
                        return;
                    g_netlist->get_module_by_id(m_current_id)->set_input_port_name(corresponding_net, ipd.text_value().toStdString());
                    update(m_current_id);
                }
            });
        }

        menu.move(dynamic_cast<QWidget*>(sender())->mapToGlobal(pos));
        menu.exec();
    }

    void ModuleDetailsWidget::handle_output_ports_table_menu_requested(const QPoint& pos)
    {
        auto curr_item = m_output_ports_table->itemAt(pos);
        if (!curr_item || curr_item->column() == 1)
            return;

        QMenu menu;
        if(curr_item->column() == 2)
        {
            auto clicked_net = g_netlist->get_net_by_id(curr_item->data(Qt::UserRole).toInt());
            if (!g_netlist->is_global_output_net(clicked_net))
            {
                menu.addAction("Jump to destination gate", [this, curr_item]() { handle_output_net_item_clicked(curr_item); });
            }
            menu.addAction(QIcon(":/icons/python"), "Extract net as python code (copy to clipboard)", [this, curr_item]() {
                QApplication::clipboard()->setText("netlist.get_net_by_id(" + curr_item->data(Qt::UserRole).toString() + ")");
            });

            menu.addAction(QIcon(":/icons/python"), "Extract destinations as python code (copy to clipboard)", [this, curr_item]() {
                QApplication::clipboard()->setText("netlist.get_net_by_id(" + curr_item->data(Qt::UserRole).toString() + ").get_destinations()");
            });
        }
        else
        {
            menu.addAction("Change output port name", [this, curr_item](){
                InputDialog ipd("Change port name", "New port name", curr_item->text());
                if(ipd.exec() == QDialog::Accepted)
                {
                    auto corresponding_net = g_netlist->get_net_by_id(m_output_ports_table->item(curr_item->row(), 2)->data(Qt::UserRole).toInt());
                    if(!corresponding_net)
                        return;
                    g_netlist->get_module_by_id(m_current_id)->set_output_port_name(corresponding_net, ipd.text_value().toStdString());
                    update(m_current_id);
                }
            });
        }

        menu.move(dynamic_cast<QWidget*>(sender())->mapToGlobal(pos));
        menu.exec();
    }

    void ModuleDetailsWidget::handle_output_net_item_clicked(const QTableWidgetItem* item)
    {
        if (item->column() != 2)
            return;

        int net_id                       = item->data(Qt::UserRole).toInt();
        std::shared_ptr<Net> clicked_net = g_netlist->get_net_by_id(net_id);

        if (!clicked_net)
            return;

        auto destinations = clicked_net->get_destinations();
        if (destinations.empty() || clicked_net->is_global_output_net())
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
            g_selection_relay.m_focus_type = SelectionRelay::item_type::gate;
            g_selection_relay.m_focus_id   = ep.get_gate()->get_id();
            g_selection_relay.m_subfocus   = SelectionRelay::subfocus::left;

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

    void ModuleDetailsWidget::handle_input_net_item_clicked(const QTableWidgetItem* item)
    {
        if (item->column() != 2)
            return;

        auto net = g_netlist->get_net_by_id(item->data(Qt::UserRole).toInt());

        if (!net)
            return;

        auto sources = net->get_sources();

        if (sources.empty() || net->is_global_input_net())
        {
            g_selection_relay.clear();
            g_selection_relay.m_selected_nets.insert(net->get_id());
            g_selection_relay.relay_selection_changed(this);
        }
        else if (sources.size() == 1)
        {
            auto ep = *sources.begin();
            g_selection_relay.clear();
            g_selection_relay.m_selected_gates.insert(ep.get_gate()->get_id());
            g_selection_relay.m_focus_type = SelectionRelay::item_type::gate;
            g_selection_relay.m_focus_id   = ep.get_gate()->get_id();
            g_selection_relay.m_subfocus   = SelectionRelay::subfocus::right;

            auto pins                          = ep.get_gate()->get_output_pins();
            auto index                         = std::distance(pins.begin(), std::find(pins.begin(), pins.end(), ep.get_pin()));
            g_selection_relay.m_subfocus_index = index;

            update(ep.get_gate()->get_id());
            g_selection_relay.relay_selection_changed(this);
        }
        else
        {
            m_navigation_table->setup(hal::node{hal::node_type::gate, 0}, net, false);
            m_navigation_table->move(QCursor::pos());
            m_navigation_table->show();
            m_navigation_table->setFocus();
        }
    }

    void ModuleDetailsWidget::handle_navigation_jump_requested(const node origin, const u32 via_net, const QSet<u32>& to_gates)
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
            g_selection_relay.m_focus_type = SelectionRelay::item_type::gate;
            auto g                         = g_netlist->get_gate_by_id(*to_gates.constBegin());
            g_selection_relay.m_focus_id   = g->get_id();
            g_selection_relay.m_subfocus   = SelectionRelay::subfocus::left;

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
    }

    void ModuleDetailsWidget::show_all_sections()
    {
        for(auto section_btn : m_util_list)
        {
            int index = m_top_lvl_layout->indexOf(section_btn);
            section_btn->show();
            if(section_btn != m_data_fields_button)
            {
                m_top_lvl_layout->itemAt(index+2)->spacerItem()->changeSize(0,7,QSizePolicy::Expanding, QSizePolicy::Fixed);
                m_top_lvl_layout->itemAt(index+2)->spacerItem()->invalidate();
            }
        }
        m_top_lvl_layout->invalidate();
        m_top_lvl_layout->update();
    }

    void ModuleDetailsWidget::hide_empty_sections()
    {
        for(auto section_btn : m_util_list)
        {
            QPushButton* curr_btn = dynamic_cast<QPushButton*>(section_btn);
            if(curr_btn->text().contains("(0)"))
            {
                curr_btn->hide();
                int index = m_top_lvl_layout->indexOf(section_btn);
                if(section_btn != m_data_fields_button)
                {
                    m_top_lvl_layout->itemAt(index+2)->spacerItem()->changeSize(0,0,QSizePolicy::Fixed, QSizePolicy::Fixed);
                    m_top_lvl_layout->itemAt(index+2)->spacerItem()->invalidate();
                }
            }
        }
        m_top_lvl_layout->invalidate();
        m_top_lvl_layout->update();
    }

    void ModuleDetailsWidget::init_settings()
    {
        m_hide_empty_sections = g_settings_manager.get("selection_details/hide_empty_sections", false).toBool();
    }

    void ModuleDetailsWidget::handle_global_settings_changed(void *sender, const QString &key, const QVariant &value)
    {
        Q_UNUSED(sender)
        if(key == "selection_details/hide_empty_sections")
        {
            m_hide_empty_sections = value.toBool();
            if(!m_hide_empty_sections)
                show_all_sections();
            else
                hide_empty_sections();
        }
    }
}    // namespace hal
