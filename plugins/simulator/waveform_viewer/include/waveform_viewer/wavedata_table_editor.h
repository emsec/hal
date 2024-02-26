#pragma once
#include <QTableWidget>
#include <unordered_set>
#include <QMap>

namespace hal {

    class Net;

    class WavedataTableEditor : public QTableWidget
    {
        Q_OBJECT

        QMap<QString, const Net*> mInputNets;
        qulonglong mMaxTime;

        static const int sIllegalValue = -99;
    Q_SIGNALS:
        void lineAdded();

    private Q_SLOTS:
        void handleItemChanged(QTableWidgetItem* changedItem);

    public:
        WavedataTableEditor(QWidget* parent = nullptr);

        void setup(const std::unordered_set<const Net*>& inpNets);

        int validLines() const;

        void generateSimulationInput(const QString& workdir); // will update mMaxTime

        int intCellValue(int irow, int icol) const;

        qulonglong maxTime() const { return mMaxTime; }
    };

}
