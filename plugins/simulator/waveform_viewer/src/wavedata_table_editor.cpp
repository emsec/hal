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

    void WavedataTableEditor::setup(const std::unordered_set<const Net *> &inpNets)
    {
        int n = inpNets.size()+1;
        setColumnCount(n);
        setRowCount(2);
        for (const Net* n : inpNets)
        {
            mInputNets.insert(QString::fromStdString(n->get_name()), n);
        }
        QStringList hlab;
        hlab << "Time";
        hlab << mInputNets.keys();
        setHorizontalHeaderLabels(hlab);
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
        int icol = 1;

        SaleaeOutputFile** sofArray;
        int* valArray;
        sofArray = new SaleaeOutputFile*[columnCount()-1];
        valArray = new int[columnCount()-1];
        QTableWidgetItem* timeCell = item(0,0);
        mMaxTime = timeCell ? timeCell->text().toULongLong() : 0;

        for (auto it = mInputNets.constBegin(); it != mInputNets.constEnd(); ++it)
        {
            SaleaeOutputFile* sof = writer->add_or_replace_waveform(it.key().toStdString(),it.value()->get_id());
            int val = intCellValue(0,icol);
            if (val == WavedataTableEditor::sIllegalValue)
                val = 0;
            sof->writeTimeValue(mMaxTime,val);
            sofArray[icol-1] = sof;
            valArray[icol-1] = val;
            ++icol;
        }

        for (int irow = 1; irow < rowCount(); irow++)
        {
            timeCell = item(irow,0);
            if (!timeCell) continue;
            uint64_t tval = timeCell->text().toULongLong();
            if (tval <= mMaxTime) continue;
            mMaxTime = tval;
            for (int i = 1; i < columnCount(); i++)
            {
                int val = intCellValue(irow,i);
                if (val == WavedataTableEditor::sIllegalValue)
                    val = 0;
                if (val != valArray[i-1])
                {
                    SaleaeOutputFile* sof = sofArray[i-1];
                    Q_ASSERT(sof);
                    sof->writeTimeValue(mMaxTime,val);
                    valArray[i-1] = val;
                }
            }
        }

        delete writer;
        delete [] sofArray;
        delete [] valArray;
    }
}
