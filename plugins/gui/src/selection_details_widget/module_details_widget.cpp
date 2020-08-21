#include "selection_details_widget/module_details_widget.h"

#include "graph_widget/graph_navigation_widget.h"
#include "gui_globals.h"
#include "input_dialog/input_dialog.h"
#include "netlist/gate.h"
#include "netlist/module.h"
#include "netlist/net.h"
#include "selection_details_widget/data_fields_table.h"
#include "selection_details_widget/details_section_widget.h"
#include "selection_details_widget/disputed_big_icon.h"

#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QTableWidget>
#include <QVBoxLayout>

namespace hal
{
    ModuleDetailsWidget::ModuleDetailsWidget(QWidget* parent) : DetailsWidget(DetailsWidget::ModuleDetails, parent)
    {
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

        m_general_info_button = new QPushButton("Module Information", this);
        m_general_info_button->setEnabled(false);

        m_general_table      = new QTableWidget(6, 2);
        m_input_ports_table  = new QTableWidget(0, 3);
        m_output_ports_table = new QTableWidget(0, 3);
        m_dataFieldsTable    = new DataFieldsTable(this);

        m_inputPortsSection  = new DetailsSectionWidget("Input Ports (%1)", m_input_ports_table, this);
        m_outputPortsSection = new DetailsSectionWidget("Output Ports (%1)", m_output_ports_table, this);
        m_dataFieldsSection  = new DetailsSectionWidget("Data Fields (%1)", m_dataFieldsTable, this);

        DetailsSectionWidget::setDefaultTableStyle(m_general_table);

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

        for (const auto& item : tmp_general_table_static_items)
            add_general_table_static_item(item);

        for (const auto& item : tmp_general_table_dynamic_items)
            add_general_table_dynamic_item(item);

        //first 3 items of the general table are interactive, instead of checking id the function above
        //just declare it here
        m_name_item->setFlags(Qt::ItemIsEnabled);
        m_id_item->setFlags(Qt::ItemIsEnabled);
        m_type_item->setFlags(Qt::ItemIsEnabled);

        // place module icon
        QLabel* img = new DisputedBigIcon("sel_module", this);

        intermediate_layout_gt->addWidget(m_general_table);
        intermediate_layout_gt->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
        intermediate_layout_gt->addWidget(img);
        intermediate_layout_gt->setAlignment(img, Qt::AlignTop);

        m_top_lvl_layout->addWidget(m_general_info_button);
        m_top_lvl_layout->addLayout(intermediate_layout_gt);
        m_top_lvl_layout->addSpacerItem(new QSpacerItem(0, 7, QSizePolicy::Expanding, QSizePolicy::Fixed));
        m_top_lvl_layout->addWidget(m_inputPortsSection);
        m_top_lvl_layout->addWidget(m_outputPortsSection);
        m_top_lvl_layout->addWidget(m_dataFieldsSection);

        m_top_lvl_layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
        m_content_layout->addWidget(m_scroll_area);

        //setup the navigation_table ("activated" by clicking on an input / output pin in the 2 tables)
        //delete the table manually so its not necessarry to add a property for the stylesheet(otherwise this table is styled like the others)
        m_navigation_table = new GraphNavigationWidget();
        m_navigation_table->setWindowFlags(Qt::CustomizeWindowHint);
        m_navigation_table->hide_when_focus_lost(true);
        m_navigation_table->hide();

        connect(m_navigation_table, &GraphNavigationWidget::navigation_requested, this, &ModuleDetailsWidget::handle_navigation_jump_requested);

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
        m_currentId = module_id;

        if (m_currentId == 0)
            return;

        auto m = g_netlist->get_module_by_id(module_id);

        if (!m)
            return;

        //update table with general information
        m_name_item->setText(QString::fromStdString(m->get_name()));
        m_id_item->setText(QString::number(m_currentId));
        m_number_of_submodules_item->setText(QString::number(m->get_submodules(nullptr, true).size()));
        m_number_of_nets_item->setText(QString::number(m->get_internal_nets().size()));

        QString type_text = QString::fromStdString(m->get_type());

        if (type_text.isEmpty())
            type_text = "None";

        m_type_item->setText(type_text);

        int total_number_of_gates           = m->get_gates(nullptr, true).size();
        int direct_member_number_of_gates   = m->get_gates(nullptr, false).size();
        int indirect_member_number_of_gates = 0;

        for (auto module : m->get_submodules())
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
        m_inputPortsSection->setRowCount(m->get_input_nets().size());
        m_input_ports_table->setRowCount(m->get_input_nets().size());
        m_input_ports_table->setMaximumHeight(m_input_ports_table->verticalHeader()->length());
        m_input_ports_table->setMinimumHeight(m_input_ports_table->verticalHeader()->length());

        int index = 0;
        for (auto net : m->get_input_nets())
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
        m_outputPortsSection->setRowCount(m->get_output_nets().size());
        m_output_ports_table->setRowCount(m->get_output_nets().size());
        m_output_ports_table->setMaximumHeight(m_output_ports_table->verticalHeader()->length());
        m_output_ports_table->setMinimumHeight(m_output_ports_table->verticalHeader()->length());

        index = 0;
        for (auto net : m->get_output_nets())
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
        m_dataFieldsSection->setRowCount(m->get_data().size());
        m_dataFieldsTable->updateData(module_id, m->get_data());
    }

