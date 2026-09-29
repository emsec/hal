#include "gui/selection_details_widget/data_table_widget.h"
#include "gui/gui_globals.h"
#include "gui/python/py_code_provider.h"
#include "gui/input_dialog/input_dialog.h"
#include "gui/user_action/action_set_object_data.h"
#include "gui/user_action/action_set_object_parameter.h"
#include <QHeaderView>
#include <QtWidgets/QMenu>
#include <QApplication>
#include <QClipboard>

namespace hal
{

    DataTableWidget::DataTableWidget(QWidget *parent) : QTableView(parent),
    mDataTableModel(new DataTableModel(this))
    {
        mCurrentContainer = nullptr;

        this->setModel(mDataTableModel);
        this->setSelectionMode(QAbstractItemView::NoSelection);
        this->setFocusPolicy(Qt::NoFocus);
        this->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
        this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        this->verticalHeader()->setVisible(false);
        this->horizontalHeader()->setVisible(true);
        this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
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
        mCurrentContainer = dynamic_cast<DataContainer*>(gate);
        mDataTableModel->updateData(gate);
        mCurrentObject = UserActionObject(gate->get_id(), UserActionObjectType::Gate);
        clearSelection();
        adjustTableSizes();
    }

    void DataTableWidget::setNet(Net* net)
    {
        if(net == nullptr){
            return;
        }
        mCurrentContainer = dynamic_cast<DataContainer*>(net);
        mDataTableModel->updateData(net);
        mCurrentObject = UserActionObject(net->get_id(), UserActionObjectType::Net);
        clearSelection();
        adjustTableSizes();
    }

    void DataTableWidget::setModule(Module* module)
    {
        if(module == nullptr){
            return;
        }
        mDataTableModel->updateData(module);
        mCurrentContainer = dynamic_cast<DataContainer*>(module);
        mCurrentObject = UserActionObject(module->get_id(), UserActionObjectType::Module);
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
        if(!idx.isValid() || mCurrentObject.type() == UserActionObjectType::None){
            return;
        }

        DataTableModel::DataEntry entry = mDataTableModel->getEntryAtRow(idx.row());
        QMenu menu;
        menu.addAction("Category to clipboard", [entry](){QApplication::clipboard()->setText(entry.category);});
        menu.addAction("Key to clipboard", [entry](){QApplication::clipboard()->setText(entry.key);});
        menu.addAction("Type to clipboard", [entry](){QApplication::clipboard()->setText(entry.dataType);});
        menu.addAction("Value to clipboard", [entry](){QApplication::clipboard()->setText(entry.value);});

        menu.addSection("ChangeSection");
        if (entry.typed)
        {
            // a typed value keeps its declaration; the value is validated against the type
            menu.addAction("Change value", [this](){changeParameterValueRequested();});
            menu.addAction("Delete parameter", [this](){deleteParameterRequested();});
        }
        else
        {
            menu.addAction("Change category", [this](){changePropertyRequested(DataTableModel::propertyType::category);});
            menu.addAction("Change key", [this](){changePropertyRequested(DataTableModel::propertyType::key);});
            menu.addAction("Change type", [this](){changePropertyRequested(DataTableModel::propertyType::type);});
            menu.addAction("Change value", [this](){changePropertyRequested(DataTableModel::propertyType::value);});
        }

        QString pyCode = "";
        switch(mCurrentObject.type())
        {
            case UserActionObjectType::Gate: pyCode = entry.typed ? PyCodeProvider::pyCodeGateParameter(mCurrentObject.id(), entry.key, entry.category) : PyCodeProvider::pyCodeGateData(mCurrentObject.id(), entry.category, entry.key); break;
            case UserActionObjectType::Net: pyCode = entry.typed ? PyCodeProvider::pyCodeNetParameter(mCurrentObject.id(), entry.key, entry.category) : PyCodeProvider::pyCodeNetData(mCurrentObject.id(), entry.category, entry.key); break;
            case UserActionObjectType::Module: pyCode = entry.typed ? PyCodeProvider::pyCodeModuleParameter(mCurrentObject.id(), entry.key, entry.category) : PyCodeProvider::pyCodeModuleData(mCurrentObject.id(), entry.category, entry.key); break;
            default: break;
        }
        menu.addSection("Python");
        menu.addAction(QIcon(":/icons/python"), entry.typed ? "Get parameter value" : "Get data entry", [pyCode](){QApplication::clipboard()->setText(pyCode);});

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

        /*
        // Configure the widget height
        int h = horizontalHeader()->height();
        for (int i = 0; i < mDataTableModel->rowCount(); i++)
            h += rowHeight(i);

        setMaximumHeight(h);
        setMinimumHeight(h);
        */
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
            QString oldValues[] = {entry.category, entry.key, entry.dataType, entry.value};
            QString newValues[] = {entry.category, entry.key, entry.dataType, entry.value};
            newValues[(int)prop] = ipd.textValue();
            ActionSetObjectData* act = new ActionSetObjectData(newValues[0], newValues[1], newValues[2], newValues[3]);
            act->setObject(mCurrentObject); //check if not none?
            if(prop == DataTableModel::propertyType::category || prop == DataTableModel::propertyType::key)
                act->setChangeKeyAndOrCategory(oldValues[0], oldValues[1]);
            act->exec();

            mDataTableModel->updateData(mCurrentContainer);
            clearSelection();
            adjustTableSizes();
        }
    }


    void DataTableWidget::changeParameterValueRequested()
    {
        if(!mCurrentContainer)
            return;
        QModelIndex idx = currentIndex();
        DataTableModel::DataEntry entry = mDataTableModel->getEntryAtRow(idx.row());
        InputDialog ipd("Change value", "New value of " + entry.category + " '" + entry.key + "' of type " + entry.dataType + ".", entry.value);
        if(ipd.exec() == QDialog::Accepted)
        {
            ActionSetObjectParameter* act = new ActionSetObjectParameter(entry.category, entry.key, ipd.textValue());
            act->setObject(mCurrentObject);
            if (act->exec())
            {
                Q_EMIT parameterChanged();
            }
            mDataTableModel->updateData(mCurrentContainer);
            clearSelection();
            adjustTableSizes();
        }
    }

    void DataTableWidget::deleteParameterRequested()
    {
        if(!mCurrentContainer)
            return;
        QModelIndex idx = currentIndex();
        DataTableModel::DataEntry entry = mDataTableModel->getEntryAtRow(idx.row());
        ActionSetObjectParameter* act = new ActionSetObjectParameter(entry.category, entry.key);
        act->setRemove();
        act->setObject(mCurrentObject);
        if (act->exec())
        {
            Q_EMIT parameterChanged();
        }
        mDataTableModel->updateData(mCurrentContainer);
        clearSelection();
        adjustTableSizes();
    }

} // namespace hal
