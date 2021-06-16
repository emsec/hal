#include "gui/new_selection_details_widget/new_gate_details_widget/boolean_function_table.h"
#include "gui/gui_globals.h"
#include <QHeaderView>

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

    void BooleanFunctionTable::adjustTableSizes()
    {
        resizeColumnsToContents();
        this->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        this->setWordWrap(true);
        this->resizeRowsToContents();
    }


} // namespace hal