    void ModuleDetailsWidget::handle_netlist_marked_global_input(Netlist* netlist, u32 associated_data)
    {
        Q_UNUSED(netlist)

        if (m_currentId == 0)
            return;

        auto module = g_netlist->get_module_by_id(m_currentId);
        auto gates  = module->get_gates(nullptr, true);
        auto net    = g_netlist->get_net_by_id(associated_data);

        for (auto gate : gates)
        {
            auto in_nets  = gate->get_fan_in_nets();
            auto out_nets = gate->get_fan_out_nets();

            if (std::find(in_nets.begin(), in_nets.end(), net) != in_nets.end() || std::find(out_nets.begin(), out_nets.end(), net) != out_nets.end())
            {
                update(m_currentId);
                return;
            }
        }
    }

    void ModuleDetailsWidget::handle_netlist_marked_global_output(Netlist* netlist, u32 associated_data)
    {
        Q_UNUSED(netlist)

        if (m_currentId == 0)
            return;

        auto module = g_netlist->get_module_by_id(m_currentId);
        auto gates  = module->get_gates(nullptr, true);
        auto net    = g_netlist->get_net_by_id(associated_data);

        for (auto gate : gates)
        {
            auto in_nets  = gate->get_fan_in_nets();
            auto out_nets = gate->get_fan_out_nets();

            if (std::find(in_nets.begin(), in_nets.end(), net) != in_nets.end() || std::find(out_nets.begin(), out_nets.end(), net) != out_nets.end())
            {
                update(m_currentId);
                return;
            }
        }
    }

    void ModuleDetailsWidget::handle_netlist_marked_global_inout(Netlist* netlist, u32 associated_data)
    {
        Q_UNUSED(netlist)

        if (m_currentId == 0)
            return;

        auto module = g_netlist->get_module_by_id(m_currentId);
        auto gates  = module->get_gates(nullptr, true);
        auto net    = g_netlist->get_net_by_id(associated_data);

        for (auto gate : gates)
        {
            auto in_nets  = gate->get_fan_in_nets();
            auto out_nets = gate->get_fan_out_nets();

            if (std::find(in_nets.begin(), in_nets.end(), net) != in_nets.end() || std::find(out_nets.begin(), out_nets.end(), net) != out_nets.end())
            {
                update(m_currentId);
                return;
            }
        }
    }

    void ModuleDetailsWidget::handle_netlist_unmarked_global_input(Netlist* netlist, u32 associated_data)
    {
        Q_UNUSED(netlist)

        if (m_currentId == 0)
            return;

        auto module = g_netlist->get_module_by_id(m_currentId);
        auto gates  = module->get_gates(nullptr, true);
        auto net    = g_netlist->get_net_by_id(associated_data);

        for (auto gate : gates)
        {
            auto in_nets  = gate->get_fan_in_nets();
            auto out_nets = gate->get_fan_out_nets();

            if (std::find(in_nets.begin(), in_nets.end(), net) != in_nets.end() || std::find(out_nets.begin(), out_nets.end(), net) != out_nets.end())
            {
                update(m_currentId);
                return;
            }
        }
    }

