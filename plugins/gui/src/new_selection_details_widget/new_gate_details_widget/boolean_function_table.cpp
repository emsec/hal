#include "gui/new_selection_details_widget/new_gate_details_widget/boolean_function_table.h"
#include "gui/gui_globals.h"
#include "gui/new_selection_details_widget/py_code_provider.h"
#include <QHeaderView>
#include <QtWidgets/QMenu>
#include <QApplication>
#include <QClipboard>

namespace hal {
    BooleanFunctionTable::BooleanFunctionTable(QWidget *parent) : QTableView(parent),
    mBooleanFunctionTableModel(new BooleanFunctionTableModel(this))
    {
        this->setModel(mBooleanFunctionTableModel);
        this->setSelectionBehavior(QAbstractItemView::SelectRows);
        this->setSelectionMode(QAbstractItemView::SingleSelection);
        this->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
        this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        this->verticalHeader()->setVisible(false);
        this->horizontalHeader()->setVisible(false);
        this->setShowGrid(false);

        SelectionTreeView* t = gContentManager->getSelectionDetailsWidget()->selectionTreeView();
        connect(t, &SelectionTreeView::triggerSelection, this, &BooleanFunctionTable::handleDetailsFocusChanged);
        connect(this, &QTableView::customContextMenuRequested, this, &BooleanFunctionTable::handleContextMenuRequest);
    }

    BooleanFunctionTableModel BooleanFunctionTable::getModel()
    {
        return mBooleanFunctionTableModel;
    }

    void BooleanFunctionTable::handleDetailsFocusChanged(const SelectionTreeItem* sti)
    {
        if(sti == nullptr){
            return;
        }

        if(sti->itemType() == SelectionTreeItem::TreeItemType::GateItem){
            Gate* g = gNetlist->get_gate_by_id(sti->id());
            setGate(g);
        }
    }

    void BooleanFunctionTable::setEntries(QList<QSharedPointer<BooleanFunctionTableEntry>> entries){
        mBooleanFunctionTableModel->setEntries(entries);
        adjustTableSizes();
        this->clearSelection();
        this->update();
    }

    //TODO: can be (partially) used later in the gate details widget
    void BooleanFunctionTable::setGate(Gate *gate)
    {
        if(gate == nullptr){
            return;
        }
        mCurrentGate = gate;
        // Only for debug
        bool showCSBehaviour = (gate->get_type()->get_clear_preset_behavior() != std::make_pair(GateType::ClearPresetBehavior::undef, GateType::ClearPresetBehavior::undef));

        static QSet<QString> lutOrLatchBfNames = {
            "clear", "preset", // Both
            "clocked_on", "clocked_on_also", "next_state", "power_down_function", // FF names
            "enable", "data_in" // Latch names
        };

        std::unordered_map<std::string, BooleanFunction> allBfs = gate->get_boolean_functions(false);

        QMap<QString, BooleanFunction> latchOrFFFunctions;
        QMap<QString, BooleanFunction> customFunctions;

        for(auto& it : allBfs){
            QString bfName = QString::fromStdString(it.first);
            if(lutOrLatchBfNames.contains(bfName))
            {
                // Function is a custom function
                latchOrFFFunctions.insert(bfName, it.second);
            }
            else
            {
                // All non-custom function are considered
                customFunctions.insert(bfName, it.second);
            }
        }

        QList<QSharedPointer<BooleanFunctionTableEntry>> entries;
        QMap<QString, BooleanFunction>::iterator i;
        for(i = latchOrFFFunctions.begin(); i != latchOrFFFunctions.end(); i++)
            entries.append(QSharedPointer<BooleanFunctionTableEntry>(new BooleanFunctionEntry(gate->get_id(), i.key(), i.value())));

        for(i = customFunctions.begin(); i != customFunctions.end(); i++)
            entries.append(QSharedPointer<BooleanFunctionTableEntry>(new BooleanFunctionEntry(gate->get_id(), i.key(), i.value())));

        if(showCSBehaviour){
            entries.append(QSharedPointer<BooleanFunctionTableEntry>(new CPBehaviorEntry(gate->get_id(), gate->get_type()->get_clear_preset_behavior())));
        }

        setEntries(entries);

    }

    void BooleanFunctionTable::resizeEvent(QResizeEvent *event)
    {
        QAbstractItemView::resizeEvent(event);
        adjustTableSizes();
    }

    void BooleanFunctionTable::handleContextMenuRequest(const QPoint &pos)
    {
        QModelIndex idx = indexAt(pos);
        if(!idx.isValid()){
            return;
        }

        QSharedPointer<BooleanFunctionTableEntry> entry = mBooleanFunctionTableModel->getEntryAtRow(idx.row());
        QMenu menu;
        QString menuText;
        QString pythonCode;

        // Entry is a boolean function
        if(!entry->isCPBehavior())
        {
            menuText = "Extract boolean function as python code (copy to clipboard)";
            pythonCode = PyCodeProvider::pyCodeGateBooleanFunction(entry->getGateId(), entry->getEntryIdentifier());
        }
        // Entry is clear-preset behavior
        else
        {
            menuText = "Extract clear-preset behavior as python code (copy to clipboard)";
            pythonCode = PyCodeProvider::pyCodeGateClearPresetBehavior(entry->getGateId());
        }
        menu.addAction(QIcon(":/icons/python"), menuText,
           [pythonCode]()
           {
               QApplication::clipboard()->setText( pythonCode );
           }
        );
        menu.move(dynamic_cast<QWidget*>(sender())->mapToGlobal(pos));
        menu.exec();
    }

    void BooleanFunctionTable::adjustTableSizes()
    {
        resizeColumnsToContents();
        this->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        this->setWordWrap(true);
        this->resizeRowsToContents();
    }


} // namespace hal
