
#include <gui/include/gui/new_selection_details_widget/models/boolean_function_table_model.h>

#include "gui/new_selection_details_widget/models/lut_table_model.h"
#include "hal_core/utilities/log.h"

namespace hal {

    BooleanFunctionTableModel::BooleanFunctionTableModel(QObject *parent) : QAbstractTableModel(parent)
    {
        mLatchOrFFFunctions.clear();
        mCustomFunctions.clear();
        mIncludeCSBehaviour = false;
    }

    int BooleanFunctionTableModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return 2;
    }

    int BooleanFunctionTableModel::rowCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        int rCount = mLatchOrFFFunctions.size() + mCustomFunctions.size();
        if(mIncludeCSBehaviour)
            rCount += 1;
        return rCount;
    }

    QVariant BooleanFunctionTableModel::data(const QModelIndex &index, int role) const
    {
        int row = index.row();
        int col = index.column();

        if (role == Qt::DisplayRole){
            // Find the entry
            BooleanFunctionTableEntry entry;
            if(row < mLatchOrFFFunctions.size())
            {
                entry = mLatchOrFFFunctions[row];
            }
            // The Custom Functions are displayed under the Latch/FF functions. The indices must be computed accordingly.
            else if(row < (mLatchOrFFFunctions.size() + mCustomFunctions.size()))
            {
                entry = mCustomFunctions[(row - mLatchOrFFFunctions.size())];
            }
            else
            {
                entry = mCSBehaviour;
            }

            // Extract the information from the entry based on the column
            if(col == 0)
            {
                return (entry.functionName + " =");
            }
            else
            {
                return entry.functionString;
            }
        }

        else if (role == Qt::TextAlignmentRole)
        {
            if(col == 0){
                return QVariant(Qt::AlignTop | Qt::AlignRight);
            }
            else
            {
                return QVariant(Qt::AlignTop | Qt::AlignLeft);
            }
            return Qt::AlignLeft;
        }

        return QVariant();
    }

    QVariant BooleanFunctionTableModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        Q_UNUSED(section);
        Q_UNUSED(orientation);
        Q_UNUSED(role);
        return QVariant(); // No Header
    }

    bool BooleanFunctionTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
    {
        Q_UNUSED(index);
        Q_UNUSED(value);
        Q_UNUSED(role);
        return false;
    }

    QPair<QString, bool> BooleanFunctionTableModel::getBooleanFunctionNameAtRow(int row) const
    {
        BooleanFunctionTableEntry entry;
        bool isClearSetBehavior = false;
        if(row < mLatchOrFFFunctions.size())
        {
            entry = mLatchOrFFFunctions[row];
        }
        // The Custom Functions are under the Latch/FF functions. The indices must be computed accordingly.
        else if(row < (mLatchOrFFFunctions.size() + mCustomFunctions.size()))
        {
            entry = mCustomFunctions[(row - mLatchOrFFFunctions.size())];
        }
        else
        {
            isClearSetBehavior = true;
            entry = mCSBehaviour;
        }

        return QPair<QString, bool>(entry.functionName, isClearSetBehavior);
    }

    void BooleanFunctionTableModel::setBooleanFunctionList(const QMap<QString, BooleanFunction>& latchOrFFFunctions,
                                                           const QMap<QString, BooleanFunction>& customFunctions,
                                                           std::pair<GateType::ClearPresetBehavior, GateType::ClearPresetBehavior> cPBehaviour)
    {
        Q_EMIT layoutAboutToBeChanged();
        mLatchOrFFFunctions = getEntryListFromMap(latchOrFFFunctions);
        mCustomFunctions = getEntryListFromMap(customFunctions);

        mIncludeCSBehaviour = true;
        mCSBehaviour = getEntryFromCPBehaviour(cPBehaviour);
        Q_EMIT layoutChanged();
    }

    void BooleanFunctionTableModel::setBooleanFunctionList(const QMap<QString, BooleanFunction>& latchOrFFFunctions,
                                                           const QMap<QString, BooleanFunction>& customFunctions)
    {
        Q_EMIT layoutAboutToBeChanged();
        mLatchOrFFFunctions = getEntryListFromMap(latchOrFFFunctions);
        mCustomFunctions = getEntryListFromMap(customFunctions);

        mIncludeCSBehaviour = false;
        Q_EMIT layoutChanged();
    }

    void BooleanFunctionTableModel::setSingleBooleanFunction(QString functionName, BooleanFunction booleanFunction)
    {
        Q_EMIT layoutAboutToBeChanged();
        mLatchOrFFFunctions.clear();
        mCustomFunctions.clear();

        BooleanFunctionTableEntry entry;
        entry.functionName = functionName;
        entry.functionString = QString::fromStdString(booleanFunction.to_string());
        mCustomFunctions.append(entry);

        mIncludeCSBehaviour = false;
        Q_EMIT layoutChanged();
    }

    QList<BooleanFunctionTableModel::BooleanFunctionTableEntry> BooleanFunctionTableModel::getEntryListFromMap(const QMap<QString, BooleanFunction>& map)
    {
        QList<BooleanFunctionTableEntry> entryList;
        for(auto it = map.constBegin(); it != map.constEnd(); it++){
            BooleanFunctionTableEntry newEntry;
            newEntry.functionName = it.key();
            newEntry.functionString = QString::fromStdString(it.value().to_string());
            entryList.append(newEntry);
        }
        // Sort the entries after their function name
        qSort(entryList.begin(), entryList.end(),
              [](BooleanFunctionTableEntry a, BooleanFunctionTableEntry b) -> bool { return a.functionName < b.functionName;});

        return entryList;
    }

    BooleanFunctionTableModel::BooleanFunctionTableEntry BooleanFunctionTableModel::getEntryFromCPBehaviour(
        std::pair<GateType::ClearPresetBehavior, GateType::ClearPresetBehavior> cPBehaviour)
    {
        static QMap<GateType::ClearPresetBehavior, QString> cPBehaviourToString {
            {GateType::ClearPresetBehavior::L, "L"},
            {GateType::ClearPresetBehavior::H, "H"},
            {GateType::ClearPresetBehavior::N, "N"},
            {GateType::ClearPresetBehavior::T, "T"},
            {GateType::ClearPresetBehavior::X, "X"},
            {GateType::ClearPresetBehavior::undef, "Undefined"},
        };

        BooleanFunctionTableEntry e;

        e.functionName = "set_clear_behaviour";
        e.functionString = cPBehaviourToString[cPBehaviour.first] + ", " + cPBehaviourToString[cPBehaviour.second];
        return e;
    }

}// namespace hal
