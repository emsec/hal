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

        mScrollArea       = new QScrollArea();
        mTopLvlContainer = new QWidget();
        mTopLvlLayout    = new QVBoxLayout(mTopLvlContainer);
        mTopLvlContainer->setLayout(mTopLvlLayout);
        mScrollArea->setWidget(mTopLvlContainer);
        mScrollArea->setWidgetResizable(true);
        mContentLayout = new QVBoxLayout(this);

        //layout customization
        mContentLayout->setContentsMargins(0, 0, 0, 0);
        mContentLayout->setSpacing(0);
        mTopLvlLayout->setContentsMargins(0, 0, 0, 0);
        mTopLvlLayout->setSpacing(0);

        //intermediate layout for the 3 sections (to add left spacing)
        QHBoxLayout* intermediate_layout_gt = new QHBoxLayout();
        intermediate_layout_gt->setContentsMargins(3, 3, 0, 0);
        intermediate_layout_gt->setSpacing(0);

        //buttons
        mGeneralInfoButton = new QPushButton("Net Information", this);
        mGeneralInfoButton->setEnabled(false);

        //table initializations
        mGeneralView        = new QTableView(this);
        mGeneralModel       = new DetailsGeneralModel(mGeneralView);
        mGeneralModel->setDummyContent<Net>();
        mGeneralView->setModel(mGeneralModel);
        mSourcePinsTable      = new QTableWidget(0, 3);
        mDestinationPinsTable = new QTableWidget(0, 3);
        mDataFieldsTable        = new DataFieldsTable(this);

        mSourcePinsSection      = new DetailsSectionWidget("Source Pins (%1)", mSourcePinsTable, this);
        mDestinationPinsSection = new DetailsSectionWidget("Destination Pins (%1)", mDestinationPinsTable, this);
        mDataFieldsSection      = new DetailsSectionWidget("Data Fields (%1)", mDataFieldsTable, this);

        DetailsTableUtilities::setDefaultTableStyle(mGeneralView);
        mGeneralView->setSelectionBehavior(QAbstractItemView::SelectRows);
        mGeneralView->setSelectionMode(QAbstractItemView::SingleSelection);

        // place net icon
        mBigIcon = new DisputedBigIcon("sel_net", this);

        //adding things to intermediate layout (the one thats neccessary for the left spacing)
        intermediate_layout_gt->addWidget(mGeneralView);
        intermediate_layout_gt->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
        intermediate_layout_gt->addWidget(mBigIcon);
        intermediate_layout_gt->setAlignment(mBigIcon,Qt::AlignTop);

        //adding things to the main layout
        mTopLvlLayout->addWidget(mGeneralInfoButton);
        mTopLvlLayout->addLayout(intermediate_layout_gt);
        mTopLvlLayout->addSpacerItem(new QSpacerItem(0, 7, QSizePolicy::Expanding, QSizePolicy::Fixed));
        mTopLvlLayout->addWidget(mSourcePinsSection);
        mTopLvlLayout->addWidget(mDestinationPinsSection);
        mTopLvlLayout->addWidget(mDataFieldsSection);

        //necessary to add at the end
        mTopLvlLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
        mContentLayout->addWidget(mScrollArea);

        //connect the tables
        connect(mSourcePinsTable, &QTableWidget::itemDoubleClicked, this, &NetDetailsWidget::handleTableItemClicked);
        connect(mDestinationPinsTable, &QTableWidget::itemDoubleClicked, this, &NetDetailsWidget::handleTableItemClicked);
        connect(mSourcePinsTable, &QTableWidget::customContextMenuRequested, this, &NetDetailsWidget::handleSourcesTableMenuRequeted);
        connect(mDestinationPinsTable, &QTableWidget::customContextMenuRequested, this, &NetDetailsWidget::handleDestinationsTableMenuRequeted);

        //NetlistRelay connections
        connect(gNetlistRelay, &NetlistRelay::netRemoved, this, &NetDetailsWidget::handleNetRemoved);
        connect(gNetlistRelay, &NetlistRelay::netNameChanged, this, &NetDetailsWidget::handleNetNameChanged);
        connect(gNetlistRelay, &NetlistRelay::netSourceAdded, this, &NetDetailsWidget::handleNetSourceAdded);
        connect(gNetlistRelay, &NetlistRelay::netSourceRemoved, this, &NetDetailsWidget::handleNetSourceRemoved);
        connect(gNetlistRelay, &NetlistRelay::netDestinationAdded, this, &NetDetailsWidget::handleNetDestinationAdded);
        connect(gNetlistRelay, &NetlistRelay::netDestinationRemoved, this, &NetDetailsWidget::handleNetDestinationRemoved);
        connect(gNetlistRelay, &NetlistRelay::gateNameChanged, this, &NetDetailsWidget::handleGateNameChanged);
        connect(mGeneralModel, &DetailsGeneralModel::requireUpdate, this, &NetDetailsWidget::update);
        gSelectionRelay->registerSender(this, "SelectionDetailsWidget");
    }

    NetDetailsWidget::~NetDetailsWidget()
    {
    }

    void NetDetailsWidget::update(u32 mNetId)
    {
        mCurrentId = mNetId;
        auto n       = gNetlist->get_net_by_id(mNetId);

        if (mCurrentId == 0 || !n)
            return;

        mGeneralModel->setContent<Net>(n);

        mGeneralView->resizeColumnsToContents();
        mGeneralView->setFixedSize(DetailsTableUtilities::tableViewSize(mGeneralView,mGeneralModel->rowCount(),mGeneralModel->columnCount()));

        // (2) update sources section
        mSourcePinsTable->clearContents();
        mSourcePinsSection->setRowCount(n->get_sources().size());
        mSourcePinsTable->setRowCount(n->get_sources().size());
        mSourcePinsTable->setMaximumHeight(mSourcePinsTable->verticalHeader()->length());
        mSourcePinsTable->setMinimumHeight(mSourcePinsTable->verticalHeader()->length());
        int index = 0;
        if (!gNetlist->is_global_input_net(n))
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

                mSourcePinsTable->setItem(index, 0, pin_name);
                mSourcePinsTable->setItem(index, 1, arrow_item);
                mSourcePinsTable->setItem(index, 2, gate_name_item);
                index++;
            }
        }

        mSourcePinsTable->resizeColumnsToContents();
        mSourcePinsTable->setFixedWidth(DetailsTableUtilities::tableWidgetSize(mSourcePinsTable).width());

        // (3) update destinations section
        mDestinationPinsTable->clearContents();
        mDestinationPinsSection->setRowCount(n->get_destinations().size());
        mDestinationPinsTable->setRowCount(n->get_destinations().size());
        mDestinationPinsTable->setMaximumHeight(mDestinationPinsTable->verticalHeader()->length());
        mDestinationPinsTable->setMinimumHeight(mDestinationPinsTable->verticalHeader()->length());
        index = 0;
        if (!gNetlist->is_global_output_net(n))
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

                mDestinationPinsTable->setItem(index, 0, pin_name);
                mDestinationPinsTable->setItem(index, 1, arrow_item);
                mDestinationPinsTable->setItem(index, 2, gate_name_item);
                index++;
            }
        }
        mDestinationPinsTable->resizeColumnsToContents();
        mDestinationPinsTable->setFixedWidth(DetailsTableUtilities::tableWidgetSize(mDestinationPinsTable).width());

        mDataFieldsSection->setRowCount(n->get_data_map().size());
        mDataFieldsTable->updateData(mNetId,  n->get_data_map());

        //to prevent any updating(render) erros that can occur, manually tell the tables to update
        mGeneralView->update();
        mSourcePinsTable->update();
        mDestinationPinsTable->update();
        mDataFieldsTable->update();
    }

    void NetDetailsWidget::handleNetRemoved(Net* n)
    {
        if (mCurrentId == n->get_id())
        {
            mGeneralView->setHidden(true);
            mScrollArea->setHidden(true);
        }
    }

    void NetDetailsWidget::handleNetNameChanged(Net* n)
    {
        mGeneralModel->setContent<Net>(n);
    }

    void NetDetailsWidget::handleNetSourceAdded(Net* n, const u32 src_gate_id)
    {
        Q_UNUSED(src_gate_id);

        if (mCurrentId == n->get_id())
            update(mCurrentId);
    }

    void NetDetailsWidget::handleNetSourceRemoved(Net* n, const u32 src_gate_id)
    {
        Q_UNUSED(src_gate_id);

        if (mCurrentId == n->get_id())
            update(mCurrentId);
    }

    void NetDetailsWidget::handleNetDestinationAdded(Net* n, const u32 dst_gate_id)
    {
        Q_UNUSED(dst_gate_id);

        if (mCurrentId == n->get_id())
            update(mCurrentId);
    }

    void NetDetailsWidget::handleNetDestinationRemoved(Net* n, const u32 dst_gate_id)
    {
        Q_UNUSED(dst_gate_id);

        if (mCurrentId == n->get_id())
            update(mCurrentId);
    }

    void NetDetailsWidget::handleGateNameChanged(Gate* g)
    {
        Q_UNUSED(g)

        if (mCurrentId == 0)
            return;

        bool update_needed = false;

        //current net
        auto n = gNetlist->get_net_by_id(mCurrentId);

        //check if current net is in netlist (mCurrentId is unassigned if netlist details widget hasn't been shown once)
        if (!gNetlist->is_net_in_netlist(n))
            return;

        //check if renamed gate is a src of the currently shown net
        for (auto e : n->get_sources())
        {
            if (e->get_gate()->get_id() == mCurrentId)
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
                if (e->get_gate()->get_id() == mCurrentId)
                {
                    update_needed = true;
                    break;
                }
            }
        }

        if (update_needed)
            update(mCurrentId);
    }

    void NetDetailsWidget::handleTableItemClicked(QTableWidgetItem* item)
    {
        if (item->column() != 2)
            return;

        QTableWidget* sender_table = dynamic_cast<QTableWidget*>(sender());

        SelectionRelay::Subfocus focus = (sender_table == mSourcePinsTable) ? SelectionRelay::Subfocus::Right : SelectionRelay::Subfocus::Left;
        auto gate_id                   = item->data(Qt::UserRole).toInt();
        auto pin                       = sender_table->item(item->row(), 0)->text().toStdString();

        auto clicked_gate = gNetlist->get_gate_by_id(gate_id);
        if (!clicked_gate)
            return;

        gSelectionRelay->clear();
        gSelectionRelay->addGate(gate_id);

        auto pins                          = (sender_table == mSourcePinsTable)
                ? clicked_gate->get_type()->get_output_pins()
                : clicked_gate->get_type()->get_input_pins();
        auto index                         = std::distance(pins.begin(), std::find(pins.begin(), pins.end(), pin));
        gSelectionRelay->setFocus(SelectionRelay::ItemType::Gate,gate_id,focus,index);

        gSelectionRelay->relaySelectionChanged(this);
    }

    void NetDetailsWidget::handleSourcesTableMenuRequeted(const QPoint& pos)
    {
        if (!mSourcePinsTable->itemAt(pos) || mSourcePinsTable->itemAt(pos)->column() != 2)
            return;

        QMenu menu;
        menu.addAction("Jump to source gate", [this, pos]() { handleTableItemClicked(mSourcePinsTable->itemAt(pos)); });
        menu.addAction(QIcon(":/icons/python"), "Extract gate as python code (copy to clipboard)", [this, pos]() {
            QApplication::clipboard()->setText("netlist.get_gate_by_id(" + mSourcePinsTable->itemAt(pos)->data(Qt::UserRole).toString() + ")");
        });

        menu.move(dynamic_cast<QWidget*>(sender())->mapToGlobal(pos));
        menu.exec();
    }

    void NetDetailsWidget::handleDestinationsTableMenuRequeted(const QPoint& pos)
    {
        if (!mDestinationPinsTable->itemAt(pos) || mDestinationPinsTable->itemAt(pos)->column() != 2)
            return;

        QMenu menu;
        menu.addAction("Jump to destination gate", [this, pos]() { handleTableItemClicked(mDestinationPinsTable->itemAt(pos)); });
        menu.addAction(QIcon(":/icons/python"), "Extract gate as python code (copy to clipboard)", [this, pos]() {
            QApplication::clipboard()->setText("netlist.get_gate_by_id(" + mDestinationPinsTable->itemAt(pos)->data(Qt::UserRole).toString() + ")");
        });

        menu.move(dynamic_cast<QWidget*>(sender())->mapToGlobal(pos));
        menu.exec();
    }

    void NetDetailsWidget::hideSectionsWhenEmpty(bool hide)
    {
        mSourcePinsSection->hideWhenEmpty(hide);
        mDestinationPinsSection->hideWhenEmpty(hide);
        mDataFieldsSection->hideWhenEmpty(hide);
    }
}    // namespace hal
