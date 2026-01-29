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
        bool mDisableCellParser;
        bool mDisplayHexValues;

        // member variables for load data callback
        uint64_t mCurrentTime;
        int mCurrentRow;
        int* mCurrentValue;

        static const int sIllegalValue = -99;
        void setValueCell(int irow, int icol, int val);
        void setupHeader();

    Q_SIGNALS:
        void lineAdded();

    public Q_SLOTS:
        void setDisplayHexValues(bool hex);

    private Q_SLOTS:
        void handleItemChanged(QTableWidgetItem* changedItem);

    public:
        WavedataTableEditor(QWidget* parent = nullptr);

        void setup(const std::vector<NetlistSimulatorController::InputColumnHeader>& inpColHeads, bool omitClock);

        int validLines() const;

        void generateSimulationInput(const QString& workdir); // will update mMaxTime

        int intCellValue(int irow, int icol) const;

        qulonglong maxTime() const { return mMaxTime; }

        bool isDisplayHexValues() const { return mDisplayHexValues; }

        void loadWaveData(const QString& saleaDirectoryFile);
    };

}
