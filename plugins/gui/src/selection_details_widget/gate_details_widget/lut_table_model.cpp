
#include "gui/selection_details_widget/gate_details_widget/lut_table_model.h"

#include "hal_core/utilities/log.h"

#include <algorithm>
#include <variant>

namespace hal
{

    LUTTableModel::LUTTableModel(QObject* parent) : QAbstractTableModel(parent)
    {
        LutEntry emptyEntry;
        emptyEntry.output = "No LUT gate selected yet...";
        mLutEntries.append(emptyEntry);
        mOutputPin = "-";
    }

    int LUTTableModel::columnCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent)
        return (mInputPins.size() + 1);
    }

    int LUTTableModel::rowCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent)
        return mLutEntries.size();
    }

    QVariant LUTTableModel::data(const QModelIndex& index, int role) const
    {
        if (role == Qt::DisplayRole)
        {
            if (index.column() < mInputPins.size())
            {
                return mLutEntries[index.row()].inputBits[index.column()];
            }
            else
            {
                return mLutEntries[index.row()].output;
            }
        }

        else if (role == Qt::TextAlignmentRole)
        {
            return Qt::AlignCenter;
        }

        return QVariant();
    }

    QVariant LUTTableModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (role != Qt::DisplayRole)
            return QVariant();
        if (orientation == Qt::Horizontal)
        {
            if (section < mInputPins.size())
                return mInputPins[section];
            else
                return mOutputPin;
        }

        return QVariant();
    }

    bool LUTTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
    {
        // TODO
        Q_UNUSED(index)
        Q_UNUSED(value)
        Q_UNUSED(role)
        return false;
    }

    void LUTTableModel::setBooleanFunction(const BooleanFunction lutFunction, const QString outputPinName)
    {
        Q_EMIT layoutAboutToBeChanged();
        mOutputPin = outputPinName;

        // Collect the variables as headers
        mInputPins.clear();
        std::vector<std::string> bfVariables = utils::to_vector(lutFunction.get_variable_names());
        std::sort(bfVariables.begin(), bfVariables.end(), std::greater<>());
        for (std::string v : bfVariables)
        {
            mInputPins.append(QString::fromStdString(v));
        }
        std::reverse(bfVariables.begin(), bfVariables.end());

        auto ttVariant = lutFunction.compute_truth_table(bfVariables, false);
        if (ttVariant.is_ok())
        {
            // can only deal with single-bit Boolean functions, but that should be the case for LUTs anyway
            std::vector<BooleanFunction::Value> truthTable = ttVariant.get().at(0);
            mLutEntries.clear();
            for (u32 truthTableIdx = 0; truthTableIdx < truthTable.size(); truthTableIdx++)
            {
                LutEntry entry;
                // Get input bits
                for (int i = 0; i < mInputPins.size(); i++)
                {
                    u32 shift   = mInputPins.size() - i - 1;
                    u8 inputBit = u8((truthTableIdx >> shift) & 1);
                    entry.inputBits.append(inputBit);
                }
                // Get output bit
                BooleanFunction::Value val = truthTable[truthTableIdx];
                if (val == BooleanFunction::Value::ZERO)
                    entry.output = "0";
                else if (val == BooleanFunction::Value::ONE)
                    entry.output = "1";
                else if (val == BooleanFunction::Value::Z)
                    entry.output = "Z";
                else
                    entry.output = "X";
                mLutEntries.append(entry);
            }
            Q_EMIT layoutChanged();
        }
    }

}    // namespace hal