    void ModuleDetailsWidget::handle_netlist_unmarked_global_output(Netlist* netlist, u32 associated_data)
    {
        Q_UNUSED(netlist)

        if (m_currentId == 0)
            return;

        auto module = g_netlist->get_module_by_id(m_currentId);
        auto gates  = module->get_gates(nullptr, true);
        auto net    = g_netlist->get_net_by_id(associated_data);

        for (auto gate : gates)
        {
            auto in_nets  = gate->get_fan_in_nets();
            auto out_nets = gate->get_fan_out_nets();

            if (std::find(in_nets.begin(), in_nets.end(), net) != in_nets.end() || std::find(out_nets.begin(), out_nets.end(), net) != out_nets.end())
            {
                update(m_currentId);
                return;
            }
        }
    }

    void ModuleDetailsWidget::handle_netlist_unmarked_global_inout(Netlist* netlist, u32 associated_data)
    {
        Q_UNUSED(netlist)

        if (m_currentId == 0)
            return;

        auto module = g_netlist->get_module_by_id(m_currentId);
        auto gates  = module->get_gates(nullptr, true);
        auto net    = g_netlist->get_net_by_id(associated_data);

        for (auto gate : gates)
        {
            auto in_nets  = gate->get_fan_in_nets();
            auto out_nets = gate->get_fan_out_nets();

            if (std::find(in_nets.begin(), in_nets.end(), net) != in_nets.end() || std::find(out_nets.begin(), out_nets.end(), net) != out_nets.end())
            {
                update(m_currentId);
                return;
            }
        }
    }

    void ModuleDetailsWidget::handle_module_name_changed(Module* module)
    {
        if (m_currentId == module->get_id())
            update(m_currentId);
    }

    void ModuleDetailsWidget::handle_submodule_added(Module* module, u32 associated_data)
    {
        Q_UNUSED(associated_data);

        if (m_currentId == 0)
            return;

        auto current_module = g_netlist->get_module_by_id(m_currentId);

        if (m_currentId == module->get_id() || current_module->contains_module(module, true))
            update(m_currentId);
    }

    void ModuleDetailsWidget::handle_submodule_removed(Module* module, u32 associated_data)
    {
        Q_UNUSED(associated_data);

        if (m_currentId == 0)
            return;

        auto current_module = g_netlist->get_module_by_id(m_currentId);

        if (m_currentId == module->get_id() || current_module->contains_module(module, true))
            update(m_currentId);
    }

    void ModuleDetailsWidget::handle_module_gate_assigned(Module* module, u32 associated_data)
    {
        Q_UNUSED(associated_data);

        if (m_currentId == 0)
            return;

        auto current_module = g_netlist->get_module_by_id(m_currentId);

        if (m_currentId == module->get_id() || current_module->contains_module(module, true))
            update(m_currentId);
    }

    void ModuleDetailsWidget::handle_module_gate_removed(Module* module, u32 associated_data)
    {
        Q_UNUSED(associated_data);

        if (m_currentId == 0)
            return;

        auto current_module = g_netlist->get_module_by_id(m_currentId);

        if (m_currentId == module->get_id() || current_module->contains_module(module, true))
            update(m_currentId);
    }

    void ModuleDetailsWidget::handle_module_input_port_name_changed(Module* module, u32 associated_data)
    {
        Q_UNUSED(associated_data);

        if (m_currentId == module->get_id())
            update(m_currentId);
    }

    void ModuleDetailsWidget::handle_module_output_port_name_changed(Module* module, u32 associated_data)
    {
        Q_UNUSED(associated_data);

        if (m_currentId == module->get_id())
            update(m_currentId);
    }

    void ModuleDetailsWidget::handle_module_type_changed(Module* module)
    {
        if (m_currentId == module->get_id())
            update(m_currentId);
    }

    void ModuleDetailsWidget::handle_net_name_changed(Net* net)
    {
        if (m_currentId == 0)
            return;

        auto module      = g_netlist->get_module_by_id(m_currentId);
        auto input_nets  = module->get_input_nets();
        auto output_nets = module->get_output_nets();

        if (std::find(input_nets.begin(), input_nets.end(), net) != input_nets.end() || std::find(output_nets.begin(), output_nets.end(), net) != output_nets.end())
            update(m_currentId);
    }

