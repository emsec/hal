#include "gui/selection_details_widget/gate_details_widget.h"
#include "gui/selection_details_widget/data_fields_table.h"
#include "gui/selection_details_widget/disputed_big_icon.h"
#include "gui/selection_details_widget/details_section_widget.h"
#include "gui/selection_details_widget/details_general_model.h"
#include "gui/selection_details_widget/details_table_utilities.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"

#include "gui/gui_globals.h"
#include "gui/gui_utils/geometry.h"

#include "gui/graph_widget/graph_navigation_widget.h"
#include "hal_core/netlist/module.h"

#include <QApplication>
#include <QCursor>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QResizeEvent>
#include <QScrollArea>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QScrollBar>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QApplication> //to extract the stylesheet of the main app.
#include <QMenu>
#include <QIcon>
#include <QClipboard>

namespace hal
{
    GateDetailsWidget::GateDetailsWidget(QWidget* parent) : DetailsWidget(DetailsWidget::GateDetails, parent)
    {

        //this line throws a warning that there is already an existing layout, yet there is no layout set and
        //even after calling delete layout(); and then setting the layout, the warning continues
        mScrollArea = new QScrollArea(this);
        mTopLvlContainer = new QWidget(mScrollArea);
        mTopLvlLayout = new QVBoxLayout(mTopLvlContainer);
        mTopLvlContainer->setLayout(mTopLvlLayout);;
		mContentLayout = new QVBoxLayout(this);
        mScrollArea->setWidget(mTopLvlContainer);
        mScrollArea->setWidgetResizable(true);

        //layout customization
        mContentLayout->setContentsMargins(0,0,0,0);
        mContentLayout->setSpacing(0);
        mTopLvlLayout->setContentsMargins(0,0,0,0);
        mTopLvlLayout->setSpacing(0);

        //container-layouts to add spacing Widgets (gt = general table, op = output pins, etc)
        QHBoxLayout *intermediate_layout_gt = new QHBoxLayout();
        intermediate_layout_gt->setContentsMargins(3,3,0,0);
        intermediate_layout_gt->setSpacing(0);

        // buttons
        mGeneralInfoButton = new QPushButton("Gate Information", this);
        mGeneralInfoButton->setEnabled(false);

        // table initializations (section 1-4)
        mGeneralView        = new QTableView(this);
        mGeneralModel       = new DetailsGeneralModel(mGeneralView);
        mGeneralModel->setDummyContent<Gate>();
        mGeneralView->setModel(mGeneralModel);
        mInputPinsTable  = new QTableWidget(0,3, this);
        mOutputPinsTable = new QTableWidget(0,3, this);
        mDataFieldsTable   = new DataFieldsTable(this);

        // sections
        mInputPinsSection  = new DetailsSectionWidget("Input Pins (%1)", mInputPinsTable, this);
        mOutputPinsSection = new DetailsSectionWidget("Output Pins (%1)", mOutputPinsTable, this);
        mDataFieldsSection = new DetailsSectionWidget("Data Fields (%1)", mDataFieldsTable, this);

        //shared stlye options (every option is applied to each table)
        DetailsTableUtilities::setDefaultTableStyle(mGeneralView);
        mGeneralView->setSelectionBehavior(QAbstractItemView::SelectRows);
        mGeneralView->setSelectionMode(QAbstractItemView::SingleSelection);

        //(5) Boolean Function section
        mBooleanFunctionsContainer = new QWidget(this);
        mBooleanFunctionsContainerLayout = new QVBoxLayout(mBooleanFunctionsContainer);
        mBooleanFunctionsContainerLayout->setContentsMargins(6,5,0,0);
        mBooleanFunctionsContainerLayout->setSpacing(0);
        mBooleanFunctionsContainer->setLayout(mBooleanFunctionsContainerLayout);
        mBooleanFunctionsSection = new DetailsSectionWidget(mBooleanFunctionsContainer, "Boolean Functions (%1)", this);

        // place gate icon
        mBigIcon = new DisputedBigIcon("sel_gate", this);

        //adding things to intermediate layout (the one thats neccessary for the left spacing)
        intermediate_layout_gt->addWidget(mGeneralView);
        intermediate_layout_gt->addSpacerItem(new QSpacerItem(0,0, QSizePolicy::Expanding, QSizePolicy::Fixed));
        intermediate_layout_gt->addWidget(mBigIcon);
        intermediate_layout_gt->setAlignment(mBigIcon,Qt::AlignTop);

        //adding things to the main layout
        mTopLvlLayout->addWidget(mGeneralInfoButton);
        mTopLvlLayout->addLayout(intermediate_layout_gt);
        mTopLvlLayout->addSpacerItem(new QSpacerItem(0, 7, QSizePolicy::Expanding, QSizePolicy::Fixed));
        mTopLvlLayout->addWidget(mInputPinsSection);
        mTopLvlLayout->addWidget(mOutputPinsSection);
        mTopLvlLayout->addWidget(mDataFieldsSection);
        mTopLvlLayout->addWidget(mBooleanFunctionsSection);

        //necessary to add at the end
        mTopLvlLayout->addSpacerItem(new QSpacerItem(0,0, QSizePolicy::Expanding, QSizePolicy::Expanding));
        mContentLayout->addWidget(mScrollArea);

        //setup the navigation_table ("activated" by clicking on an input / output pin in the 2 tables)
        //delete the table manually so its not necessarry to add a property for the stylesheet(otherwise this table is styled like the others)
        mNavigationTable = new GraphNavigationWidget(true);
        mNavigationTable->setWindowFlags(Qt::CustomizeWindowHint);
        mNavigationTable->hide();
        connect(mNavigationTable, &GraphNavigationWidget::navigationRequested, this, &GateDetailsWidget::handleNavigationJumpRequested);
        connect(mNavigationTable, &GraphNavigationWidget::closeRequested, this, &GateDetailsWidget::handleNavigationCloseRequested);

        connect(mInputPinsTable, &QTableWidget::itemDoubleClicked, this, &GateDetailsWidget::handleInputPinItemClicked);
        connect(mOutputPinsTable, &QTableWidget::itemDoubleClicked, this, &GateDetailsWidget::handleOutputPinItemClicked);
        connect(mGeneralModel, &DetailsGeneralModel::requireUpdate, this, &GateDetailsWidget::update);


        //context menu connects
        connect(mInputPinsTable, &QTableWidget::customContextMenuRequested, this, &GateDetailsWidget::handleInputPinTableMenuRequested);
        connect(mOutputPinsTable, &QTableWidget::customContextMenuRequested, this, &GateDetailsWidget::handleOutputPinTableMenuRequested);
        connect(gNetlistRelay, &NetlistRelay::gateNameChanged, this, &GateDetailsWidget::handleGateNameChanged);

        gSelectionRelay->registerSender(this, "SelectionDetailsWidget");

        //extract the width of the scrollbar out of the stylesheet to fix a scrollbar related bug
        QString main_stylesheet = qApp->styleSheet();
        main_stylesheet.replace("\n", ""); //remove newlines so the regex is a bit easier
        QRegularExpression re(".+?QScrollBar:vertical ?{[^}]+?(?: *width *?|; *width *?): *([0-9]*)[^;]*");
        QRegularExpressionMatch ma = re.match(main_stylesheet);
        mScrollbarWidth = (ma.hasMatch()) ? ma.captured(1).toInt() : 0;
    }

