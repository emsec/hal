#include <QHeaderView>

#include "gui/selection_details_widget/gate_details_widget/lut_table_widget.h"
#include "gui/gui_globals.h"
#include "gui/selection_details_widget/selection_details_widget.h"

namespace hal{

    LUTTableWidget::LUTTableWidget(QWidget* parent):
        QTableView(parent),
        mLutModel(new LUTTableModel(this))
    {
        this->setModel(mLutModel);
        this->setSelectionMode(QAbstractItemView::NoSelection);
        this->setFocusPolicy(Qt::NoFocus);
        this->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
        this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setFrameStyle(QFrame::NoFrame);

        this->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    }

    void LUTTableWidget::setBooleanFunction(BooleanFunction bf, QString functionName)
    {
        mLutModel->setBooleanFunction(bf, functionName);
        this->clearSelection();
        this->update();
        adjustTableSizes();
    }

    void LUTTableWidget::adjustTableSizes(){
        // Columns
        int w = this->size().width();
        int colCount = mLutModel->columnCount();

        // The output column is bigger than the size of input columns so that both sizes are in the golden ratio
        int inputWidth = w/(colCount + 0.618);
        for(int c = 0; c < (colCount-1); c++){
            this->horizontalHeader()->setSectionResizeMode(c, QHeaderView::Fixed);
            this->horizontalHeader()->resizeSection(c, inputWidth);
        }
        this->horizontalHeader()->setSectionResizeMode(colCount-1, QHeaderView::Stretch);
        
        // Rows
        this->resizeRowsToContents();
        int h = horizontalHeader()->height() + 4;
        for (int i = 0; i < mLutModel->rowCount(); i++)
            h += rowHeight(i);

        setMaximumHeight(h);
        setMinimumHeight(h);

    }

    void LUTTableWidget::resizeEvent(QResizeEvent *event) {
        QTableView::resizeEvent(event);
        adjustTableSizes();
    }

} // namespace hal
