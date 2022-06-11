#include "gui/selection_details_widget/data_table_widget.h"
#include "gui/gui_globals.h"
#include "gui/python/py_code_provider.h"
#include "gui/input_dialog/input_dialog.h"
#include <QHeaderView>
#include <QtWidgets/QMenu>
#include <QApplication>
#include <QClipboard>

namespace hal
{

    DataTableWidget::DataTableWidget(QWidget *parent) : QTableView(parent),
    mDataTableModel(new DataTableModel(this))
    {
        mCurrentObjectType=DataContainerType::DATA_CONTAINER;
        mCurrentContainer = nullptr;

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
        mCurrentContainer = dynamic_cast<DataContainer*>(gate);
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
        mCurrentContainer = dynamic_cast<DataContainer*>(net);
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
        mCurrentContainer = dynamic_cast<DataContainer*>(module);
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

        menu.addSection("ChangeSection");
        menu.addAction("Change category", [this](){changePropertyRequested(DataTableModel::propertyType::category);});
        menu.addAction("Change key", [this](){changePropertyRequested(DataTableModel::propertyType::key);});
        menu.addAction("Change type", [this](){changePropertyRequested(DataTableModel::propertyType::type);});
        menu.addAction("Change value", [this](){changePropertyRequested(DataTableModel::propertyType::value);});

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

    void DataTableWidget::changePropertyRequested(DataTableModel::propertyType prop)
    {
        if(!mCurrentContainer)
            return;
        const QString propertyString[] = {"category", "key", "type", "value"};
        QModelIndex idx = currentIndex(); //could also be a parameter to be extra cautious (or the dataentry)
        DataTableModel::DataEntry entry = mDataTableModel->getEntryAtRow(idx.row());
        InputDialog ipd("Change " + propertyString[(int)prop], "Choose new value for the " + propertyString[(int)prop] + " field.", entry.getPropertyValueByPropType(prop));
        if(ipd.exec() == QDialog::Accepted)
        {
            QString values[] = {entry.category, entry.key, entry.dataType, entry.value};
            if(prop == DataTableModel::propertyType::category || prop == DataTableModel::propertyType::key)
                mCurrentContainer->delete_data(values[0].toStdString(), values[1].toStdString());

            values[(int)prop] = ipd.textValue();
            mCurrentContainer->set_data(values[0].toStdString(), values[1].toStdString(), values[2].toStdString(), values[3].toStdString());
            mDataTableModel->updateData(mCurrentContainer->get_data_map());
            clearSelection();
            adjustTableSizes();
        }
    }


} // namespace hal
