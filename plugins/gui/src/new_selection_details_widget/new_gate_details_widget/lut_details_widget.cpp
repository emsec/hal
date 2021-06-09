#include <QHeaderView>

#include "gui/new_selection_details_widget/new_gate_details_widget/lut_details_widget.h"
#include "gui/gui_globals.h"
#include "gui/selection_details_widget/selection_details_widget.h"

namespace hal{

    LutDetailsWidget::LutDetailsWidget(QWidget* parent):
        QTableView(parent),
        mLutModel(new LutModel(this))
    {
        this->setModel(mLutModel);
        this->setSelectionBehavior(QAbstractItemView::SelectRows);
        this->setSelectionMode(QAbstractItemView::SingleSelection);
        this->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
        this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        SelectionTreeView* t = gContentManager->getSelectionDetailsWidget()->selectionTreeView();

        this->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        connect(t, &SelectionTreeView::triggerSelection, this, &LutDetailsWidget::handleFocusChanged);
    }

    void LutDetailsWidget::handleFocusChanged(const SelectionTreeItem* sti){
        if(sti == nullptr){
            return;
        }

        if(sti->itemType() == SelectionTreeItem::TreeItemType::GateItem){
            Gate* g = gNetlist->get_gate_by_id(sti->id());
            updateGate(g);
        }
    }

    void LutDetailsWidget::updateGate(Gate* gate){
        if(gate == nullptr)
            return;
        if(gate->get_type()->get_pins_of_type(PinType::lut).size() > 0){
            mLutModel->update(gate);
            this->resizeColumnsToContents();
            this->resizeRowsToContents();
            this->clearSelection();
            this->update();
            adjustColumnSizes();
        }
    }

    void LutDetailsWidget::adjustColumnSizes(){
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

    void LutDetailsWidget::resizeEvent(QResizeEvent *event) {
        QTableView::resizeEvent(event);
        adjustColumnSizes();
    }

} // namespace hal