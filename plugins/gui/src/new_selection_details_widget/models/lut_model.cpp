
#include <algorithm>

#include "gui/new_selection_details_widget/models/lut_model.h"
#include "hal_core/utilities/log.h"


namespace hal
{

    LutModel::LutModel(QObject* parent) : QAbstractTableModel(parent){
        LutEntry emptyEntry;
        emptyEntry.output = "Error: Not initialized yet";
        mLutEntries.append(emptyEntry);
        mHeaderPins.append("-");
        mInputCount = 0;
        mOutPinName = "-";
    }

    int LutModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent)
        return (mInputCount + 1);
    }

    int LutModel::rowCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent)
        return mLutEntries.size();
    }

    QVariant LutModel::data(const QModelIndex &index, int role) const
    {
        if (role != Qt::DisplayRole) return QVariant();
        if(index.column() < mInputCount){
            return mLutEntries[index.row()].inputBits[index.column()];
        }
        else{
            return mLutEntries[index.row()].output;
        }

    }

    QVariant LutModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (role != Qt::DisplayRole) return QVariant();
        if(orientation == Qt::Horizontal)
        {
            if (section < mInputCount)
                return mHeaderPins[section];
            else
                return mOutPinName;
        }

        return QVariant();
    }

    bool LutModel::setData(const QModelIndex &index, const QVariant &value, int role)
    {
        // TODO
        Q_UNUSED(index)
        Q_UNUSED(value)
        Q_UNUSED(role)
        return false;
    }

    void LutModel::update(Gate* gate)
    {
        std::unordered_set<std::basic_string<char>> lutPins = gate->get_type()->get_pins_of_type(PinType::lut);
        if(lutPins.size() == 0){
            log_error("gui", "LutModel was initialized with non-LUT gate!");
            return;
        }
        // All LUT pins have the same boolean function
        std::basic_string<char> outPin = *lutPins.begin();
        BooleanFunction lutFunction = gate->get_boolean_function(outPin);

        Q_EMIT layoutAboutToBeChanged();
        mOutPinName = QString::fromStdString(outPin);

        // Collect the variables as headers
        mHeaderPins.clear();
        for(std::string v : lutFunction.get_variables()){
            mHeaderPins.append(QString::fromStdString(v));
        }

        // Collect the lut data
        std::vector<std::string> bfVariables = lutFunction.get_variables();
        mInputCount = bfVariables.size();
        std::reverse(bfVariables.begin(), bfVariables.end()); // To read the lut the variable list must be reversed

        std::vector<BooleanFunction::Value> truthTable = lutFunction.get_truth_table(bfVariables, false);
        mLutEntries.clear();
        for(u32 truthTableIdx = 0; truthTableIdx < truthTable.size(); truthTableIdx++){
            LutEntry entry;
            // Get input bits
            for(int i = 0; i < mInputCount; i++){
                u32 shift = mInputCount-i-1;
                u8 inputBit = u8((truthTableIdx >> shift) & 1);
                entry.inputBits.append(inputBit);
            }
            // Get output bit
            BooleanFunction::Value val = truthTable[truthTableIdx];
            if(val == BooleanFunction::Value::ZERO)
                entry.output = "0";
            else if(val == BooleanFunction::Value::ONE)
                entry.output = "1";
            else if(val == BooleanFunction::Value::Z)
                entry.output = "Z";
            else
                entry.output = "X";
            mLutEntries.append(entry);
        }
        Q_EMIT layoutChanged();
    }

} // namespace hal


