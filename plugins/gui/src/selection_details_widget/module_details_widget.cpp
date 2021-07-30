#include "gui/selection_details_widget/module_details_widget.h"

#include "gui/graph_widget/graph_navigation_widget.h"
#include "gui/gui_globals.h"
#include "gui/input_dialog/input_dialog.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "gui/selection_details_widget/data_fields_table.h"
#include "gui/selection_details_widget/details_section_widget.h"
#include "gui/selection_details_widget/disputed_big_icon.h"
#include "gui/selection_details_widget/details_general_model.h"
#include "gui/selection_details_widget/details_table_utilities.h"
#include "gui/user_action/action_rename_object.h"

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
        mScrollArea       = new QScrollArea();
        mTopLvlContainer = new QWidget();
        mTopLvlLayout    = new QVBoxLayout(mTopLvlContainer);
        mTopLvlContainer->setLayout(mTopLvlLayout);
        mContentLayout = new QVBoxLayout(this);
        mScrollArea->setWidget(mTopLvlContainer);
        mScrollArea->setWidgetResizable(true);

        mContentLayout->setContentsMargins(0, 0, 0, 0);
        mContentLayout->setSpacing(0);
        mTopLvlLayout->setContentsMargins(0, 0, 0, 0);
        mTopLvlLayout->setSpacing(0);

        QHBoxLayout* intermediate_layout_gt = new QHBoxLayout();
        intermediate_layout_gt->setContentsMargins(3, 3, 0, 0);
        intermediate_layout_gt->setSpacing(0);

        mGeneralInfoButton = new QPushButton("Module Information", this);
        mGeneralInfoButton->setEnabled(false);

        mGeneralView         = new QTableView(this);
        mGeneralModel        = new DetailsGeneralModel(mGeneralView);
        mGeneralModel->setDummyContent<Module>();
        mGeneralView->setModel(mGeneralModel);
        mInputPortsTable  = new QTableWidget(0, 3);
        mOutputPortsTable = new QTableWidget(0, 3);
        mDataFieldsTable    = new DataFieldsTable(this);

        mInputPortsSection  = new DetailsSectionWidget("Input Ports (%1)", mInputPortsTable, this);
        mOutputPortsSection = new DetailsSectionWidget("Output Ports (%1)", mOutputPortsTable, this);
        mDataFieldsSection  = new DetailsSectionWidget("Data Fields (%1)", mDataFieldsTable, this);

        DetailsTableUtilities::setDefaultTableStyle(mGeneralView);
        mGeneralView->setSelectionBehavior(QAbstractItemView::SelectRows);
        mGeneralView->setSelectionMode(QAbstractItemView::SingleSelection);

        // place module icon
        mBigIcon = new DisputedBigIcon("sel_module", this);

        intermediate_layout_gt->addWidget(mGeneralView);
        intermediate_layout_gt->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
        intermediate_layout_gt->addWidget(mBigIcon);
        intermediate_layout_gt->setAlignment(mBigIcon, Qt::AlignTop);

        mTopLvlLayout->addWidget(mGeneralInfoButton);
        mTopLvlLayout->addLayout(intermediate_layout_gt);
        mTopLvlLayout->addSpacerItem(new QSpacerItem(0, 7, QSizePolicy::Expanding, QSizePolicy::Fixed));
        mTopLvlLayout->addWidget(mInputPortsSection);
        mTopLvlLayout->addWidget(mOutputPortsSection);
        mTopLvlLayout->addWidget(mDataFieldsSection);

        mTopLvlLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
        mContentLayout->addWidget(mScrollArea);

        //setup the navigation_table ("activated" by clicking on an input / output pin in the 2 tables)
        //delete the table manually so its not necessarry to add a property for the stylesheet(otherwise this table is styled like the others)
        mNavigationTable = new GraphNavigationWidget(true);
        mNavigationTable->setWindowFlags(Qt::CustomizeWindowHint);
        mNavigationTable->hide();

        connect(mNavigationTable, &GraphNavigationWidget::navigationRequested, this, &ModuleDetailsWidget::handleNavigationJumpRequested);
        connect(mNavigationTable, &GraphNavigationWidget::closeRequested, this, &ModuleDetailsWidget::handleNavigationCloseRequested);

        connect(gNetlistRelay, &NetlistRelay::netlistMarkedGlobalInput, this, &ModuleDetailsWidget::handleNetlistMarkedGlobalInput);
        connect(gNetlistRelay, &NetlistRelay::netlistMarkedGlobalOutput, this, &ModuleDetailsWidget::handleNetlistMarkedGlobalOutput);
        connect(gNetlistRelay, &NetlistRelay::netlistMarkedGlobalInout, this, &ModuleDetailsWidget::handleNetlistMarkedGlobalInout);
        connect(gNetlistRelay, &NetlistRelay::netlistUnmarkedGlobalInput, this, &ModuleDetailsWidget::handleNetlistUnmarkedGlobalInput);
        connect(gNetlistRelay, &NetlistRelay::netlistUnmarkedGlobalOutput, this, &ModuleDetailsWidget::handleNetlistUnmarkedGlobalOutput);
        connect(gNetlistRelay, &NetlistRelay::netlistUnmarkedGlobalInout, this, &ModuleDetailsWidget::handleNetlistUnmarkedGlobalInout);

        connect(gNetlistRelay, &NetlistRelay::moduleNameChanged, this, &ModuleDetailsWidget::handleModuleNameChanged);
        connect(gNetlistRelay, &NetlistRelay::moduleSubmoduleAdded, this, &ModuleDetailsWidget::handleSubmoduleAdded);
        connect(gNetlistRelay, &NetlistRelay::moduleSubmoduleRemoved, this, &ModuleDetailsWidget::handleSubmoduleRemoved);
        connect(gNetlistRelay, &NetlistRelay::moduleGateAssigned, this, &ModuleDetailsWidget::handleModuleGateAssigned);
        connect(gNetlistRelay, &NetlistRelay::moduleGateRemoved, this, &ModuleDetailsWidget::handleModuleGateRemoved);
        connect(gNetlistRelay, &NetlistRelay::moduleInputPortNameChanged, this, &ModuleDetailsWidget::handleModuleInputPortNameChanged);
        connect(gNetlistRelay, &NetlistRelay::moduleOutputPortNameChanged, this, &ModuleDetailsWidget::handleModuleOutputPortNameChanged);
        connect(gNetlistRelay, &NetlistRelay::moduleTypeChanged, this, &ModuleDetailsWidget::handleModuleTypeChanged);

        connect(gNetlistRelay, &NetlistRelay::netNameChanged, this, &ModuleDetailsWidget::handleNetNameChanged);
        connect(gNetlistRelay, &NetlistRelay::netSourceAdded, this, &ModuleDetailsWidget::handleNetSourceAdded);
        connect(gNetlistRelay, &NetlistRelay::netSourceRemoved, this, &ModuleDetailsWidget::handleNetSourceRemoved);
        connect(gNetlistRelay, &NetlistRelay::netDestinationAdded, this, &ModuleDetailsWidget::handleNetDestinationAdded);
        connect(gNetlistRelay, &NetlistRelay::netDestinationRemoved, this, &ModuleDetailsWidget::handleNetDestinationRemoved);

        connect(mInputPortsTable, &QTableWidget::customContextMenuRequested, this, &ModuleDetailsWidget::handleInputPortsTableMenuRequested);
        connect(mOutputPortsTable, &QTableWidget::customContextMenuRequested, this, &ModuleDetailsWidget::handleOutputPortsTableMenuRequested);
        connect(mInputPortsTable, &QTableWidget::itemDoubleClicked, this, &ModuleDetailsWidget::handleInputNetItemClicked);
        connect(mOutputPortsTable, &QTableWidget::itemDoubleClicked, this, &ModuleDetailsWidget::handleOutputNetItemClicked);

        connect(mGeneralModel, &DetailsGeneralModel::requireUpdate, this, &ModuleDetailsWidget::update);
        gSelectionRelay->registerSender(this, "SelectionDetailsWidget");
    }

    ModuleDetailsWidget::~ModuleDetailsWidget()
    {
        delete mNavigationTable;
    }

    void ModuleDetailsWidget::update(const u32 module_id)
    {
        mCurrentId = module_id;

        if (mCurrentId == 0)
            return;

        auto m = gNetlist->get_module_by_id(module_id);
        if (!m) return;

        mGeneralModel->setContent<Module>(m);

        mGeneralView->resizeColumnsToContents();
        mGeneralView->setFixedSize(DetailsTableUtilities::tableViewSize(mGeneralView,mGeneralModel->rowCount(),mGeneralModel->columnCount()));
        mGeneralView->update();

        //update table with input ports
        mInputPortsTable->clearContents();
        mInputPortsSection->setRowCount(m->get_input_nets().size());
        mInputPortsTable->setRowCount(m->get_input_nets().size());
        mInputPortsTable->setMaximumHeight(mInputPortsTable->verticalHeader()->length());
        mInputPortsTable->setMinimumHeight(mInputPortsTable->verticalHeader()->length());

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

            mInputPortsTable->setItem(index, 0, port_name);
            mInputPortsTable->setItem(index, 1, arrow_item);
            mInputPortsTable->setItem(index, 2, net_item);

            index++;
        }

        mInputPortsTable->resizeColumnsToContents();
        mInputPortsTable->setFixedWidth(DetailsTableUtilities::tableWidgetSize(mInputPortsTable).width());

        //update table with output ports
        mOutputPortsTable->clearContents();
        mOutputPortsSection->setRowCount(m->get_output_nets().size());
        mOutputPortsTable->setRowCount(m->get_output_nets().size());
        mOutputPortsTable->setMaximumHeight(mOutputPortsTable->verticalHeader()->length());
        mOutputPortsTable->setMinimumHeight(mOutputPortsTable->verticalHeader()->length());

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

            mOutputPortsTable->setItem(index, 0, port_name);
            mOutputPortsTable->setItem(index, 1, arrow_item);
            mOutputPortsTable->setItem(index, 2, net_item);

            index++;
        }

        mOutputPortsTable->resizeColumnsToContents();
        mOutputPortsTable->setFixedWidth(DetailsTableUtilities::tableWidgetSize(mOutputPortsTable).width());

        //update data fields table
        mDataFieldsSection->setRowCount(m->get_data_map().size());
        mDataFieldsTable->updateData(module_id, m->get_data_map());
    }

    void ModuleDetailsWidget::handleNetlistMarkedGlobalInput(Netlist* netlist, u32 associated_data)
    {
        Q_UNUSED(netlist)

        if (mCurrentId == 0)
            return;

        auto module = gNetlist->get_module_by_id(mCurrentId);
        auto gates  = module->get_gates(nullptr, true);
        auto net    = gNetlist->get_net_by_id(associated_data);

        for (auto gate : gates)
        {
            auto in_nets  = gate->get_fan_in_nets();
            auto out_nets = gate->get_fan_out_nets();

            if (std::find(in_nets.begin(), in_nets.end(), net) != in_nets.end() || std::find(out_nets.begin(), out_nets.end(), net) != out_nets.end())
            {
                update(mCurrentId);
                return;
            }
        }
    }

    void ModuleDetailsWidget::handleNetlistMarkedGlobalOutput(Netlist* netlist, u32 associated_data)
    {
        Q_UNUSED(netlist)

        if (mCurrentId == 0)
            return;

        auto module = gNetlist->get_module_by_id(mCurrentId);
        auto gates  = module->get_gates(nullptr, true);
        auto net    = gNetlist->get_net_by_id(associated_data);

        for (auto gate : gates)
        {
            auto in_nets  = gate->get_fan_in_nets();
            auto out_nets = gate->get_fan_out_nets();

            if (std::find(in_nets.begin(), in_nets.end(), net) != in_nets.end() || std::find(out_nets.begin(), out_nets.end(), net) != out_nets.end())
            {
                update(mCurrentId);
                return;
            }
        }
    }

    void ModuleDetailsWidget::handleNetlistMarkedGlobalInout(Netlist* netlist, u32 associated_data)
    {
        Q_UNUSED(netlist)

        if (mCurrentId == 0)
            return;

        auto module = gNetlist->get_module_by_id(mCurrentId);
        auto gates  = module->get_gates(nullptr, true);
        auto net    = gNetlist->get_net_by_id(associated_data);

        for (auto gate : gates)
        {
            auto in_nets  = gate->get_fan_in_nets();
            auto out_nets = gate->get_fan_out_nets();

            if (std::find(in_nets.begin(), in_nets.end(), net) != in_nets.end() || std::find(out_nets.begin(), out_nets.end(), net) != out_nets.end())
            {
                update(mCurrentId);
                return;
            }
        }
    }

    void ModuleDetailsWidget::handleNetlistUnmarkedGlobalInput(Netlist* netlist, u32 associated_data)
    {
        Q_UNUSED(netlist)

        if (mCurrentId == 0)
            return;

        auto module = gNetlist->get_module_by_id(mCurrentId);
        auto gates  = module->get_gates(nullptr, true);
        auto net    = gNetlist->get_net_by_id(associated_data);

        for (auto gate : gates)
        {
            auto in_nets  = gate->get_fan_in_nets();
            auto out_nets = gate->get_fan_out_nets();

            if (std::find(in_nets.begin(), in_nets.end(), net) != in_nets.end() || std::find(out_nets.begin(), out_nets.end(), net) != out_nets.end())
            {
                update(mCurrentId);
                return;
            }
        }
    }

    void ModuleDetailsWidget::handleNetlistUnmarkedGlobalOutput(Netlist* netlist, u32 associated_data)
    {
        Q_UNUSED(netlist)

        if (mCurrentId == 0)
            return;

        auto module = gNetlist->get_module_by_id(mCurrentId);
        auto gates  = module->get_gates(nullptr, true);
        auto net    = gNetlist->get_net_by_id(associated_data);

        for (auto gate : gates)
        {
            auto in_nets  = gate->get_fan_in_nets();
            auto out_nets = gate->get_fan_out_nets();

            if (std::find(in_nets.begin(), in_nets.end(), net) != in_nets.end() || std::find(out_nets.begin(), out_nets.end(), net) != out_nets.end())
            {
                update(mCurrentId);
                return;
            }
        }
    }

    void ModuleDetailsWidget::handleNetlistUnmarkedGlobalInout(Netlist* netlist, u32 associated_data)
    {
        Q_UNUSED(netlist)

        if (mCurrentId == 0)
            return;

        auto module = gNetlist->get_module_by_id(mCurrentId);
        auto gates  = module->get_gates(nullptr, true);
        auto net    = gNetlist->get_net_by_id(associated_data);

        for (auto gate : gates)
        {
            auto in_nets  = gate->get_fan_in_nets();
            auto out_nets = gate->get_fan_out_nets();

            if (std::find(in_nets.begin(), in_nets.end(), net) != in_nets.end() || std::find(out_nets.begin(), out_nets.end(), net) != out_nets.end())
            {
                update(mCurrentId);
                return;
            }
        }
    }

    void ModuleDetailsWidget::handleModuleNameChanged(Module* module)
    {
        if (mCurrentId == module->get_id())
            update(mCurrentId);
    }

    void ModuleDetailsWidget::handleSubmoduleAdded(Module* module, u32 associated_data)
    {
        Q_UNUSED(associated_data);

        if (mCurrentId == 0)
            return;

        auto current_module = gNetlist->get_module_by_id(mCurrentId);

        if (mCurrentId == module->get_id() || current_module->contains_module(module, true))
            update(mCurrentId);
    }

    void ModuleDetailsWidget::handleSubmoduleRemoved(Module* module, u32 associated_data)
    {
        Q_UNUSED(associated_data);

        if (mCurrentId == 0)
            return;

        auto current_module = gNetlist->get_module_by_id(mCurrentId);

        if (mCurrentId == module->get_id() || current_module->contains_module(module, true))
            update(mCurrentId);
    }

    void ModuleDetailsWidget::handleModuleGateAssigned(Module* module, u32 associated_data)
    {
        Q_UNUSED(associated_data);

        if (mCurrentId == 0)
            return;

        auto current_module = gNetlist->get_module_by_id(mCurrentId);

        if (mCurrentId == module->get_id() || current_module->contains_module(module, true))
            update(mCurrentId);
    }

    void ModuleDetailsWidget::handleModuleGateRemoved(Module* module, u32 associated_data)
    {
        Q_UNUSED(associated_data);

        if (mCurrentId == 0)
            return;

        auto current_module = gNetlist->get_module_by_id(mCurrentId);

        if (mCurrentId == module->get_id() || current_module->contains_module(module, true))
            update(mCurrentId);
    }

    void ModuleDetailsWidget::handleModuleInputPortNameChanged(Module* module, u32 associated_data)
    {
        Q_UNUSED(associated_data);

        if (mCurrentId == module->get_id())
            update(mCurrentId);
    }

    void ModuleDetailsWidget::handleModuleOutputPortNameChanged(Module* module, u32 associated_data)
    {
        Q_UNUSED(associated_data);

        if (mCurrentId == module->get_id())
            update(mCurrentId);
    }

    void ModuleDetailsWidget::handleModuleTypeChanged(Module* module)
    {
        if (mCurrentId == module->get_id())
            update(mCurrentId);
    }

    void ModuleDetailsWidget::handleNetNameChanged(Net* net)
    {
        if (mCurrentId == 0)
            return;

        auto module      = gNetlist->get_module_by_id(mCurrentId);
        auto input_nets  = module->get_input_nets();
        auto output_nets = module->get_output_nets();

        if (std::find(input_nets.begin(), input_nets.end(), net) != input_nets.end() || std::find(output_nets.begin(), output_nets.end(), net) != output_nets.end())
            update(mCurrentId);
    }

    void ModuleDetailsWidget::handleNetSourceAdded(Net* net, const u32 src_gate_id)
    {
        Q_UNUSED(net)

        if (mCurrentId == 0)
            return;

        auto module = gNetlist->get_module_by_id(mCurrentId);
        auto gate   = gNetlist->get_gate_by_id(src_gate_id);

        if (module->contains_gate(gate, true))
            update(mCurrentId);
    }

    void ModuleDetailsWidget::handleNetSourceRemoved(Net* net, const u32 src_gate_id)
    {
        Q_UNUSED(net)

        if (mCurrentId == 0)
            return;

        auto module = gNetlist->get_module_by_id(mCurrentId);
        auto gate   = gNetlist->get_gate_by_id(src_gate_id);

        if (module->contains_gate(gate, true))
            update(mCurrentId);
    }

    void ModuleDetailsWidget::handleNetDestinationAdded(Net* net, const u32 dst_gate_id)
    {
        Q_UNUSED(net)

        if (mCurrentId == 0)
            return;

        auto module = gNetlist->get_module_by_id(mCurrentId);
        auto gate   = gNetlist->get_gate_by_id(dst_gate_id);

        if (module->contains_gate(gate, true))
            update(mCurrentId);
    }

    void ModuleDetailsWidget::handleNetDestinationRemoved(Net* net, const u32 dst_gate_id)
    {
        Q_UNUSED(net)

        if (mCurrentId == 0)
            return;

        auto module = gNetlist->get_module_by_id(mCurrentId);
        auto gate   = gNetlist->get_gate_by_id(dst_gate_id);

        if (module->contains_gate(gate, true))
            update(mCurrentId);
    }

    void ModuleDetailsWidget::handleInputPortsTableMenuRequested(const QPoint& pos)
    {
        auto curr_item = mInputPortsTable->itemAt(pos);

        if (!curr_item || curr_item->column() == 1)
            return;

        QMenu menu;
        if (curr_item->column() == 2)
        {
            auto clicked_net = gNetlist->get_net_by_id(curr_item->data(Qt::UserRole).toInt());
            if (!gNetlist->is_global_input_net(clicked_net))
            {
                menu.addAction("Jump to source gate", [this, curr_item]() { handleInputNetItemClicked(curr_item); });
            }

            menu.addAction(QIcon(":/icons/python"), "Extract net as python code (copy to clipboard)", [curr_item]() {
                QApplication::clipboard()->setText("netlist.get_net_by_id(" + curr_item->data(Qt::UserRole).toString() + ")");
            });

            menu.addAction(QIcon(":/icons/python"), "Extract sources as python code (copy to clipboard)", [curr_item]() {
                QApplication::clipboard()->setText("netlist.get_net_by_id(" + curr_item->data(Qt::UserRole).toString() + ").get_sources()");
            });
        }
        else
        {
            menu.addAction("Change input port name", [this, curr_item]() {
                InputDialog ipd("Change port name", "New port name", curr_item->text());
                if (ipd.exec() == QDialog::Accepted)
                {
                    auto corresponding_net = gNetlist->get_net_by_id(mInputPortsTable->item(curr_item->row(), 2)->data(Qt::UserRole).toInt());
                    if (!corresponding_net)
                        return;
                    ActionRenameObject* act = new ActionRenameObject(ipd.textValue());
                    act->setObject(UserActionObject(mCurrentId,UserActionObjectType::Port));
                    act->setInputNetId(corresponding_net->get_id());
                    act->exec();
                    update(mCurrentId);
                }
            });
        }

        menu.move(dynamic_cast<QWidget*>(sender())->mapToGlobal(pos));
        menu.exec();
    }

    void ModuleDetailsWidget::handleOutputPortsTableMenuRequested(const QPoint& pos)
    {
        auto curr_item = mOutputPortsTable->itemAt(pos);
        if (!curr_item || curr_item->column() == 1)
            return;

        QMenu menu;
        if (curr_item->column() == 2)
        {
            auto clicked_net = gNetlist->get_net_by_id(curr_item->data(Qt::UserRole).toInt());
            if (!gNetlist->is_global_output_net(clicked_net))
            {
                menu.addAction("Jump to destination gate", [this, curr_item]() { handleOutputNetItemClicked(curr_item); });
            }
            menu.addAction(QIcon(":/icons/python"), "Extract net as python code (copy to clipboard)", [curr_item]() {
                QApplication::clipboard()->setText("netlist.get_net_by_id(" + curr_item->data(Qt::UserRole).toString() + ")");
            });

            menu.addAction(QIcon(":/icons/python"), "Extract destinations as python code (copy to clipboard)", [curr_item]() {
                QApplication::clipboard()->setText("netlist.get_net_by_id(" + curr_item->data(Qt::UserRole).toString() + ").get_destinations()");
            });
        }
        else
        {
            menu.addAction("Change output port name", [this, curr_item]() {
                InputDialog ipd("Change port name", "New port name", curr_item->text());
                if (ipd.exec() == QDialog::Accepted)
                {
                    auto corresponding_net = gNetlist->get_net_by_id(mOutputPortsTable->item(curr_item->row(), 2)->data(Qt::UserRole).toInt());
                    if (!corresponding_net)
                        return;
                    ActionRenameObject* act = new ActionRenameObject(ipd.textValue());
                    act->setObject(UserActionObject(mCurrentId,UserActionObjectType::Port));
                    act->setOutputNetId(corresponding_net->get_id());
                    act->exec();
                    update(mCurrentId);
                }
            });
        }

        menu.move(dynamic_cast<QWidget*>(sender())->mapToGlobal(pos));
        menu.exec();
    }

    void ModuleDetailsWidget::handleOutputNetItemClicked(const QTableWidgetItem* item)
    {
        if (item->column() != 2)
            return;

        int mNetId       = item->data(Qt::UserRole).toInt();
        Net* clicked_net = gNetlist->get_net_by_id(mNetId);

        if (!clicked_net)
            return;

        auto destinations = clicked_net->get_destinations();
        if (destinations.empty() || clicked_net->is_global_output_net())
        {
            gSelectionRelay->clear();
            gSelectionRelay->addNet(mNetId);
            gSelectionRelay->relaySelectionChanged(this);
        }
        else if (destinations.size() == 1)
        {
            auto ep = *destinations.begin();
            gSelectionRelay->clear();
            gSelectionRelay->addGate(ep->get_gate()->get_id());

            auto pins                          = ep->get_gate()->get_type()->get_input_pins();
            auto index                         = std::distance(pins.begin(), std::find(pins.begin(), pins.end(), ep->get_pin()));
            gSelectionRelay->setFocus(SelectionRelay::ItemType::Gate,ep->get_gate()->get_id(),
                                      SelectionRelay::Subfocus::Left,index);

            update(ep->get_gate()->get_id());
            gSelectionRelay->relaySelectionChanged(this);
        }
        else
        {
            mNavigationTable->setup(Node(), clicked_net, SelectionRelay::Subfocus::Right);
            if (mNavigationTable->isEmpty())
                mNavigationTable->closeRequest();
            else
            {
                mNavigationTable->move(QCursor::pos());
                mNavigationTable->show();
                mNavigationTable->setFocus();
            }
        }
    }

    void ModuleDetailsWidget::handleInputNetItemClicked(const QTableWidgetItem* item)
    {
        if (item->column() != 2)
            return;

        auto net = gNetlist->get_net_by_id(item->data(Qt::UserRole).toInt());

        if (!net)
            return;

        auto sources = net->get_sources();

        if (sources.empty() || net->is_global_input_net())
        {
            gSelectionRelay->clear();
            gSelectionRelay->addNet(net->get_id());
            gSelectionRelay->relaySelectionChanged(this);
        }
        else if (sources.size() == 1)
        {
            auto ep = *sources.begin();
            gSelectionRelay->clear();
            gSelectionRelay->addGate(ep->get_gate()->get_id());

            auto pins                          = ep->get_gate()->get_type()->get_output_pins();
            auto index                         = std::distance(pins.begin(), std::find(pins.begin(), pins.end(), ep->get_pin()));
            gSelectionRelay->setFocus(SelectionRelay::ItemType::Gate,ep->get_gate()->get_id(),
                                      SelectionRelay::Subfocus::Right,index);

            update(ep->get_gate()->get_id());
            gSelectionRelay->relaySelectionChanged(this);
        }
        else
        {
            mNavigationTable->setup(Node(), net, SelectionRelay::Subfocus::Left);
            if (mNavigationTable->isEmpty())
                mNavigationTable->closeRequest();
            else
            {
                mNavigationTable->move(QCursor::pos());
                mNavigationTable->show();
                mNavigationTable->setFocus();
            }
        }
    }

    void ModuleDetailsWidget::handleNavigationCloseRequested()
    {
        mNavigationTable->hide();
    }

    void ModuleDetailsWidget::handleNavigationJumpRequested(const Node& origin, const u32 via_net, const QSet<u32>& to_gates)
    {
        Q_UNUSED(origin);

        auto n = gNetlist->get_net_by_id(via_net);

        if (to_gates.isEmpty() || !n)
            return;
        for (u32 id : to_gates)
        {
            if (!gNetlist->get_gate_by_id(id))
                return;
        }

        mNavigationTable->hide();
        gSelectionRelay->clear();
        gSelectionRelay->setSelectedGates(to_gates);
        if (to_gates.size() == 1)
        {
            auto g = gNetlist->get_gate_by_id(*to_gates.constBegin());

            u32 index_cnt = 0;
            for (const auto& pin : g->get_type()->get_input_pins())
            {
                if (g->get_fan_in_net(pin) == n)
                {
                    gSelectionRelay->setFocus(SelectionRelay::ItemType::Gate,g->get_id(),
                                              SelectionRelay::Subfocus::Left,index_cnt);
                    break;
                }
                index_cnt++;
            }
        }
        gSelectionRelay->relaySelectionChanged(this);
        mNavigationTable->hide();
    }

    void ModuleDetailsWidget::hideSectionsWhenEmpty(bool hide)
    {
        mInputPortsSection->hideWhenEmpty(hide);
        mOutputPortsSection->hideWhenEmpty(hide);
        mDataFieldsSection->hideWhenEmpty(hide);
    }
}    // namespace hal
