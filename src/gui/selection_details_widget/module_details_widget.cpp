#include "selection_details_widget/module_details_widget.h"

#include "gui_globals.h"

#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/module.h"

#include <QApplication>
#include <QHeaderView>
#include <QScrollArea>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QScrollBar>

namespace hal
{
    ModuleDetailsWidget::ModuleDetailsWidget(QWidget* parent) : QWidget(parent)
    {
        m_current_id = 0;

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

        m_content_layout->setContentsMargins(0,0,0,0);
        m_content_layout->setSpacing(0);
        m_top_lvl_layout->setContentsMargins(0,0,0,0);
        m_top_lvl_layout->setSpacing(0);

        QHBoxLayout *intermediate_layout_gt = new QHBoxLayout();
        intermediate_layout_gt->setContentsMargins(3,3,0,0);
        intermediate_layout_gt->setSpacing(0);
        QHBoxLayout *intermediate_layout_ip = new QHBoxLayout();
        intermediate_layout_ip->setContentsMargins(3,3,0,0);
        intermediate_layout_ip->setSpacing(10);
        QHBoxLayout *intermediate_layout_op = new QHBoxLayout();
        intermediate_layout_op->setContentsMargins(3,3,0,0);
        intermediate_layout_op->setSpacing(0);

        m_general_info_button = new QPushButton("General Information", this);
        m_general_info_button->setEnabled(false);
        m_input_ports_button = new QPushButton("Input Ports", this);
        m_output_ports_button = new QPushButton("Output Ports", this);

        m_general_table = new QTableWidget(6,2);
        m_input_ports_table = new QTableWidget(0,3);
        m_output_ports_table = new QTableWidget(0,3);

        QList<QTableWidget*> tmp_tables = {
            m_general_table,
            m_input_ports_table,
            m_output_ports_table
        };

        QList<QTableWidgetItem*> tmp_general_table_static_items = {
            new QTableWidgetItem("Name:"),
            new QTableWidgetItem("Id:"),
            new QTableWidgetItem("Type:"),
            new QTableWidgetItem("Gates:"),
            new QTableWidgetItem("Submodules:"),
            new QTableWidgetItem("Nets:")
        };

        QList<QTableWidgetItem*> tmp_general_table_dynamic_items = {
            m_name_item = new QTableWidgetItem(),
            m_id_item = new QTableWidgetItem(),
            m_type_item = new QTableWidgetItem(),
            m_number_of_gates_item = new QTableWidgetItem(),
            m_number_of_submodules_item = new QTableWidgetItem(),
            m_number_of_nets_item = new QTableWidgetItem()
        };

        for(const auto &table : tmp_tables)
            style_table(table);

        for(const auto &item : tmp_general_table_static_items)
            add_general_table_static_item(item);

        for(const auto &item : tmp_general_table_dynamic_items)
            add_general_table_dynamic_item(item);

        intermediate_layout_gt->addWidget(m_general_table);
        intermediate_layout_gt->addSpacerItem(new QSpacerItem(0,0, QSizePolicy::Expanding, QSizePolicy::Fixed));
        intermediate_layout_ip->addWidget(m_input_ports_table);
        intermediate_layout_ip->addSpacerItem(new QSpacerItem(0,0, QSizePolicy::Expanding, QSizePolicy::Fixed));
        intermediate_layout_op->addWidget(m_output_ports_table);
        intermediate_layout_op->addSpacerItem(new QSpacerItem(0,0, QSizePolicy::Expanding, QSizePolicy::Fixed));

        m_top_lvl_layout->addWidget(m_general_info_button);
        m_top_lvl_layout->addLayout(intermediate_layout_gt);
        m_top_lvl_layout->addSpacerItem(new QSpacerItem(0, 7, QSizePolicy::Expanding, QSizePolicy::Fixed));
        m_top_lvl_layout->addWidget(m_input_ports_button);
        m_top_lvl_layout->addLayout(intermediate_layout_ip);
        m_top_lvl_layout->addSpacerItem(new QSpacerItem(0, 7, QSizePolicy::Expanding, QSizePolicy::Fixed));
        m_top_lvl_layout->addWidget(m_output_ports_button);
        m_top_lvl_layout->addLayout(intermediate_layout_op);

        m_top_lvl_layout->addSpacerItem(new QSpacerItem(0,0, QSizePolicy::Expanding, QSizePolicy::Expanding));
        m_content_layout->addWidget(m_scroll_area);

        connect(m_general_info_button, &QPushButton::clicked, this, &ModuleDetailsWidget::handle_buttons_clicked);
        connect(m_input_ports_button, &QPushButton::clicked, this, &ModuleDetailsWidget::handle_buttons_clicked);
        connect(m_output_ports_button, &QPushButton::clicked, this, &ModuleDetailsWidget::handle_buttons_clicked);

        connect(&g_NetlistRelay, &NetlistRelay::netlist_marked_global_input, this, &ModuleDetailsWidget::handle_netlist_marked_global_input);
        connect(&g_NetlistRelay, &NetlistRelay::netlist_marked_global_output, this, &ModuleDetailsWidget::handle_netlist_marked_global_output);
        connect(&g_NetlistRelay, &NetlistRelay::netlist_marked_global_inout, this, &ModuleDetailsWidget::handle_netlist_marked_global_inout);
        connect(&g_NetlistRelay, &NetlistRelay::netlist_unmarked_global_input, this, &ModuleDetailsWidget::handle_netlist_unmarked_global_input);
        connect(&g_NetlistRelay, &NetlistRelay::netlist_unmarked_global_output, this, &ModuleDetailsWidget::handle_netlist_unmarked_global_output);
        connect(&g_NetlistRelay, &NetlistRelay::netlist_unmarked_global_inout, this, &ModuleDetailsWidget::handle_netlist_unmarked_global_inout);

        connect(&g_NetlistRelay, &NetlistRelay::module_name_changed, this, &ModuleDetailsWidget::handle_module_name_changed);
        connect(&g_NetlistRelay, &NetlistRelay::module_submodule_added, this, &ModuleDetailsWidget::handle_submodule_added);
        connect(&g_NetlistRelay, &NetlistRelay::module_submodule_removed, this, &ModuleDetailsWidget::handle_submodule_removed);
        connect(&g_NetlistRelay, &NetlistRelay::module_gate_assigned, this, &ModuleDetailsWidget::handle_module_gate_assigned);
        connect(&g_NetlistRelay, &NetlistRelay::module_gate_removed, this, &ModuleDetailsWidget::handle_module_gate_removed);
        connect(&g_NetlistRelay, &NetlistRelay::module_input_port_name_changed, this, &ModuleDetailsWidget::handle_module_input_port_name_changed);
        connect(&g_NetlistRelay, &NetlistRelay::module_output_port_name_changed, this, &ModuleDetailsWidget::handle_module_output_port_name_changed);
        connect(&g_NetlistRelay, &NetlistRelay::module_type_changed, this, &ModuleDetailsWidget::handle_module_type_changed);

        connect(&g_NetlistRelay, &NetlistRelay::net_name_changed, this, &ModuleDetailsWidget::handle_net_name_changed);
        connect(&g_NetlistRelay, &NetlistRelay::net_source_added, this, &ModuleDetailsWidget::handle_net_source_added);
        connect(&g_NetlistRelay, &NetlistRelay::net_source_removed, this, &ModuleDetailsWidget::handle_net_source_removed);
        connect(&g_NetlistRelay, &NetlistRelay::net_destination_added, this, &ModuleDetailsWidget::handle_net_destination_added);
        connect(&g_NetlistRelay, &NetlistRelay::net_destination_removed, this, &ModuleDetailsWidget::handle_net_destination_removed);
    }

