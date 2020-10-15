#include "gui/selection_details_widget/details_table_utilities.h"
#include <QTableWidget>
#include <QHeaderView>

namespace hal {

    QSize DetailsTableUtilities::tableWidgetSize(const QTableWidget *table)
    {
        return tableViewSize(table, table->rowCount(), table->columnCount());
    }

    QSize DetailsTableUtilities::tableViewSize(const QTableView* table, int nrows, int ncols)
    {
        //necessary to test if the table is empty, otherwise (due to the resizeColumnsToContents function)
        //is the tables width far too big, so just return 0 as the size
        if (!nrows)
            return QSize(0, 0);

        int w = table->verticalHeader()->width() + 4;    // +4 seems to be needed
        for (int i = 0; i < ncols; i++)
            w += table->columnWidth(i);    // seems to include gridline
        int h = table->horizontalHeader()->height() + 4;
        for (int i = 0; i < nrows; i++)
            h += table->rowHeight(i);
        return QSize(w + 5, h);
    }

    void DetailsTableUtilities::setDefaultTableStyle(QTableView* table)
    {
        //tab->horizontalHeader()->setStretchLastSection(true);
        table->horizontalHeader()->hide();
        table->verticalHeader()->hide();
        table->verticalHeader()->setDefaultSectionSize(16);
        table->resizeColumnToContents(0);
        table->setShowGrid(false);
        table->setFocusPolicy(Qt::NoFocus);
        table->setFrameStyle(QFrame::NoFrame);
        table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        table->setMaximumHeight(table->verticalHeader()->length());
        table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        table->setContextMenuPolicy(Qt::CustomContextMenu);
    }

}
