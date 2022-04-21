#pragma once

#include <QDialog>
#include <QList>
#include "netlist_simulator_controller/wave_data.h"

class QLineEdit;
class QTableWidget;
class QRadioButton;

namespace hal {

    class BooleanDialog : public QDialog
    {
        Q_OBJECT
        QLineEdit* mLineEdit;
        QTableWidget* mTableWidget;
        QRadioButton* mEnterExpression;
        QRadioButton* mEnterTable;
        bool mHandleTableEdit;
        void activateExpression(bool enable);
        void addEmptyTableColumn(int icol);
    private Q_SLOTS:
        void handleExpressionToggled(bool state);
        void handleTableToggled(bool state);
        void handleTableCellChanged(int irow, int icol);
    public:
        BooleanDialog(const QList<WaveData*> inputList, QWidget* parent=nullptr);
        bool hasExpression() const;
        QString expression() const;
        QList<int> tableValues() const;
    };

}
