#include "gui/new_selection_details_widget/new_gate_details_widget/lut_details_widget.h"

namespace hal{

    LutDetailsWidget::LutDetailsWidget(QWidget* parent):
        mLutModel(new LutModel(this))
    {
        mLutTableView = new QTableView(this);
        mLutTableView->setModel(mLutModel);
        mLutTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        mLutTableView->setSelectionMode(QAbstractItemView::SingleSelection); // ERROR ???
        mLutTableView->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
        //mLutTableView->verticalHeader()->hide();
        mLutTableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

} // namespace hal