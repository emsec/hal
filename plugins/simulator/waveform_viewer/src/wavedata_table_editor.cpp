#include "waveform_viewer/wavedata_table_editor.h"
#include "hal_core/netlist/net.h"
#include "netlist_simulator_controller/saleae_writer.h"
#include "netlist_simulator_controller/saleae_file.h"
#include "netlist_simulator_controller/saleae_parser.h"
#include <QHeaderView>
#include <QDebug>
#include <QDir>

namespace hal {
    WavedataTableEditor::WavedataTableEditor(QWidget *parent)
    : QTableWidget(parent), mMaxTime(0), mDisableCellParser(false), mDisplayHexValues(true), mCurrentTime(0), mCurrentRow(0)
    {;}

    void WavedataTableEditor::setup(const std::vector<NetlistSimulatorController::InputColumnHeader> &inpColHeads, bool omitClock)
    {
        for (NetlistSimulatorController::InputColumnHeader ich : inpColHeads)
        {
            if (omitClock && ich.is_clock) continue;
            mInputColumnHeader.append(ich);
        }

        setupHeader();

        setRowCount(2);
        int n = mInputColumnHeader.size()+1;
        for (int i=0; i<n; i++) setValueCell(0,i,0);

        // Must not change start time t=0
        item(0,0)->setFlags(item(0,0)->flags() & (~Qt::ItemIsEditable));
        connect(this, &QTableWidget::itemChanged, this, &WavedataTableEditor::handleItemChanged);
    }

    void WavedataTableEditor::setupHeader()
    {
        QStringList headerLabel;
        for (NetlistSimulatorController::InputColumnHeader ich : mInputColumnHeader)
        {
            QString colName = QString::fromStdString(ich.name);
            if (ich.nets.size() > 1) colName += QString("[%1:0]").arg(ich.nets.size()-1);
            headerLabel << colName;
        }
        int n = mInputColumnHeader.size()+1;
        setColumnCount(n);
        headerLabel.prepend("Time");
        setHorizontalHeaderLabels(headerLabel);
    }

    void WavedataTableEditor::setValueCell(int irow, int icol, int val)
    {
        QTableWidgetItem* valItem = new QTableWidgetItem(val > 9 && mDisplayHexValues
                                                             ? "0x" + QString::number(val,16)
                                                             : QString::number(val));
        valItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        setItem (irow, icol, valItem);
    }

    void WavedataTableEditor::loadWaveData(const QString& saleaDirectoryFile)
    {
        mDisableCellParser = true;
        SaleaeParser parser(saleaDirectoryFile.toStdString());

        mCurrentTime = 0;
        mCurrentRow = 0;
        int nrows = 1;
        int ncols = columnCount() - 1; // columns with values
        mCurrentValue = new int[ncols];
        memset(mCurrentValue,0,ncols*sizeof(int));

        for (int icol = 0; icol < ncols; icol++)
        {
            const NetlistSimulatorController::InputColumnHeader& ich = mInputColumnHeader.at(icol);

            int nBits = ich.nets.size();

            for (int i=0; i<nBits; i++)
            {
                int mask = 1 << i;

                const Net* n = ich.nets.at(i);

                parser.register_callback(n->get_name(), n->get_id(), [icol,mask,ncols,this](const void* obj, uint64_t t, int val) {
                    if (t != mCurrentTime)
                    {
                        mCurrentRow++;
                        if (mCurrentRow+1 >= rowCount()) setRowCount(mCurrentRow+2);
                        QTableWidgetItem* tItem = new QTableWidgetItem(QString::number(t));
                        tItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                        setItem(mCurrentRow,0,tItem);
                        mCurrentTime = t;
                        for (int i=0; i<ncols; i++)
                            setValueCell(mCurrentRow-1, i+1, mCurrentValue[i]);
                    }

                    if (val)
                        mCurrentValue[icol] |= mask;
                    else
                        mCurrentValue[icol] &= (~mask);
                }, nullptr);
            }
        }

        clear();
        setupHeader();
        setRowCount(2);
        QTableWidgetItem* zeroItem = new QTableWidgetItem("0");
        zeroItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        zeroItem->setFlags(zeroItem->flags() & (~Qt::ItemIsEditable));
        setItem(0,0,zeroItem);

        while (parser.next_event()) {;} // loop over entries calling callback

        for (int i=0; i<ncols; i++)
            setValueCell(mCurrentRow, i+1, mCurrentValue[i]);

        delete [] mCurrentValue;

        mDisableCellParser = false;
    }

