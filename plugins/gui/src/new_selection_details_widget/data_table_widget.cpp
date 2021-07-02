#include "gui/new_selection_details_widget/data_table_widget.h"
#include "gui/gui_globals.h"
#include "gui/new_selection_details_widget/py_code_provider.h"
#include <QHeaderView>
#include <QtWidgets/QMenu>
#include <QApplication>
#include <QClipboard>

namespace hal {
    DataTableWidget::DataTableWidget(QWidget *parent) : QTableView(parent),
    mDataTableModel(new DataTableModel(this))
    {
        mCurrentObjectType=DataContainerType::DATA_CONTAINER;

        this->setModel(mDataTableModel);
        this->setSelectionBehavior(QAbstractItemView::SelectRows);
        this->setSelectionMode(QAbstractItemView::SingleSelection);
        this->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
        this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        this->verticalHeader()->setVisible(false);
        this->horizontalHeader()->setVisible(false);
        this->setShowGrid(false);

        SelectionTreeView* t = gContentManager->getSelectionDetailsWidget()->selectionTreeView();
        connect(t, &SelectionTreeView::triggerSelection, this, &DataTableWidget::handleDetailsFocusChanged);
        connect(this, &QTableView::customContextMenuRequested, this, &DataTableWidget::handleContextMenuRequest);
    }

    DataTableModel* DataTableWidget::getModel()
    {
        return mDataTableModel;
    }

    void DataTableWidget::handleDetailsFocusChanged(const SelectionTreeItem* sti)
    {
        if(sti == nullptr){
            return;
        }

        if(sti->itemType() == SelectionTreeItem::TreeItemType::GateItem){
            Gate* g = gNetlist->get_gate_by_id(sti->id());
            setGate(g);
        }
        else if(sti->itemType() == SelectionTreeItem::TreeItemType::NetItem){
            Net* n = gNetlist->get_net_by_id(sti->id());
            setNet(n);
        }
        else if(sti->itemType() == SelectionTreeItem::TreeItemType::ModuleItem){
            Module* m = gNetlist->get_module_by_id(sti->id());
            setModule(m);
        }
    }

    void DataTableWidget::setGate(Gate *gate)
    {
        if(gate == nullptr){
            return;
        }
        mCurrentObjectType = DataContainerType::GATE;
        mCurrentObjectId = gate->get_id();
        mDataTableModel->updateData(gate->get_data_map());
        adjustTableSizes();
    }

    void DataTableWidget::setNet(Net* net)
    {
        if(net == nullptr){
            return;
        }
        mCurrentObjectType = DataContainerType::NET;
        mCurrentObjectId = net->get_id();
        mDataTableModel->updateData(net->get_data_map());
        adjustTableSizes();
    }

    void DataTableWidget::setModule(Module* module)
    {
        if(module == nullptr){
            return;
        }
        mCurrentObjectType = DataContainerType::MODULE;
        mCurrentObjectId = module->get_id();
        mDataTableModel->updateData(module->get_data_map());
        adjustTableSizes();
    }
    

    void DataTableWidget::resizeEvent(QResizeEvent *event)
    {
        QAbstractItemView::resizeEvent(event);
        adjustTableSizes();
    }

    void DataTableWidget::handleContextMenuRequest(const QPoint &pos)
    {
        QModelIndex idx = indexAt(pos);
        // A pure data container can't be accessed via python
        if(!idx.isValid() || mCurrentObjectType==DataContainerType::DATA_CONTAINER){
            return;
        }

        DataTableModel::DataEntry entry = mDataTableModel->getEntryAtRow(idx.row());

        QString menuText = "Exctract data as python code (copy to clipboard)";
        QString pythonCode;

        if(mCurrentObjectType == DataContainerType::GATE)
            pythonCode = PyCodeProvider::pyCodeGateData(mCurrentObjectId, entry.category, entry.key);
        else if(mCurrentObjectType == DataContainerType::NET)
            pythonCode = PyCodeProvider::pyCodeNetData(mCurrentObjectId, entry.category, entry.key);
        else if(mCurrentObjectType == DataContainerType::MODULE)
            pythonCode = PyCodeProvider::pyCodeModuleData(mCurrentObjectId, entry.category, entry.key);         

        QMenu menu;
        menu.addAction(QIcon(":/icons/python"), menuText,
           [pythonCode]()
           {
               QApplication::clipboard()->setText( pythonCode );
           }
        );
        menu.move(dynamic_cast<QWidget*>(sender())->mapToGlobal(pos));
        menu.exec();
    }

    void DataTableWidget::adjustTableSizes()
    {
        resizeColumnsToContents();
        this->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        this->setWordWrap(true);
        this->resizeRowsToContents();
    }


} // namespace hal
