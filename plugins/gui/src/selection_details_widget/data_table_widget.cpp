#include "gui/selection_details_widget/data_table_widget.h"
#include "gui/python/py_code_provider.h"
#include "gui/user_action/action_set_object_data.h"
#include <QHeaderView>
#include <QtWidgets/QMenu>
#include <QApplication>
#include <QClipboard>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QLineEdit>

namespace hal
{

    DataEntryDialog::DataEntryDialog(const DataTableEntry *entry, QWidget* parent)
        : QDialog(parent)
    {
        QFormLayout* layout = new QFormLayout(this);
        const QString label[] = {"Category:", "Key:", "Type:", "Value:"};
        for (int i=0; i<4; i++)
        {
            mLineEdit[i] = new QLineEdit(this);
            if (entry)
                mLineEdit[i]->setText(entry->getPropertyValueByPropType((DataTableEntry::PropertyType)i));
            layout->addRow(label[i],mLineEdit[i]);
        }
        QDialogButtonBox* dbb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
        connect(dbb, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(dbb, &QDialogButtonBox::rejected, this, &QDialog::reject);
        layout->addWidget(dbb);
    }

    DataTableEntry DataEntryDialog::getEntry() const
    {
        return DataTableEntry(mLineEdit[0]->text(), mLineEdit[1]->text(), mLineEdit[2]->text(), mLineEdit[3]->text());
    }

    DataTableWidget::DataTableWidget(QWidget *parent) : QTableView(parent),
    mDataTableModel(new DataTableModel(this))
    {
        mCurrentContainer = nullptr;

        this->setModel(mDataTableModel);
        this->setSelectionMode(QAbstractItemView::SingleSelection);
        this->setSelectionBehavior(QAbstractItemView::SelectRows);
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
        mDataTableModel->updateData(gate->get_data_map());
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
        mDataTableModel->updateData(net->get_data_map());
        mCurrentObject = UserActionObject(net->get_id(), UserActionObjectType::Net);
        clearSelection();
        adjustTableSizes();
    }

    void DataTableWidget::setModule(Module* module)
    {
        if(module == nullptr){
            return;
        }
        mDataTableModel->updateData(module->get_data_map());
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

        DataTableEntry entry = mDataTableModel->getEntryAtRow(idx.row());
        QMenu menu;
        QAction* act;
        menu.addAction("Category to clipboard", [entry](){QApplication::clipboard()->setText(entry.category);});
        menu.addAction("Key to clipboard", [entry](){QApplication::clipboard()->setText(entry.key);});
        menu.addAction("Type to clipboard", [entry](){QApplication::clipboard()->setText(entry.dataType);});
        menu.addAction("Value to clipboard", [entry](){QApplication::clipboard()->setText(entry.value);});

        menu.addSection("ChangeSection");
        menu.addAction("Add data entry", [this](){contextAction(ActionSetObjectData::CreateAction);});
        act = menu.addAction("Change current data entry", [this](){contextAction(ActionSetObjectData::ModifyAction);});
        act->setEnabled(currentIndex().isValid());
        act = menu.addAction("Delete current data entry", [this](){contextAction(ActionSetObjectData::DeleteAction);});
        act->setEnabled(currentIndex().isValid());

        QString pyCode = "";
        switch(mCurrentObject.type())
        {
            case UserActionObjectType::Gate: pyCode = PyCodeProvider::pyCodeGateData(mCurrentObject.id(), entry.category, entry.key); break;
            case UserActionObjectType::Net: pyCode = PyCodeProvider::pyCodeNetData(mCurrentObject.id(), entry.category, entry.key); break;
            case UserActionObjectType::Module: pyCode = PyCodeProvider::pyCodeModuleData(mCurrentObject.id(), entry.category, entry.key); break;
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

        /*
        // Configure the widget height
        int h = horizontalHeader()->height();
        for (int i = 0; i < mDataTableModel->rowCount(); i++)
            h += rowHeight(i);

        setMaximumHeight(h);
        setMinimumHeight(h);
        */
    }

    void DataTableWidget::contextAction(ActionSetObjectData::ActionType actionType)
    {
        DataTableEntry oldEntry;

        if(actionType != ActionSetObjectData::CreateAction) // current (=old) entry is requested for all actions except create
        {
            if (!mCurrentContainer) return;
            QModelIndex idx = currentIndex(); //could also be a parameter to be extra cautious (or the dataentry)
            oldEntry = mDataTableModel->getEntryAtRow(idx.row());
        }

        ActionSetObjectData* act = nullptr;

        if (actionType == ActionSetObjectData::DeleteAction)
        {
            act = new ActionSetObjectData(ActionSetObjectData::DeleteAction, oldEntry);
        }
        else
        {
            DataEntryDialog ded(&oldEntry);
            if (ded.exec() != QDialog::Accepted) return;

            DataTableEntry newEntry = ded.getEntry();
            act = new ActionSetObjectData(actionType, newEntry);
            act->setObject(mCurrentObject); //check if not none?

            // if category or key gets changed it is treated as "MoveAction" internally (delete old + create new)
            if (actionType == ActionSetObjectData::ModifyAction &&
                ((oldEntry.category != newEntry.category) || (oldEntry.key != newEntry.key)))
            {
               act->setChangeKeyAndOrCategory(oldEntry.category, oldEntry.key);
            }

        }

        act->setObject(mCurrentObject); //check if not none?
        act->exec();

        mDataTableModel->updateData(mCurrentContainer->get_data_map());
        clearSelection();
        adjustTableSizes();
    }

} // namespace hal