    void WavedataTableEditor::handleItemChanged(QTableWidgetItem* changedItem)
    {
        if (mDisableCellParser) return;

        mDisableCellParser = true;
        int cur = rowCount() - 1;
        if (cur > 0 && !changedItem->column() && changedItem->row() == cur)
        {
            setRowCount(cur+2);
            for (int icol=1; icol < columnCount(); icol++)
            {
                QTableWidgetItem* testItem = item(cur, icol);
                QTableWidgetItem* lastItem = item(cur-1, icol);
                if ((!testItem || testItem->text().isEmpty()) && lastItem)
                {
                    QTableWidgetItem* twi = lastItem->clone();
                    twi->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    setItem(cur, icol, twi);
                }
            }
        }
        if (!changedItem->column())
        {
            // time column
            int irow = changedItem->row();
            if (cur > 0 && irow == cur)
            {
                setRowCount(cur+2);
                for (int icol=1; icol < columnCount(); icol++)
                {
                   QTableWidgetItem* testItem = item(cur, icol);
                   QTableWidgetItem* lastItem = item(cur-1, icol);
                   if ((!testItem || testItem->text().isEmpty()) && lastItem)
                   {
                       QTableWidgetItem* twi = lastItem->clone();
                       twi->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                       setItem(cur,icol, twi);
                   }
                }
                Q_EMIT lineAdded();
            }
            bool numericOk = false;
            qlonglong thisVal = changedItem->text().toLongLong(&numericOk);
            qlonglong prevVal = irow ? item(irow-1,0)->text().toLongLong() : 0;
            qlonglong nextVal   = 0;
            if (rowCount() > irow+1 && item(irow+1,0))
            {
                qlonglong tempNextVal = item(irow+1,0)->text().toLongLong(&numericOk);
                if (numericOk && tempNextVal >= prevVal+2) nextVal = tempNextVal;
            }
            if (nextVal)
            {
                // changed time value has successor
                if (!numericOk || thisVal <= prevVal || thisVal >= nextVal)
                    changedItem->setText(QString::number((prevVal+nextVal)/2));
            }
            else
            {
                // changed last time value
                if (!numericOk || thisVal <= prevVal)
                    changedItem->setText(QString::number(prevVal+1000));
            }
        }
        else
        {
            // wave value column
            int val = intCellValue(changedItem->row(),changedItem->column());
            if (val == sIllegalValue)
                changedItem->setText("0");
            else
            {
                if (val > 9 && mDisplayHexValues)
                    changedItem->setText("0x" + QString::number(val,16));
                else
                    changedItem->setText(QString::number(val,10));
            }
        }
        changedItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        mDisableCellParser = false;
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
        int nBits = mInputColumnHeader.at(icol-1).nets.size();
        int upperLimit = 1 << nBits;

        if (item(irow,icol))
            txt = item(irow,icol)->text();
        if (txt.isEmpty())
            return sIllegalValue;

        bool numberOk = true;
        int tempValue = -1;

        // most common input 0,1,x,z
        if (txt.size() == 1)
        {
            switch (txt.at(0).toUpper().unicode())
            {
            case 'Z':
                return -2;
            case 'X':
                return -1;
            default:
                // legal values 0-9
                tempValue = txt.toInt(&numberOk);
                if (!numberOk || tempValue >= upperLimit) return sIllegalValue;
                return tempValue;
            }
        }

        // txt size at least 2
        if (txt.at(0) == '"')
        {
            // enter ASCII character, eg. "A -> 0x41 (65)
            tempValue = txt.at(1).unicode();
        }
        else
        {
            int apos = txt.indexOf('\'');
            if (apos < 0)
            {
                if (txt.at(0) == '0')
                {
                    switch (txt.at(1).toUpper().unicode())
                    {
                    case 'X':
                        tempValue = txt.mid(2).toInt(&numberOk, 16);
                        break;
                    case 'O':
                        tempValue = txt.mid(2).toInt(&numberOk, 8);
                        break;
                    case 'B':
                        tempValue = txt.mid(2).toInt(&numberOk, 2);
                        break;
                    default:
                        tempValue = txt.toInt(&numberOk, 10);
                        break;
                    }
                }
                else
                    tempValue = txt.toInt(&numberOk, 10);
            }
            else
            {
                if (apos + 2 >= txt.size())
                    return sIllegalValue;
                switch (txt.at(apos+1).toUpper().unicode())
                {
                case 'H':
                    tempValue = txt.mid(apos+2).toInt(&numberOk, 16);
                    break;
                case 'O':
                    tempValue = txt.mid(apos+2).toInt(&numberOk, 8);
                    break;
                case 'B':
                    tempValue = txt.mid(apos+2).toInt(&numberOk, 2);
                    break;
                case 'D':
                    tempValue = txt.mid(apos+2).toInt(&numberOk, 10);
                    break;
                default:
                    return sIllegalValue;
                }
            }
        }

        if (!numberOk || tempValue >= upperLimit)
            return sIllegalValue;

        return tempValue;
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

    void WavedataTableEditor::setDisplayHexValues(bool hex)
    {
        if (hex == mDisplayHexValues) return; // nothing to do;

        mDisableCellParser = true;
        mDisplayHexValues = hex;

        for (int icol = 1; icol < columnCount(); icol++)
            for (int irow = 0; irow < rowCount(); irow++)
            {
                QTableWidgetItem* cell = item(irow,icol);
                if (!cell) continue;
                int val = intCellValue(irow,icol);
                QString txt = QString::number(val, mDisplayHexValues ? 16 : 10);
                if (mDisplayHexValues && val > 9) txt.prepend("0x");
                cell->setText(txt);
            }

        mDisableCellParser = false;
    }

}
