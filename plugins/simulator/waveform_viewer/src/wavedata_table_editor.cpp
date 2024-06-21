#include "waveform_viewer/wavedata_table_editor.h"
#include "hal_core/netlist/net.h"
#include "netlist_simulator_controller/saleae_writer.h"
#include "netlist_simulator_controller/saleae_file.h"
#include <QHeaderView>
#include <QDebug>
#include <QDir>

namespace hal {
    WavedataTableEditor::WavedataTableEditor(QWidget *parent)
        : QTableWidget(parent), mMaxTime(0)
    {;}

    void WavedataTableEditor::setup(const std::vector<NetlistSimulatorController::InputColumnHeader> &inpColHeads)
    {
        int n = inpColHeads.size()+1;
        setColumnCount(n);
        setRowCount(2);
        QStringList headerLabel;
        for (NetlistSimulatorController::InputColumnHeader ich : inpColHeads)
        {
            QString colName = QString::fromStdString(ich.name);
            if (ich.nets.size() > 1) colName += QString("[%1:0]").arg(ich.nets.size()-1);
            mInputColumnHeader.append(ich);
            headerLabel << colName;
        }
        headerLabel.prepend("Time");
        setHorizontalHeaderLabels(headerLabel);
        for (int i=0; i<n; i++)
        {
            setItem(0,i, new QTableWidgetItem("0"));
        }
        connect(this, &QTableWidget::itemChanged, this, &WavedataTableEditor::handleItemChanged);
    }

    void WavedataTableEditor::handleItemChanged(QTableWidgetItem* changedItem)
    {
        int cur = rowCount() - 1;
        if (cur > 0 && !changedItem->column() && changedItem->row() == cur)
        {
            setRowCount(cur+2);
            for (int icol=1; icol < columnCount(); icol++)
            {
               QTableWidgetItem* testItem = item(cur, icol);
               QTableWidgetItem* lastItem = item(cur-1, icol);
               if ((!testItem || testItem->text().isEmpty()) && lastItem)
                   setItem(cur,icol, lastItem->clone());
            }
        }
        if (!changedItem->column())
        {
            int irow = changedItem->row();
            if (cur > 0 && irow == cur)
            {
                setRowCount(cur+2);
                for (int icol=1; icol < columnCount(); icol++)
                {
                   QTableWidgetItem* testItem = item(cur, icol);
                   QTableWidgetItem* lastItem = item(cur-1, icol);
                   if ((!testItem || testItem->text().isEmpty()) && lastItem)
                       setItem(cur,icol, lastItem->clone());
                }
                Q_EMIT lineAdded();
            }
            bool numericOk = false;
            qlonglong nextval = changedItem->text().toLongLong(&numericOk);
            qlonglong prevval = irow ? item(irow-1,0)->text().toLongLong() : 0;
            if (!numericOk || nextval <= prevval)
                changedItem->setText(QString::number(prevval+1000));
        }
        else
        {
            if (intCellValue(changedItem->row(),changedItem->column()) == sIllegalValue)
                changedItem->setText("0");
        }
    }

    int WavedataTableEditor::validLines() const
    {
        int retval = 0;
        qlonglong tlast = -1;
        bool numericOk = false;
        for (int irow = 0; irow < rowCount(); irow++)
        {
            QTableWidgetItem* timeCell = item(irow,0);
            if (!timeCell) continue;
            qlonglong t = timeCell->text().toLongLong(&numericOk);
            if (!numericOk) continue;
            if (t <= tlast) continue;
            bool incomplete = false;
            for (int icol=1; icol<columnCount(); icol++)
            {
                QTableWidgetItem* testItem = item(irow,icol);
                if (!testItem || testItem->text().isEmpty())
                {
                    incomplete = true;
                    break;
                }
            }
            if (incomplete) continue;
            ++retval;
            tlast = t;
        }
        return retval;
    }

    int WavedataTableEditor::intCellValue(int irow, int icol) const
    {
        QString txt;
        if (item(irow,icol)) txt = item(irow,icol)->text();
        if (txt.size() != 1) return sIllegalValue;

        switch (txt.at(0).unicode())
        {
        case 'Z':
        case 'z':
            return -2;
        case 'X':
        case 'x':
            return -1;
        case '0':
            return 0;
        case '1':
            return 1;
        }

        return sIllegalValue;
    }

    void WavedataTableEditor::generateSimulationInput(const QString& workdir)
    {
        QDir saleaeDir = QDir(workdir).absoluteFilePath("saleae");
        QString saleaeDirectoryFilename = saleaeDir.absoluteFilePath("saleae.json");
        SaleaeWriter* writer = new SaleaeWriter(saleaeDirectoryFilename.toStdString());
        int iTableCol = 1;
        QTableWidgetItem* timeCell = item(0,0);
        mMaxTime = timeCell ? timeCell->text().toULongLong() : 0;
        int sofIndex = 0;
        int maxIndex = 0;
        for (auto it = mInputColumnHeader.constBegin(); it != mInputColumnHeader.constEnd(); ++it)
            maxIndex += it->nets.size();

        SaleaeOutputFile** sofArray;
        int* valArray;
        sofArray = new SaleaeOutputFile*[maxIndex];
        valArray = new int[maxIndex];

        for (auto it = mInputColumnHeader.constBegin(); it != mInputColumnHeader.constEnd(); ++it)
        {
            int tableVal = intCellValue(0,iTableCol);
            if (tableVal == WavedataTableEditor::sIllegalValue)
                tableVal = 0;
            int mask = 1;
            int iBit = 0;
            for (const Net* n : it->nets)
            {
                SaleaeOutputFile* sof = writer->add_or_replace_waveform(n->get_name(),n->get_id());
                int netVal = tableVal & mask;
                sof->writeTimeValue(mMaxTime,netVal);
                sofArray[sofIndex + iBit] = sof;
                valArray[sofIndex + iBit] = netVal;
                mask <<= 1;
                ++iBit;
            }
            ++iTableCol;
            sofIndex += it->nets.size();
        }

        for (int irow = 1; irow < rowCount(); irow++)
        {
            sofIndex = 0;
            timeCell = item(irow,0);
            if (!timeCell) continue;
            uint64_t tval = timeCell->text().toULongLong();
            if (tval <= mMaxTime) continue;
            mMaxTime = tval;
            iTableCol = 1;
            for (auto it = mInputColumnHeader.constBegin(); it != mInputColumnHeader.constEnd(); ++it)
            {
                int tableVal = intCellValue(irow,iTableCol);
                if (tableVal == WavedataTableEditor::sIllegalValue)
                    tableVal = 0;
                int mask = 1;

                for (int iBit = 0; iBit < it->nets.size(); ++iBit)
                {
                    int netVal = tableVal & mask;
                    if (netVal != valArray[sofIndex+iBit])
                    {
                        SaleaeOutputFile* sof = sofArray[sofIndex + iBit];
                        Q_ASSERT(sof);
                        sof->writeTimeValue(mMaxTime,netVal);
                        valArray[sofIndex + iBit] = netVal;
                    }
                    mask <<= 1;
                }
                ++iTableCol;
                sofIndex += it->nets.size();
            }
        }

        delete writer;
        delete [] sofArray;
        delete [] valArray;
    }
}
