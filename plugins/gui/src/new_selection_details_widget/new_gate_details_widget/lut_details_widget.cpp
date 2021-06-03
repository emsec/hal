#include "gui/new_selection_details_widget/new_gate_details_widget/lut_details_widget.h"
#include "gui/gui_globals.h"
#include "gui/selection_details_widget/selection_details_widget.h"

namespace hal{

    LutDetailsWidget::LutDetailsWidget(QWidget* parent):
        QWidget(parent),
        mLutModel(new LutModel(this))
    {
        mLutTableView = new QTableView(this);
        mLutTableView->setModel(mLutModel);
        mLutTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        mLutTableView->setSelectionMode(QAbstractItemView::SingleSelection); // ERROR ???
        mLutTableView->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
        //mLutTableView->verticalHeader()->hide();
        mLutTableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        SelectionTreeView* t = gContentManager->getSelectionDetailsWidget()->selectionTreeView();


        connect(t, &SelectionTreeView::triggerSelection, this, &LutDetailsWidget::handleFocusChanged);
    }

    void LutDetailsWidget::handleFocusChanged(const SelectionTreeItem* sti){
        if(sti == nullptr){
            return;
        }

        if(sti->itemType() == SelectionTreeItem::TreeItemType::GateItem){
            Gate* g = gNetlist->get_gate_by_id(sti->id());
            if(g == nullptr)
                return;
            if(g->get_type()->get_pins_of_type(PinType::lut).size() > 0){
                mLutModel->update(g);
                mLutTableView->resizeColumnsToContents();
                mLutTableView->resizeRowsToContents();
                mLutTableView->update();
            }
        }
    }

} // namespace hal