    GateDetailsWidget::~GateDetailsWidget()
    {
        delete mNavigationTable;
    }

    void GateDetailsWidget::handleGateNameChanged(Gate* gate)
    {
        if (mCurrentId == gate->get_id())
            update(mCurrentId);
    }

    void GateDetailsWidget::handleGateRemoved(Gate* gate)
    {
        if (mCurrentId == gate->get_id())
        {
            mGeneralView->setHidden(true);
            mScrollArea->setHidden(true);
        }
    }

    void GateDetailsWidget::handleNetNameChanged(Net* net)
    {
        bool update_needed = false;

        //check if currently shown gate is a src of renamed net
        for (auto& e : net->get_sources())
        {
            if (mCurrentId == e->get_gate()->get_id())
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
                if (mCurrentId == e->get_gate()->get_id())
                {
                    update_needed = true;
                    break;
                }
            }
        }

        if (update_needed)
            update(mCurrentId);
    }

    void GateDetailsWidget::handleNetSourceAdded(Net* net, const u32 src_gate_id)
    {
        Q_UNUSED(net);
        if (mCurrentId == src_gate_id)
            update(mCurrentId);
    }

    void GateDetailsWidget::handleNetSourceRemoved(Net* net, const u32 src_gate_id)
    {
        Q_UNUSED(net);
        if (mCurrentId == src_gate_id)
            update(mCurrentId);
    }

    void GateDetailsWidget::handleNetDestinationAdded(Net* net, const u32 dst_gate_id)
    {
        Q_UNUSED(net);
        if (mCurrentId == dst_gate_id)
            update(mCurrentId);
    }

    void GateDetailsWidget::handleNetDestinationRemoved(Net* net, const u32 dst_gate_id)
    {
        Q_UNUSED(net);
        if (mCurrentId == dst_gate_id)
            update(mCurrentId);
    }


    void GateDetailsWidget::handleInputPinItemClicked(const QTableWidgetItem *item)
    {
        if(item->column() != 2)
            return;

        int mNetId = item->data(Qt::UserRole).toInt();

        auto clicked_net = gNetlist->get_net_by_id(mNetId);

        if(!clicked_net)
            return;

        auto sources = clicked_net->get_sources();

        if(sources.empty() || clicked_net->is_global_input_net())
        {
            gSelectionRelay->clear();
            gSelectionRelay->addNet(mNetId);
            gSelectionRelay->relaySelectionChanged(this);
        }
        else if(sources.size() == 1)
        {
            auto ep = *sources.begin();
            gSelectionRelay->clear();
            gSelectionRelay->addGate(ep->get_gate()->get_id());

            auto pins                          = ep->get_gate()->get_type()->get_output_pins();
            auto index                         = std::distance(pins.begin(), std::find(pins.begin(), pins.end(), ep->get_pin()));
            gSelectionRelay->setFocus(SelectionRelay::ItemType::Gate,
                                      ep->get_gate()->get_id(),
                                      SelectionRelay::Subfocus::Right,index);

            update(ep->get_gate()->get_id());
            gSelectionRelay->relaySelectionChanged(this);
        }
        else
        {
            mNavigationTable->setup(Node(), clicked_net, SelectionRelay::Subfocus::Left);
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

    void GateDetailsWidget::handleOutputPinItemClicked(const QTableWidgetItem *item)
    {
        if(item->column() != 2)
            return;

        int mNetId = item->data(Qt::UserRole).toInt();
        Net* clicked_net = gNetlist->get_net_by_id(mNetId);

        if(!clicked_net)
            return;

        auto destinations = clicked_net->get_destinations();
        if(destinations.empty() || clicked_net->is_global_output_net())
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
            gSelectionRelay->setFocus(SelectionRelay::ItemType::Gate,
                                      ep->get_gate()->get_id(),
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

    void GateDetailsWidget::handleInputPinTableMenuRequested(const QPoint &pos)
    {
        if(!mInputPinsTable->itemAt(pos) || mInputPinsTable->itemAt(pos)->column() != 2)
            return;

        QMenu menu;

        auto clicked_net = gNetlist->get_net_by_id(mInputPinsTable->itemAt(pos)->data(Qt::UserRole).toInt());
        if(!gNetlist->is_global_input_net(clicked_net))
        {
            menu.addAction("Jump to source gate", [this, pos](){
                handleInputPinItemClicked(mInputPinsTable->itemAt(pos));
            });
        }

        menu.addAction(QIcon(":/icons/python"), "Extract net as python code (copy to clipboard)",[this, pos](){
            QApplication::clipboard()->setText("netlist.get_net_by_id(" + mInputPinsTable->itemAt(pos)->data(Qt::UserRole).toString() + ")");
        });

        menu.addAction(QIcon(":/icons/python"), "Extract sources as python code (copy to clipboard)",[this, pos](){
            QApplication::clipboard()->setText("netlist.get_net_by_id(" + mInputPinsTable->itemAt(pos)->data(Qt::UserRole).toString() + ").get_sources()" );
        });

        menu.move(dynamic_cast<QWidget*>(sender())->mapToGlobal(pos));
        menu.exec();

    }

    void GateDetailsWidget::handleOutputPinTableMenuRequested(const QPoint &pos)
    {
        if(!mOutputPinsTable->itemAt(pos) || mOutputPinsTable->itemAt(pos)->column() != 2)
            return;

        QMenu menu;

        auto clicked_net = gNetlist->get_net_by_id(mOutputPinsTable->itemAt(pos)->data(Qt::UserRole).toInt());
        if(!gNetlist->is_global_output_net(clicked_net))
        {
            menu.addAction("Jump to destination gate", [this, pos](){
                handleOutputPinItemClicked(mOutputPinsTable->itemAt(pos));
            });
        }
        menu.addAction(QIcon(":/icons/python"), "Extract net as python code (copy to clipboard)",[this, pos](){
            QApplication::clipboard()->setText("netlist.get_net_by_id(" + mOutputPinsTable->itemAt(pos)->data(Qt::UserRole).toString() + ")");
        });

        menu.addAction(QIcon(":/icons/python"), "Extract destinations as python code (copy to clipboard)",[this, pos](){
            QApplication::clipboard()->setText("netlist.get_net_by_id(" + mOutputPinsTable->itemAt(pos)->data(Qt::UserRole).toString() + ").get_destinations()" );
        });

        menu.move(dynamic_cast<QWidget*>(sender())->mapToGlobal(pos));
        menu.exec();
    }

    void GateDetailsWidget::handleModuleRemoved(Module* module)
    {
        if (mCurrentId == 0)
            return;
        auto g = gNetlist->get_gate_by_id(mCurrentId);

        if (module->contains_gate(g))
        {
            update(mCurrentId);
        }
    }

    void GateDetailsWidget::handleModuleNameChanged(Module* module)
    {
        if (mCurrentId == 0)
            return;
        auto g = gNetlist->get_gate_by_id(mCurrentId);

        if (module->contains_gate(g))
        {
            update(mCurrentId);
        }
    }

    void GateDetailsWidget::handleModuleGateAssigned(Module* module, u32 associated_data)
    {
        Q_UNUSED(module);
        if (mCurrentId == associated_data)
        {
            update(mCurrentId);
        }
    }

    void GateDetailsWidget::handleModuleGateRemoved(Module* module, u32 associated_data)
    {
        Q_UNUSED(module);
        if (!gNetlist->is_gate_in_netlist(gNetlist->get_gate_by_id(associated_data)))
            return;

        if (mCurrentId == associated_data)
        {
            update(mCurrentId);
        }
    }

    void GateDetailsWidget::resizeEvent(QResizeEvent* event)
    {
        //2 is needed because just the scrollbarwitdth is not enough (does not include its border?)
        mBooleanFunctionsContainer->setFixedWidth(event->size().width() - mScrollbarWidth-2);
    }

    void GateDetailsWidget::update(const u32 gate_id)
    {
        auto g = gNetlist->get_gate_by_id(gate_id);
        mCurrentId = gate_id;

        if(!g || mCurrentId == 0)
            return;

        mGeneralModel->setContent<Gate>(g);

        mGeneralView->resizeColumnsToContents();
        mGeneralView->setFixedSize(DetailsTableUtilities::tableViewSize(mGeneralView,
                                    mGeneralModel->rowCount(),
                                    mGeneralModel->columnCount()));

        //update (2)input-pin section
        mInputPinsTable->clearContents();
        mInputPinsSection->setRowCount(g->get_type()->get_input_pins().size());
        mInputPinsTable->setRowCount(g->get_type()->get_input_pins().size());
        mInputPinsTable->setMaximumHeight(mInputPinsTable->verticalHeader()->length());
        mInputPinsTable->setMinimumHeight(mInputPinsTable->verticalHeader()->length());
        int index = 0;
        for(const auto &pin : g->get_type()->get_input_pins())
        {
            QTableWidgetItem* pin_name = new QTableWidgetItem(QString::fromStdString(pin));
            QTableWidgetItem* arrow_item = new QTableWidgetItem(QChar(0x2b05));
            QTableWidgetItem* net_item = new QTableWidgetItem();
            arrow_item->setForeground(QBrush(QColor(114, 140, 0), Qt::SolidPattern));
            //pin_name->setFlags(Qt::ItemIsEnabled);
            pin_name->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
            arrow_item->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
            //arrow_item->setFlags((Qt::ItemFlag)(~Qt::ItemIsSelectable));
            net_item->setFlags(Qt::ItemIsEnabled);

            auto input_net = gNetlist->get_gate_by_id(gate_id)->get_fan_in_net(pin);
            if(input_net)
            {
                net_item->setText(QString::fromStdString(input_net->get_name()));
                net_item->setData(Qt::UserRole, input_net->get_id());
            }
            else
                net_item->setText("unconnected");

            mInputPinsTable->setItem(index, 0, pin_name);
            mInputPinsTable->setItem(index, 1, arrow_item);
            mInputPinsTable->setItem(index, 2, net_item);
            index++;
        }
        mInputPinsTable->resizeColumnsToContents();
        mInputPinsTable->setFixedWidth(DetailsTableUtilities::tableWidgetSize(mInputPinsTable).width());

        //update(3) output pins section
        mOutputPinsTable->clearContents();
        mOutputPinsSection->setRowCount(g->get_type()->get_output_pins().size());
        mOutputPinsTable->setRowCount(g->get_type()->get_output_pins().size());
        mOutputPinsTable->setMaximumHeight(mOutputPinsTable->verticalHeader()->length());
        mOutputPinsTable->setMinimumHeight(mOutputPinsTable->verticalHeader()->length());
        index = 0;
        for(const auto &pin : g->get_type()->get_output_pins())
        {
            QTableWidgetItem* pin_name = new QTableWidgetItem(QString::fromStdString(pin));
            QTableWidgetItem* arrow_item = new QTableWidgetItem(QChar(0x27a1));
            QTableWidgetItem* net_item = new QTableWidgetItem();
            arrow_item->setForeground(QBrush(QColor(114, 140, 0), Qt::SolidPattern));//stylesheet?
            //pin_name->setFlags(Qt::ItemIsEnabled);
            pin_name->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
            //arrow_item->setFlags(Qt::ItemIsEnabled);
            arrow_item->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
            net_item->setFlags(Qt::ItemIsEnabled);

            auto output_net = gNetlist->get_gate_by_id(gate_id)->get_fan_out_net(pin);
            if(output_net)
            {
                net_item->setText(QString::fromStdString(output_net->get_name()));
                net_item->setData(Qt::UserRole, output_net->get_id());
            }
            else
                net_item->setText("unconnected");

            mOutputPinsTable->setItem(index, 0, pin_name);
            mOutputPinsTable->setItem(index, 1, arrow_item);
            mOutputPinsTable->setItem(index, 2, net_item);
            index++;
        }
        mOutputPinsTable->resizeColumnsToContents();
        mOutputPinsTable->setFixedWidth(DetailsTableUtilities::tableWidgetSize(mOutputPinsTable).width());

        //update(4) data fields section
        mDataFieldsSection->setRowCount(g->get_data_map().size());
        mDataFieldsTable->updateData(gate_id,g->get_data_map());

        //update(5) boolean functions section
        //clear container layout
        while(mBooleanFunctionsContainerLayout->itemAt(0) != 0)
        {
            QLayoutItem* i = mBooleanFunctionsContainerLayout->takeAt(0);
            delete i->widget();
            delete i;
        }

        mBooleanFunctionsSection->setRowCount(g->get_boolean_functions().size());
        QFrame* last_line = nullptr; //unexpected behaviour below otherwise
        for(const auto& it : g->get_boolean_functions())
        {
            QLabel* fnct = new QLabel(QString::fromStdString(it.first) + " = " + QString::fromStdString(it.second.to_string()));
            fnct->setWordWrap(true);
            mBooleanFunctionsContainerLayout->addWidget(fnct);
            QFrame* line = new QFrame;
            line->setFrameShape(QFrame::HLine);
            line->setFrameShadow(QFrame::Sunken);
            //to outsource this line into the stylesheet you need to make a new class that inherits from QFrame
            //and style that class. properties and the normal way does not work (other tables are also affected)
            line->setStyleSheet("QFrame{background-color: gray;}");
            last_line = line;
            mBooleanFunctionsContainerLayout->addWidget(line);
        }

        if(last_line){
            mBooleanFunctionsContainerLayout->removeWidget(last_line);
            delete last_line;
        }

        //to prevent any updating(render) errors that can occur, manually tell the tables to update
        mGeneralView->resizeColumnsToContents();
        mGeneralView->setFixedSize(DetailsTableUtilities::tableViewSize(mGeneralView,mGeneralModel->rowCount(),mGeneralModel->columnCount()));
        mGeneralView->update();

        mInputPinsTable->update();
        mOutputPinsTable->update();
        mDataFieldsTable->update();
    }

    void GateDetailsWidget::handleNavigationCloseRequested()
    {
        mNavigationTable->hide();
    }

    void GateDetailsWidget::handleNavigationJumpRequested(const Node& origin, const u32 via_net, const QSet<u32>& to_gates)
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
                    gSelectionRelay->setFocus(SelectionRelay::ItemType::Gate,
                                              g->get_id(),
                                              SelectionRelay::Subfocus::Left,index_cnt);
                    break;
                }
                index_cnt++;
            }
        }
        gSelectionRelay->relaySelectionChanged(this);
        mNavigationTable->hide();

        // TODO ensure gates mVisible in graph
    }
    
    void GateDetailsWidget::hideSectionsWhenEmpty(bool hide)
    {
        mInputPinsSection->hideWhenEmpty(hide);
        mOutputPinsSection->hideWhenEmpty(hide);
        mDataFieldsSection->hideWhenEmpty(hide);
        mBooleanFunctionsSection->hideWhenEmpty(hide);  
    }
/*
    void GateDetailsWidget::handle_general_table_item_clicked(const QTableWidgetItem *item)
    {
        //cant get the item from the index (static_cast<QTableWidgetItem*>(index.internalPointer()) fails),
        //so ask the item QTableWidgetItem directly
        if (item->row() == m_ModuleItem->row() && item->column() == m_ModuleItem->column())
        {
            gSelectionRelay->clear();
            gSelectionRelay->addModule(m_ModuleItem->data(Qt::UserRole).toInt());
            gSelectionRelay->relaySelectionChanged(this);
        }
    }
    */
}
