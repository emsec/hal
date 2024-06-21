#pragma once
#include <QTableWidget>
#include <unordered_set>
#include <QMap>
#include "netlist_simulator_controller/netlist_simulator_controller.h"

namespace hal {

    class Net;

    class WavedataTableEditor : public QTableWidget
    {
        Q_OBJECT

        QList<NetlistSimulatorController::InputColumnHeader> mInputColumnHeader;
        qulonglong mMaxTime;

        static const int sIllegalValue = -99;
    Q_SIGNALS:
        void lineAdded();

    private Q_SLOTS:
        void handleItemChanged(QTableWidgetItem* changedItem);

    public:
        WavedataTableEditor(QWidget* parent = nullptr);

        void setup(const std::vector<NetlistSimulatorController::InputColumnHeader>& inpColHeads);

        int validLines() const;

        void generateSimulationInput(const QString& workdir); // will update mMaxTime

        int intCellValue(int irow, int icol) const;

        qulonglong maxTime() const { return mMaxTime; }
    };

}
