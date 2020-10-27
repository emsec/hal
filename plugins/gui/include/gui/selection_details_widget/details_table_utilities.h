#pragma once
#include <QSize>

class QTableView;
class QTableWidget;

namespace hal {

    class DetailsTableUtilities
    {
    public:
        //utility function, used to calculate the actual width so the scrollbars and the accuracy of the click functionality is correct
        static QSize tableViewSize(const QTableView* table, int nrows, int ncols);
        static QSize tableWidgetSize(const QTableWidget* table);

        static void setDefaultTableStyle(QTableView* table);
    };
}
