#include "gui/selection_details_widget/data_table_widget.h"
#include "gui/gui_globals.h"
#include "gui/python/py_code_provider.h"
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
        this->setSelectionMode(QAbstractItemView::NoSelection);
        this->setFocusPolicy(Qt::NoFocus);
        this->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
        this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        this->verticalHeader()->setVisible(false);
        this->horizontalHeader()->setVisible(true);
        setFrameStyle(QFrame::NoFrame);

        connect(this, &QTableView::customContextMenuRequested, this, &DataTableWidget::handleContextMenuRequest);
    }

    DataTableModel* DataTableWidget::getModel()
    {
        return mDataTableModel;
    }

    void DataTableWidget::setGate(Gate *gate)
    {
        if(gate == nullptr){
            return;
        }
        mCurrentObjectType = DataContainerType::GATE;
        mCurrentObjectId = gate->get_id();
        mDataTableModel->updateData(gate->get_data_map());
        clearSelection();
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
        clearSelection();
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
        clearSelection();
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
        QMenu menu;
        menu.addAction("Category to clipboard", [entry](){QApplication::clipboard()->setText(entry.category);});
        menu.addAction("Key to clipboard", [entry](){QApplication::clipboard()->setText(entry.key);});
        menu.addAction("Type to clipboard", [entry](){QApplication::clipboard()->setText(entry.dataType);});
        menu.addAction("Value to clipboard", [entry](){QApplication::clipboard()->setText(entry.value);});

        QString pyCode = "";
        switch(mCurrentObjectType)
        {
            case DataContainerType::GATE: pyCode = PyCodeProvider::pyCodeGateData(mCurrentObjectId, entry.category, entry.key); break;
            case DataContainerType::NET: pyCode = PyCodeProvider::pyCodeNetData(mCurrentObjectId, entry.category, entry.key); break;
            case DataContainerType::MODULE: pyCode = PyCodeProvider::pyCodeModuleData(mCurrentObjectId, entry.category, entry.key); break;
            default: break;
        }
        menu.addSection("Python");
        menu.addAction(QIcon(":/icons/python"), "Get data entry", [pyCode](){QApplication::clipboard()->setText(pyCode);});


//        QString clipboardText = QString("%1: %2").arg(entry.key, entry.value);

//        menu.addAction("Copy data entry to clipboard",
//           [clipboardText]()
//           {
//               QApplication::clipboard()->setText( clipboardText );
//           }
//        );

//        clipboardText = QString("(%1, %2): (%3, %4)").arg(entry.category, entry.key, entry.dataType, entry.value);

//        menu.addAction("Copy data entry to clipboard (with category and data type)",
//           [clipboardText]()
//           {
//               QApplication::clipboard()->setText( clipboardText );
//           }
//        );
        

//        /*====================================
//                  Data from Python
//          ====================================*/
//        menu.addSection("Python");
//        QString pythonCode;

//        if(mCurrentObjectType == DataContainerType::GATE)
//            pythonCode = PyCodeProvider::pyCodeGateData(mCurrentObjectId, entry.category, entry.key);
//        else if(mCurrentObjectType == DataContainerType::NET)
//            pythonCode = PyCodeProvider::pyCodeNetData(mCurrentObjectId, entry.category, entry.key);
//        else if(mCurrentObjectType == DataContainerType::MODULE)
//            pythonCode = PyCodeProvider::pyCodeModuleData(mCurrentObjectId, entry.category, entry.key);

//        menu.addAction(QIcon(":/icons/python"), "Exctract data as python code (copy to clipboard)",
//           [pythonCode]()
//           {
//               QApplication::clipboard()->setText( pythonCode );
//           }
//        );

//        /*====================================
//                Data Map from Python
//          ====================================*/
//        if(mCurrentObjectType == DataContainerType::GATE)
//            pythonCode = PyCodeProvider::pyCodeGateDataMap(mCurrentObjectId);
//        else if(mCurrentObjectType == DataContainerType::NET)
//            pythonCode = PyCodeProvider::pyCodeNetDataMap(mCurrentObjectId);
//        else if(mCurrentObjectType == DataContainerType::MODULE)
//            pythonCode = PyCodeProvider::pyCodeModuleDataMap(mCurrentObjectId);

//        menu.addAction(QIcon(":/icons/python"), "Exctract data map as python code (copy to clipboard)",
//           [pythonCode]()
//           {
//               QApplication::clipboard()->setText( pythonCode );
//           }
//        );

        menu.move(dynamic_cast<QWidget*>(sender())->mapToGlobal(pos));
        menu.exec();
    }

    void DataTableWidget::adjustTableSizes()
    {
        resizeColumnsToContents();
        this->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
        //this->horizontalHeader()->setStretchLastSection(true);
        this->setWordWrap(true);
        this->resizeRowsToContents();

        // Configure the widget height
        int h = horizontalHeader()->height();
        for (int i = 0; i < mDataTableModel->rowCount(); i++)
            h += rowHeight(i);

        setMaximumHeight(h);
        setMinimumHeight(h);
    }


} // namespace hal
