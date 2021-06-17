#include "gui/new_selection_details_widget/new_gate_details_widget/boolean_function_table.h"
#include "gui/gui_globals.h"
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
        //std::unordered_map<std::string, BooleanFunction> customBfs = gate->get_boolean_functions(true);
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
        if(showCSBehaviour)
        {
            mBooleanFunctionTableModel->setBooleanFunctionList( latchOrFFFunctions,
                                                                customFunctions,
                                                                gate->get_type()->get_clear_preset_behavior());
        }
        else
        {
            mBooleanFunctionTableModel->setBooleanFunctionList( latchOrFFFunctions,
                                                                customFunctions);
        }
        adjustTableSizes();
        this->clearSelection();
        this->update();
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

        QPair<QString, bool> bfNameOrCSBehavior = mBooleanFunctionTableModel->getBooleanFunctionNameAtRow(idx.row());
        QMenu menu;
        QString menuText;
        QString pythonCode;

        QString getGatePythonCode = QString("netlist.get_gate_by_id(%1)").arg(mCurrentGate->get_id());

        mCurrentGate->get_type()->get_clear_preset_behavior();
        // Entry is a boolean function
        if(!bfNameOrCSBehavior.second)
        {
            menuText = "Extract boolean function as python code (copy to clipboard)";
            pythonCode = getGatePythonCode + QString(".get_boolean_function(\"%1\")").arg(bfNameOrCSBehavior.first);
        }
        // Entry is Clear-Set Behavior
        else
        {
            menuText = "Extract clear-preset behavior as python code (copy to clipboard)";
            pythonCode = getGatePythonCode + QString(".get_type().get_clear_preset_behavior()");
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
