#include "gui/selection_details_widget/net_details_widget.h"
#include "gui/selection_details_widget/data_fields_table.h"
#include "gui/selection_details_widget/disputed_big_icon.h"
#include "gui/selection_details_widget/details_section_widget.h"
#include "gui/selection_details_widget/details_general_model.h"
#include "gui/selection_details_widget/details_table_utilities.h"

#include "gui/gui_globals.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "gui/input_dialog/input_dialog.h"

#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QMenu>
#include <QMouseEvent>
#include <QPushButton>
#include <QScrollArea>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace hal
{
    NetDetailsWidget::NetDetailsWidget(QWidget* parent) : DetailsWidget(DetailsWidget::NetDetails, parent)
    {
        //general initializations

        m_scroll_area       = new QScrollArea();
        m_top_lvl_container = new QWidget();
        m_top_lvl_layout    = new QVBoxLayout(m_top_lvl_container);
        m_top_lvl_container->setLayout(m_top_lvl_layout);
        m_scroll_area->setWidget(m_top_lvl_container);
        m_scroll_area->setWidgetResizable(true);
        m_content_layout = new QVBoxLayout(this);

        //layout customization
        m_content_layout->setContentsMargins(0, 0, 0, 0);
        m_content_layout->setSpacing(0);
        m_top_lvl_layout->setContentsMargins(0, 0, 0, 0);
        m_top_lvl_layout->setSpacing(0);

        //intermediate layout for the 3 sections (to add left spacing)
        QHBoxLayout* intermediate_layout_gt = new QHBoxLayout();
        intermediate_layout_gt->setContentsMargins(3, 3, 0, 0);
        intermediate_layout_gt->setSpacing(0);

        //buttons
        m_general_info_button = new QPushButton("Net Information", this);
        m_general_info_button->setEnabled(false);

        //table initializations
        mGeneralView        = new QTableView(this);
        mGeneralModel       = new DetailsGeneralModel(mGeneralView);
        mGeneralModel->setDummyContent<Net>();
        mGeneralView->setModel(mGeneralModel);
        m_source_pins_table      = new QTableWidget(0, 3);
        m_destination_pins_table = new QTableWidget(0, 3);
        m_dataFieldsTable        = new DataFieldsTable(this);

        m_sourcePinsSection      = new DetailsSectionWidget("Source Pins (%1)", m_source_pins_table, this);
        m_destinationPinsSection = new DetailsSectionWidget("Destination Pins (%1)", m_destination_pins_table, this);
        m_dataFieldsSection      = new DetailsSectionWidget("Data Fields (%1)", m_dataFieldsTable, this);

        DetailsTableUtilities::setDefaultTableStyle(mGeneralView);
        mGeneralView->setSelectionBehavior(QAbstractItemView::SelectRows);
        mGeneralView->setSelectionMode(QAbstractItemView::SingleSelection);

        // place net icon
        QLabel* img = new DisputedBigIcon("sel_net", this);

        //adding things to intermediate layout (the one thats neccessary for the left spacing)
        intermediate_layout_gt->addWidget(mGeneralView);
        intermediate_layout_gt->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
        intermediate_layout_gt->addWidget(img);
        intermediate_layout_gt->setAlignment(img,Qt::AlignTop);

        //adding things to the main layout
        m_top_lvl_layout->addWidget(m_general_info_button);
        m_top_lvl_layout->addLayout(intermediate_layout_gt);
        m_top_lvl_layout->addSpacerItem(new QSpacerItem(0, 7, QSizePolicy::Expanding, QSizePolicy::Fixed));
        m_top_lvl_layout->addWidget(m_sourcePinsSection);
        m_top_lvl_layout->addWidget(m_destinationPinsSection);
        m_top_lvl_layout->addWidget(m_dataFieldsSection);

        //necessary to add at the end
        m_top_lvl_layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
        m_content_layout->addWidget(m_scroll_area);

        //connect the tables
        connect(m_source_pins_table, &QTableWidget::itemDoubleClicked, this, &NetDetailsWidget::handle_table_item_clicked);
        connect(m_destination_pins_table, &QTableWidget::itemDoubleClicked, this, &NetDetailsWidget::handle_table_item_clicked);
        connect(m_source_pins_table, &QTableWidget::customContextMenuRequested, this, &NetDetailsWidget::handle_sources_table_menu_requeted);
        connect(m_destination_pins_table, &QTableWidget::customContextMenuRequested, this, &NetDetailsWidget::handle_destinations_table_menu_requeted);

        //NetlistRelay connections
        connect(g_netlist_relay, &NetlistRelay::net_removed, this, &NetDetailsWidget::handle_net_removed);
        connect(g_netlist_relay, &NetlistRelay::net_name_changed, this, &NetDetailsWidget::handle_net_name_changed);
        connect(g_netlist_relay, &NetlistRelay::net_source_added, this, &NetDetailsWidget::handle_net_source_added);
        connect(g_netlist_relay, &NetlistRelay::net_source_removed, this, &NetDetailsWidget::handle_net_source_removed);
        connect(g_netlist_relay, &NetlistRelay::net_destination_added, this, &NetDetailsWidget::handle_net_destination_added);
        connect(g_netlist_relay, &NetlistRelay::net_destination_removed, this, &NetDetailsWidget::handle_net_destination_removed);
        connect(g_netlist_relay, &NetlistRelay::gate_name_changed, this, &NetDetailsWidget::handle_gate_name_changed);
        connect(mGeneralModel, &DetailsGeneralModel::requireUpdate, this, &NetDetailsWidget::update);
    }

    NetDetailsWidget::~NetDetailsWidget()
    {
    }

    void NetDetailsWidget::update(u32 net_id)
    {
        m_currentId = net_id;
        auto n       = g_netlist->get_net_by_id(net_id);

        if (m_currentId == 0 || !n)
            return;

        mGeneralModel->setContent<Net>(n);

        mGeneralView->resizeColumnsToContents();
        mGeneralView->setFixedSize(DetailsTableUtilities::tableViewSize(mGeneralView,mGeneralModel->rowCount(),mGeneralModel->columnCount()));

        // (2) update sources section
        m_source_pins_table->clearContents();
        m_sourcePinsSection->setRowCount(n->get_sources().size());
        m_source_pins_table->setRowCount(n->get_sources().size());
        m_source_pins_table->setMaximumHeight(m_source_pins_table->verticalHeader()->length());
        int index = 0;
        if (!g_netlist->is_global_input_net(n))
        {
            for (const auto& ep_source : n->get_sources())
            {
                QTableWidgetItem* pin_name       = new QTableWidgetItem(QString::fromStdString(ep_source->get_pin()));
                QTableWidgetItem* arrow_item     = new QTableWidgetItem(QChar(0x2b05));
                QTableWidgetItem* gate_name_item = new QTableWidgetItem(QString::fromStdString(ep_source->get_gate()->get_name()));
                arrow_item->setForeground(QBrush(QColor(114, 140, 0), Qt::SolidPattern));
                pin_name->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
                arrow_item->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
                //arrow_item->setFlags((Qt::ItemFlag)(~Qt::ItemIsSelectable));
                gate_name_item->setFlags(Qt::ItemIsEnabled);
                gate_name_item->setData(Qt::UserRole, ep_source->get_gate()->get_id());

                m_source_pins_table->setItem(index, 0, pin_name);
                m_source_pins_table->setItem(index, 1, arrow_item);
                m_source_pins_table->setItem(index, 2, gate_name_item);
                index++;
            }
        }

        m_source_pins_table->resizeColumnsToContents();
        m_source_pins_table->setFixedWidth(DetailsTableUtilities::tableWidgetSize(m_source_pins_table).width());

        // (3) update destinations section
        m_destination_pins_table->clearContents();
        m_destinationPinsSection->setRowCount(n->get_destinations().size());
        m_destination_pins_table->setRowCount(n->get_destinations().size());
        m_destination_pins_table->setMaximumHeight(m_destination_pins_table->verticalHeader()->length());
        index = 0;
        if (!g_netlist->is_global_output_net(n))
        {
            for (const auto& ep_destination : n->get_destinations())
            {
                QTableWidgetItem* pin_name       = new QTableWidgetItem(QString::fromStdString(ep_destination->get_pin()));
                QTableWidgetItem* arrow_item     = new QTableWidgetItem(QChar(0x27a1));
                QTableWidgetItem* gate_name_item = new QTableWidgetItem(QString::fromStdString(ep_destination->get_gate()->get_name()));
                arrow_item->setForeground(QBrush(QColor(114, 140, 0), Qt::SolidPattern));
                pin_name->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
                arrow_item->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
                //arrow_item->setFlags((Qt::ItemFlag)(~Qt::ItemIsSelectable));
                gate_name_item->setFlags(Qt::ItemIsEnabled);
                gate_name_item->setData(Qt::UserRole, ep_destination->get_gate()->get_id());

                m_destination_pins_table->setItem(index, 0, pin_name);
                m_destination_pins_table->setItem(index, 1, arrow_item);
                m_destination_pins_table->setItem(index, 2, gate_name_item);
                index++;
            }
        }
        m_destination_pins_table->resizeColumnsToContents();
        m_destination_pins_table->setFixedWidth(DetailsTableUtilities::tableWidgetSize(m_destination_pins_table).width());

        m_dataFieldsSection->setRowCount(n->get_data().size());
        m_dataFieldsTable->updateData(net_id,  n->get_data());

        //to prevent any updating(render) erros that can occur, manually tell the tables to update
        mGeneralView->update();
        m_source_pins_table->update();
        m_destination_pins_table->update();
        m_dataFieldsTable->update();
    }

    void NetDetailsWidget::handle_net_removed(Net* n)
    {
        Q_UNUSED(n)
    }

    void NetDetailsWidget::handle_net_name_changed(Net* n)
    {
        mGeneralModel->setContent<Net>(n);
    }

    void NetDetailsWidget::handle_net_source_added(Net* n, const u32 src_gate_id)
    {
        Q_UNUSED(src_gate_id);

        if (m_currentId == n->get_id())
            update(m_currentId);
    }

    void NetDetailsWidget::handle_net_source_removed(Net* n, const u32 src_gate_id)
    {
        Q_UNUSED(src_gate_id);

        if (m_currentId == n->get_id())
            update(m_currentId);
    }

    void NetDetailsWidget::handle_net_destination_added(Net* n, const u32 dst_gate_id)
    {
        Q_UNUSED(dst_gate_id);

        if (m_currentId == n->get_id())
            update(m_currentId);
    }

    void NetDetailsWidget::handle_net_destination_removed(Net* n, const u32 dst_gate_id)
    {
        Q_UNUSED(dst_gate_id);

        if (m_currentId == n->get_id())
            update(m_currentId);
    }

    void NetDetailsWidget::handle_gate_name_changed(Gate* g)
    {
        Q_UNUSED(g)

        if (m_currentId == 0)
            return;

        bool update_needed = false;

        //current net
        auto n = g_netlist->get_net_by_id(m_currentId);

        //check if current net is in netlist (m_currentId is unassigned if netlist details widget hasn't been shown once)
        if (!g_netlist->is_net_in_netlist(n))
            return;

        //check if renamed gate is a src of the currently shown net
        for (auto e : n->get_sources())
        {
            if (e->get_gate()->get_id() == m_currentId)
            {
                update_needed = true;
                break;
            }
        }

        //check if renamed gate is a dst of the currently shown net
        if (!update_needed)
        {
            for (auto e : n->get_destinations())
            {
                if (e->get_gate()->get_id() == m_currentId)
                {
                    update_needed = true;
                    break;
                }
            }
        }

        if (update_needed)
            update(m_currentId);
    }

    void NetDetailsWidget::handle_table_item_clicked(QTableWidgetItem* item)
    {
        if (item->column() != 2)
            return;

        QTableWidget* sender_table = dynamic_cast<QTableWidget*>(sender());

        SelectionRelay::subfocus focus = (sender_table == m_source_pins_table) ? SelectionRelay::subfocus::right : SelectionRelay::subfocus::left;
        auto gate_id                   = item->data(Qt::UserRole).toInt();
        auto pin                       = sender_table->item(item->row(), 0)->text().toStdString();

        auto clicked_gate = g_netlist->get_gate_by_id(gate_id);
        if (!clicked_gate)
            return;

        g_selection_relay->clear();
        g_selection_relay->m_selected_gates.insert(gate_id);
        g_selection_relay->m_focus_type = SelectionRelay::item_type::gate;
        g_selection_relay->m_focus_id   = gate_id;
        g_selection_relay->m_subfocus   = focus;

        auto pins                          = (sender_table == m_source_pins_table) ? clicked_gate->get_output_pins() : clicked_gate->get_input_pins();
        auto index                         = std::distance(pins.begin(), std::find(pins.begin(), pins.end(), pin));
        g_selection_relay->m_subfocus_index = index;

        g_selection_relay->relay_selection_changed(this);
    }

    void NetDetailsWidget::handle_sources_table_menu_requeted(const QPoint& pos)
    {
        if (!m_source_pins_table->itemAt(pos) || m_source_pins_table->itemAt(pos)->column() != 2)
            return;

        QMenu menu;
        menu.addAction("Jump to source gate", [this, pos]() { handle_table_item_clicked(m_source_pins_table->itemAt(pos)); });
        menu.addAction(QIcon(":/icons/python"), "Extract gate as python code (copy to clipboard)", [this, pos]() {
            QApplication::clipboard()->setText("netlist.get_gate_by_id(" + m_source_pins_table->itemAt(pos)->data(Qt::UserRole).toString() + ")");
        });

        menu.move(dynamic_cast<QWidget*>(sender())->mapToGlobal(pos));
        menu.exec();
    }

    void NetDetailsWidget::handle_destinations_table_menu_requeted(const QPoint& pos)
    {
        if (!m_destination_pins_table->itemAt(pos) || m_destination_pins_table->itemAt(pos)->column() != 2)
            return;

        QMenu menu;
        menu.addAction("Jump to destination gate", [this, pos]() { handle_table_item_clicked(m_destination_pins_table->itemAt(pos)); });
        menu.addAction(QIcon(":/icons/python"), "Extract gate as python code (copy to clipboard)", [this, pos]() {
            QApplication::clipboard()->setText("netlist.get_gate_by_id(" + m_destination_pins_table->itemAt(pos)->data(Qt::UserRole).toString() + ")");
        });

        menu.move(dynamic_cast<QWidget*>(sender())->mapToGlobal(pos));
        menu.exec();
    }

}    // namespace hal
