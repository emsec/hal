#include <QHeaderView>

#include "gui/new_selection_details_widget/new_gate_details_widget/lut_table_widget.h"
#include "gui/gui_globals.h"
#include "gui/selection_details_widget/selection_details_widget.h"

namespace hal{

    LUTTableWidget::LUTTableWidget(QWidget* parent):
        QTableView(parent),
        mLutModel(new LUTTableModel(this))
    {
        this->setModel(mLutModel);
        this->setSelectionBehavior(QAbstractItemView::SelectRows);
        this->setSelectionMode(QAbstractItemView::SingleSelection);
        this->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
        this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        SelectionTreeView* t = gContentManager->getSelectionDetailsWidget()->selectionTreeView();

        this->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        connect(t, &SelectionTreeView::triggerSelection, this, &LUTTableWidget::handleDetailsFocusChanged);
    }

    void LUTTableWidget::setBooleanFunction(BooleanFunction bf, QString functionName)
    {
        mLutModel->setBooleanFunction(bf, functionName);
        this->clearSelection();
        this->update();
        adjustColumnSizes();
        Q_EMIT updateText(mFrameTitle);
    }

    void LUTTableWidget::handleDetailsFocusChanged(const SelectionTreeItem* sti){
        if(sti == nullptr){
            return;
        }

        if(sti->itemType() == SelectionTreeItem::TreeItemType::GateItem){
            Gate* g = gNetlist->get_gate_by_id(sti->id());
            updateGate(g);
        }
    }

    void LUTTableWidget::updateGate(Gate* gate){
        if(gate == nullptr)
            return;

        std::unordered_set<std::basic_string<char>> lutPins = gate->get_type()->get_pins_of_type(PinType::lut);
        // The table is only updated if the gate has a LUT pin
        if(lutPins.size() > 0){
            // All LUT pins have the same boolean function
            std::basic_string<char> outPin = *lutPins.begin();
            BooleanFunction lutFunction = gate->get_boolean_function(outPin);
            setBooleanFunction(lutFunction, QString::fromStdString(outPin));
        }
    }

    void LUTTableWidget::adjustColumnSizes(){
        int w = this->size().width();
        int colCount = mLutModel->columnCount();

        // The output column is bigger than the size of input columns so that both sizes are in the golden ratio
        int inputWidth = w/(colCount + 0.618);
        for(int c = 0; c < (colCount-1); c++){
            this->horizontalHeader()->setSectionResizeMode(c, QHeaderView::Fixed);
            this->horizontalHeader()->resizeSection(c, inputWidth);
        }
        this->horizontalHeader()->setSectionResizeMode(colCount-1, QHeaderView::Stretch);
    }

    void LUTTableWidget::resizeEvent(QResizeEvent *event) {
        QTableView::resizeEvent(event);
        adjustColumnSizes();
    }

} // namespace hal