#include "gui/new_selection_details_widget/new_gate_details_widget/boolean_function_table.h"
#include "gui/gui_globals.h"
#include <QHeaderView>

namespace hal {
    BooleanFunctionTable::BooleanFunctionTable(QWidget *parent) : QTableWidget(parent)
    {
        // TODO
        this->setSelectionBehavior(QAbstractItemView::SelectRows);
        this->setSelectionMode(QAbstractItemView::SingleSelection);
        this->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
        this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setColumnCount(3);
        this->verticalHeader()->setVisible(false);
        this->horizontalHeader()->setVisible(false);

        SelectionTreeView* t = gContentManager->getSelectionDetailsWidget()->selectionTreeView();


        connect(t, &SelectionTreeView::triggerSelection, this, &BooleanFunctionTable::handleDetailsFocusChanged);
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
        // TODO
        if(gate == nullptr){
            return;
        }
        clear();
        std::unordered_map<std::string, BooleanFunction> bfs = gate->get_boolean_functions();
        setRowCount(bfs.size());
        setColumnCount(3);
        int r = 0;
        for(auto& it : bfs){
            QTableWidgetItem* outNameItem = new QTableWidgetItem(QString::fromStdString(it.first));
            outNameItem->setData(Qt::TextAlignmentRole, QVariant(Qt::AlignTop|Qt::AlignRight));
            setItem(r,0,outNameItem);

            QTableWidgetItem* equalSign = new QTableWidgetItem("=");
            equalSign->setData(Qt::TextAlignmentRole, QVariant(Qt::AlignTop|Qt::AlignHCenter));
            setItem(r,1, equalSign);

            setItem(r,2,new QTableWidgetItem(QString::fromStdString(it.second.to_string())));
            r++;
        }
        adjustTableSizes();
    }



    QLabel BooleanFunctionTable::getBooleanFunctionLabel(QString outPinName, const BooleanFunction bf)
    {
        return QLabel();
    }

    void BooleanFunctionTable::resizeEvent(QResizeEvent *event)
    {
        QAbstractItemView::resizeEvent(event);
        adjustTableSizes();
    }

    void BooleanFunctionTable::adjustTableSizes()
    {
        resizeColumnsToContents();
        this->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
        this->setWordWrap(true);
        this->resizeRowsToContents();
    }

} // namespace hal
