#pragma once

#include <QDialog>
#include <QList>
#include "netlist_simulator_controller/wave_data.h"

class QComboBox;
class QTableWidget;
class QRadioButton;

namespace hal {
    class WaveData;

    class TriggerDialog : public QDialog
    {
        Q_OBJECT
        QTableWidget* mTableWidget;
        QRadioButton* mEnterFilter;
        QList<WaveData*> mFilterList;
        QComboBox* mSelectFilter;
        bool mHandleTableEdit;
    private Q_SLOTS:
        void handleFilterToggled(bool state);
        void handleTableCellChanged(int irow, int icol);
    public:
        TriggerDialog(const QList<WaveData*> inputList, const QList<WaveData*> filterList, QWidget* parent=nullptr);
        QList<int> transitionToValue() const;
        WaveData* filterWave() const;
    };
}