    void ModuleDetailsWidget::update(const u32 module_id)
    {
        m_current_id = module_id;

        if(m_current_id == 0)
            return;

        auto m = g_netlist->get_module_by_id(module_id);

        if(!m)
            return;


        //update table with general information
        m_name_item->setText(QString::fromStdString(m->get_name()));
        m_id_item->setText(QString::number(m_current_id));
        m_number_of_submodules_item->setText(QString::number(m->get_submodules(nullptr, true).size()));
        m_number_of_nets_item->setText(QString::number(m->get_internal_nets().size()));

        QString type_text = QString::fromStdString(m->get_type());

        if(type_text.isEmpty())
            type_text = "None";

        m_type_item->setText(type_text);

        int total_number_of_gates = m->get_gates(nullptr, true).size();
        int direct_member_number_of_gates = m->get_gates(nullptr, false).size();
        int indirect_member_number_of_gates = 0;

        for(const auto &module : m->get_submodules())
            indirect_member_number_of_gates += module->get_gates(nullptr, true).size();

        QString number_of_gates_text = QString::number(total_number_of_gates);

        if(indirect_member_number_of_gates > 0)
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
        for(const auto &net : m->get_input_nets())
        {
            QTableWidgetItem* port_name = new QTableWidgetItem(QString::fromStdString(m->get_input_port_name(net)));
            QTableWidgetItem* arrow_item = new QTableWidgetItem(QChar(0x2b05));
            QTableWidgetItem* net_item = new QTableWidgetItem(QString::fromStdString(net->get_name()));

            arrow_item->setForeground(QBrush(QColor(114, 140, 0), Qt::SolidPattern));
            port_name->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
            arrow_item->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
            net_item->setFlags(Qt::ItemIsEnabled);

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
        for(const auto &net : m->get_output_nets())
        {
            QTableWidgetItem* port_name = new QTableWidgetItem(QString::fromStdString(m->get_output_port_name(net)));
            QTableWidgetItem* arrow_item = new QTableWidgetItem(QChar(0x27a1));
            QTableWidgetItem* net_item = new QTableWidgetItem(QString::fromStdString(net->get_name()));

            arrow_item->setForeground(QBrush(QColor(114, 140, 0), Qt::SolidPattern));
            port_name->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
            arrow_item->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
            net_item->setFlags(Qt::ItemIsEnabled);

            m_output_ports_table->setItem(index, 0, port_name);
            m_output_ports_table->setItem(index, 1, arrow_item);
            m_output_ports_table->setItem(index, 2, net_item);

            index++;
        }

        m_output_ports_table->resizeColumnsToContents();
        m_output_ports_table->setFixedWidth(calculate_table_size(m_output_ports_table).width());
    }

    void ModuleDetailsWidget::handle_netlist_marked_global_input(std::shared_ptr<Netlist> netlist, u32 associated_data)
    {
        Q_UNUSED(netlist)

        if(m_current_id == 0)
            return;

        auto module = g_netlist->get_module_by_id(m_current_id);
        auto gates = module->get_gates(nullptr, true);
        auto net = g_netlist->get_net_by_id(associated_data);

        for(const auto& gate : gates)
        {
            auto in_nets = gate->get_fan_in_nets();
            auto out_nets = gate->get_fan_out_nets();

            if(in_nets.find(net) != in_nets.end() || out_nets.find(net) != out_nets.end())
            {
                update(m_current_id);
                return;
            }
        }
    }

    void ModuleDetailsWidget::handle_netlist_marked_global_output(std::shared_ptr<Netlist> netlist, u32 associated_data)
    {
        Q_UNUSED(netlist)

        if(m_current_id == 0)
            return;

        auto module = g_netlist->get_module_by_id(m_current_id);
        auto gates = module->get_gates(nullptr, true);
        auto net = g_netlist->get_net_by_id(associated_data);

        for(const auto& gate : gates)
        {
            auto in_nets = gate->get_fan_in_nets();
            auto out_nets = gate->get_fan_out_nets();

            if(in_nets.find(net) != in_nets.end() || out_nets.find(net) != out_nets.end())
            {
                update(m_current_id);
                return;
            }
        }
    }

    void ModuleDetailsWidget::handle_netlist_marked_global_inout(std::shared_ptr<Netlist> netlist, u32 associated_data)
    {
        Q_UNUSED(netlist)

        if(m_current_id == 0)
            return;

        auto module = g_netlist->get_module_by_id(m_current_id);
        auto gates = module->get_gates(nullptr, true);
        auto net = g_netlist->get_net_by_id(associated_data);

        for(const auto& gate : gates)
        {
            auto in_nets = gate->get_fan_in_nets();
            auto out_nets = gate->get_fan_out_nets();

            if(in_nets.find(net) != in_nets.end() || out_nets.find(net) != out_nets.end())
            {
                update(m_current_id);
                return;
            }
        }
    }

    void ModuleDetailsWidget::handle_netlist_unmarked_global_input(std::shared_ptr<Netlist> netlist, u32 associated_data)
    {
        Q_UNUSED(netlist)

        if(m_current_id == 0)
            return;

        auto module = g_netlist->get_module_by_id(m_current_id);
        auto gates = module->get_gates(nullptr, true);
        auto net = g_netlist->get_net_by_id(associated_data);

        for(const auto& gate : gates)
        {
            auto in_nets = gate->get_fan_in_nets();
            auto out_nets = gate->get_fan_out_nets();

            if(in_nets.find(net) != in_nets.end() || out_nets.find(net) != out_nets.end())
            {
                update(m_current_id);
                return;
            }
        }
    }

    void ModuleDetailsWidget::handle_netlist_unmarked_global_output(std::shared_ptr<Netlist> netlist, u32 associated_data)
    {
        Q_UNUSED(netlist)

        if(m_current_id == 0)
            return;

        auto module = g_netlist->get_module_by_id(m_current_id);
        auto gates = module->get_gates(nullptr, true);
        auto net = g_netlist->get_net_by_id(associated_data);

        for(const auto& gate : gates)
        {
            auto in_nets = gate->get_fan_in_nets();
            auto out_nets = gate->get_fan_out_nets();

            if(in_nets.find(net) != in_nets.end() || out_nets.find(net) != out_nets.end())
            {
                update(m_current_id);
                return;
            }
        }
    }

    void ModuleDetailsWidget::handle_netlist_unmarked_global_inout(std::shared_ptr<Netlist> netlist, u32 associated_data)
    {
        Q_UNUSED(netlist)

        if(m_current_id == 0)
            return;

        auto module = g_netlist->get_module_by_id(m_current_id);
        auto gates = module->get_gates(nullptr, true);
        auto net = g_netlist->get_net_by_id(associated_data);

        for(const auto& gate : gates)
        {
            auto in_nets = gate->get_fan_in_nets();
            auto out_nets = gate->get_fan_out_nets();

            if(in_nets.find(net) != in_nets.end() || out_nets.find(net) != out_nets.end())
            {
                update(m_current_id);
                return;
            }
        }
    }

    void ModuleDetailsWidget::handle_module_name_changed(std::shared_ptr<Module> module)
    {
        if(m_current_id == module->get_id())
            update(m_current_id);
    }

    void ModuleDetailsWidget::handle_submodule_added(std::shared_ptr<Module> module, u32 associated_data)
    {
        Q_UNUSED(associated_data);

        if(m_current_id == 0)
            return;

        auto current_module = g_netlist->get_module_by_id(m_current_id);

        if(m_current_id == module->get_id() || current_module->contains_module(module, true))
            update(m_current_id);
    }

    void ModuleDetailsWidget::handle_submodule_removed(std::shared_ptr<Module> module, u32 associated_data)
    {
        Q_UNUSED(associated_data);

        if(m_current_id == 0)
            return;

        auto current_module = g_netlist->get_module_by_id(m_current_id);

        if(m_current_id == module->get_id() || current_module->contains_module(module, true))
            update(m_current_id);
    }

    void ModuleDetailsWidget::handle_module_gate_assigned(std::shared_ptr<Module> module, u32 associated_data)
    {
        Q_UNUSED(associated_data);

        if(m_current_id == 0)
            return;

        auto current_module = g_netlist->get_module_by_id(m_current_id);

        if(m_current_id == module->get_id() || current_module->contains_module(module, true))
            update(m_current_id);
    }

    void ModuleDetailsWidget::handle_module_gate_removed(std::shared_ptr<Module> module, u32 associated_data)
    {
        Q_UNUSED(associated_data);

        if(m_current_id == 0)
            return;

        auto current_module = g_netlist->get_module_by_id(m_current_id);

        if(m_current_id == module->get_id() || current_module->contains_module(module, true))
            update(m_current_id);
    }

    void ModuleDetailsWidget::handle_module_input_port_name_changed(std::shared_ptr<Module> module, u32 associated_data)
    {
        Q_UNUSED(associated_data);

        if(m_current_id == module->get_id())
            update(m_current_id);
    }

    void ModuleDetailsWidget::handle_module_output_port_name_changed(std::shared_ptr<Module> module, u32 associated_data)
    {
        Q_UNUSED(associated_data);

        if(m_current_id == module->get_id())
            update(m_current_id);
    }

    void ModuleDetailsWidget::handle_module_type_changed(std::shared_ptr<Module> module)
    {
        if(m_current_id == module->get_id())
            update(m_current_id);
    }

    void ModuleDetailsWidget::handle_net_name_changed(std::shared_ptr<Net> net)
    {
        if(m_current_id == 0)
            return;

        auto module = g_netlist->get_module_by_id(m_current_id);
        auto input_nets = module->get_input_nets();
        auto output_nets = module->get_output_nets();

        if(input_nets.find(net) != input_nets.end() || output_nets.find(net) != output_nets.end())
            update(m_current_id);
    }

    void ModuleDetailsWidget::handle_net_source_added(std::shared_ptr<Net> net, const u32 src_gate_id)
    {
        Q_UNUSED(net)

        if(m_current_id == 0)
            return;

        auto module = g_netlist->get_module_by_id(m_current_id);
        auto gate = g_netlist->get_gate_by_id(src_gate_id);

        if(module->contains_gate(gate, true))
            update(m_current_id);
    }

    void ModuleDetailsWidget::handle_net_source_removed(std::shared_ptr<Net> net, const u32 src_gate_id)
    {
        Q_UNUSED(net)

        if(m_current_id == 0)
            return;

        auto module = g_netlist->get_module_by_id(m_current_id);
        auto gate = g_netlist->get_gate_by_id(src_gate_id);

        if(module->contains_gate(gate, true))
            update(m_current_id);
    }

    void ModuleDetailsWidget::handle_net_destination_added(std::shared_ptr<Net> net, const u32 dst_gate_id)
    {
        Q_UNUSED(net)

        if(m_current_id == 0)
            return;

        auto module = g_netlist->get_module_by_id(m_current_id);
        auto gate = g_netlist->get_gate_by_id(dst_gate_id);

        if(module->contains_gate(gate, true))
            update(m_current_id);
    }

    void ModuleDetailsWidget::handle_net_destination_removed(std::shared_ptr<Net> net, const u32 dst_gate_id)
    {
        Q_UNUSED(net)

        if(m_current_id == 0)
            return;

        auto module = g_netlist->get_module_by_id(m_current_id);
        auto gate = g_netlist->get_gate_by_id(dst_gate_id);

        if(module->contains_gate(gate, true))
            update(m_current_id);
    }

    void ModuleDetailsWidget::handle_buttons_clicked()
    {
        QPushButton* btn = dynamic_cast<QPushButton*>(sender());

        if(!btn)
            return;

        int index = m_top_lvl_layout->indexOf(btn);

        QWidget* widget;
        widget = m_top_lvl_layout->itemAt(index+1)->layout()->itemAt(0)->widget();

        if(!widget)
            return;

        if(widget->isHidden())
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

    QSize ModuleDetailsWidget::calculate_table_size(QTableWidget *table)
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
    }
}