    void ModuleDetailsWidget::handle_net_source_added(Net* net, const u32 src_gate_id)
    {
        Q_UNUSED(net)

        if (m_currentId == 0)
            return;

        auto module = g_netlist->get_module_by_id(m_currentId);
        auto gate   = g_netlist->get_gate_by_id(src_gate_id);

        if (module->contains_gate(gate, true))
            update(m_currentId);
    }

    void ModuleDetailsWidget::handle_net_source_removed(Net* net, const u32 src_gate_id)
    {
        Q_UNUSED(net)

        if (m_currentId == 0)
            return;

        auto module = g_netlist->get_module_by_id(m_currentId);
        auto gate   = g_netlist->get_gate_by_id(src_gate_id);

        if (module->contains_gate(gate, true))
            update(m_currentId);
    }

    void ModuleDetailsWidget::handle_net_destination_added(Net* net, const u32 dst_gate_id)
    {
        Q_UNUSED(net)

        if (m_currentId == 0)
            return;

        auto module = g_netlist->get_module_by_id(m_currentId);
        auto gate   = g_netlist->get_gate_by_id(dst_gate_id);

        if (module->contains_gate(gate, true))
            update(m_currentId);
    }

    void ModuleDetailsWidget::handle_net_destination_removed(Net* net, const u32 dst_gate_id)
    {
        Q_UNUSED(net)

        if (m_currentId == 0)
            return;

        auto module = g_netlist->get_module_by_id(m_currentId);
        auto gate   = g_netlist->get_gate_by_id(dst_gate_id);

        if (module->contains_gate(gate, true))
            update(m_currentId);
    }

    void ModuleDetailsWidget::add_general_table_static_item(QTableWidgetItem* item)
    {
        static int row_index = 0;

        item->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
        item->setFont(m_keyFont);
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

    void ModuleDetailsWidget::handle_general_table_menu_requested(const QPoint& pos)
    {
        auto curr_item = m_general_table->itemAt(pos);

        if (!curr_item || curr_item->column() != 1 || curr_item->row() >= 3)
            return;

        QMenu menu;
        QString description;
        QString python_command = "netlist.get_module_by_id(" + QString::number(m_currentId) + ").";
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

        if (curr_item->row() == 0)
        {
            menu.addAction("Change name", [this, curr_item]() {
                InputDialog ipd("Change name", "New name", curr_item->text());
                if (ipd.exec() == QDialog::Accepted)
                {
                    g_netlist->get_module_by_id(m_currentId)->set_name(ipd.text_value().toStdString());
                    update(m_currentId);
                }
            });
        }

        if (curr_item->row() == 2)
        {
            menu.addAction("Change type", [this, curr_item]() {
                InputDialog ipd("Change type", "New type", curr_item->text());
                if (ipd.exec() == QDialog::Accepted)
                {
                    g_netlist->get_module_by_id(m_currentId)->set_type(ipd.text_value().toStdString());
                    update(m_currentId);
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
        if (curr_item->column() == 2)
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
            menu.addAction("Change input port name", [this, curr_item]() {
                InputDialog ipd("Change port name", "New port name", curr_item->text());
                if (ipd.exec() == QDialog::Accepted)
                {
                    auto corresponding_net = g_netlist->get_net_by_id(m_input_ports_table->item(curr_item->row(), 2)->data(Qt::UserRole).toInt());
                    if (!corresponding_net)
                        return;
                    g_netlist->get_module_by_id(m_currentId)->set_input_port_name(corresponding_net, ipd.text_value().toStdString());
                    update(m_currentId);
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
        if (curr_item->column() == 2)
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
            menu.addAction("Change output port name", [this, curr_item]() {
                InputDialog ipd("Change port name", "New port name", curr_item->text());
                if (ipd.exec() == QDialog::Accepted)
                {
                    auto corresponding_net = g_netlist->get_net_by_id(m_output_ports_table->item(curr_item->row(), 2)->data(Qt::UserRole).toInt());
                    if (!corresponding_net)
                        return;
                    g_netlist->get_module_by_id(m_currentId)->set_output_port_name(corresponding_net, ipd.text_value().toStdString());
                    update(m_currentId);
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

        int net_id       = item->data(Qt::UserRole).toInt();
        Net* clicked_net = g_netlist->get_net_by_id(net_id);

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
}    // namespace hal
