
#include <gui/include/gui/selection_details_widget/gate_details_widget/boolean_function_table_model.h>

#include "gui/selection_details_widget/gate_details_widget/lut_table_model.h"
#include "gui/python/py_code_provider.h"
#include "hal_core/utilities/log.h"

namespace hal {


    /* ========================================================
     * BooleanFunctionTableEntry - Superclass
     * ========================================================*/

    BooleanFunctionTableEntry::BooleanFunctionTableEntry(u32 gateId)
    {
        mGateId = gateId;
        mType = EntryType::BooleanFunctionStandard;
    }

    QString BooleanFunctionTableEntry::getEntryIdentifier() const
    {
        return mLeft;
    }

    QString BooleanFunctionTableEntry::getEntryValueString() const
    {
        return mRight;
    }

    u32 BooleanFunctionTableEntry::getGateId() const {
        return mGateId;
    }

    BooleanFunctionTableEntry::EntryType BooleanFunctionTableEntry::getEntryType() const
    {
        return mType;
    }

    /* ========================================================
     * BooleanFunctionTableEntry - BooleanFunctionEntry
     * ========================================================*/

    BooleanFunctionEntry::BooleanFunctionEntry(u32 gateId, QString functionName, BooleanFunction bf)
    : BooleanFunctionTableEntry(gateId)
    {
        mLeft = functionName;
        mRight = QString::fromStdString(bf.to_string());
        mType = EntryType::BooleanFunctionStandard;
    }

    BooleanFunctionEntry::BooleanFunctionEntry(QString functionName, BooleanFunction bf) : BooleanFunctionTableEntry(0)
    {
        //set the gateId to 0, indicating that this entry is an arbitrary boolean function
        mLeft = functionName;
        mRight = QString::fromStdString(bf.to_string());
        mType = EntryType::BooleanFunctionStandard;
    }

    BooleanFunction BooleanFunctionEntry::getBooleanFunction() const
    {
        return mBF;
    }

    QString BooleanFunctionEntry::getPythonCode()
    {
        if(mGateId == 0)
            return "";
        else
            return PyCodeProvider::pyCodeGateBooleanFunction(mGateId, mLeft);
    }

    /* ========================================================
     * BooleanFunctionTableEntry - CPBehaviorEntry
     * ========================================================*/

    CPBehaviorEntry::CPBehaviorEntry(u32 gateId, std::pair<hal::AsyncSetResetBehavior, hal::AsyncSetResetBehavior> cPBehavior)
    : BooleanFunctionTableEntry(gateId)
    {
        mLeft = "set_reset_behavior";
        mRight = cPBehaviourToString(cPBehavior);
        mCPBehavior =cPBehavior;
        mType = EntryType::CPBehavior;
    }

    std::pair<hal::AsyncSetResetBehavior, hal::AsyncSetResetBehavior> CPBehaviorEntry::getCPBehavior() const
    {
        return mCPBehavior;
    }

    QString CPBehaviorEntry::cPBehaviourToString (std::pair<hal::AsyncSetResetBehavior, hal::AsyncSetResetBehavior> cPBehaviour)
    {
        static QMap<hal::AsyncSetResetBehavior, QString> cPBehaviourToString {
            {hal::AsyncSetResetBehavior::L, "L"},
            {hal::AsyncSetResetBehavior::H, "H"},
            {hal::AsyncSetResetBehavior::N, "N"},
            {hal::AsyncSetResetBehavior::T, "T"},
            {hal::AsyncSetResetBehavior::X, "X"},
            {hal::AsyncSetResetBehavior::undef, "Undefined"},
        };
        return QString(cPBehaviourToString[cPBehaviour.first] + ", " + cPBehaviourToString[cPBehaviour.second]);
    }

    StateComponentEntry::StateComponentEntry(u32 gateId, StateCompType type, QString stateVal) : BooleanFunctionTableEntry(gateId)
    {
        mType = EntryType::StateComp;
        specificType = type;
        mLeft = enumTypeToString();
        mRight = stateVal;
    }

    QString StateComponentEntry::getPythonCode()
    {
        switch (specificType)
        {
            case StateCompType::NegState: return PyCodeProvider::pyCodeStateCompNegState(mGateId); break;
            case StateCompType::PosState: return PyCodeProvider::pyCodeStateCompPosState(mGateId); break;
            default: return "";
        }
    }

    QString StateComponentEntry::enumTypeToString()
    {
        const QString types[] = {"internal_state", "neg_internal_state"};
        return types[specificType];
    }

    /* ========================================================
     * BooleanFunctionTableModel
     * ========================================================*/

    BooleanFunctionTableModel::BooleanFunctionTableModel(QObject *parent) : QAbstractTableModel(parent)
    {
        mEntries.clear();
    }

    int BooleanFunctionTableModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return 3;
    }

    int BooleanFunctionTableModel::rowCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return mEntries.size();
    }

    QVariant BooleanFunctionTableModel::data(const QModelIndex &index, int role) const
    {
        int row = index.row();
        int col = index.column();

        if (role == Qt::DisplayRole){

            // Extract the information from the entry based on the column
            if(col == 0)
            {
                return (mEntries[row]->getEntryIdentifier());
            }
            else if(col == 1)
            {
                return mSeparator;
            }
            else
            {
                return (mEntries[row]->getEntryValueString());
            }
        }

        else if (role == Qt::TextAlignmentRole)
        {
            if(col == 0)
            {
                return QVariant(Qt::AlignTop | Qt::AlignLeft);
            }
            else if(col == 1)
            {
                return QVariant(Qt::AlignTop | Qt::AlignHCenter);
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

    QSharedPointer<BooleanFunctionTableEntry> BooleanFunctionTableModel::getEntryAtRow(int row) const
    {
        return mEntries[row];
    }

    void BooleanFunctionTableModel::setEntries(QVector<QSharedPointer<BooleanFunctionTableEntry>> entries){
        Q_EMIT layoutAboutToBeChanged();
        mEntries = entries;
        Q_EMIT layoutChanged();
    }

    FFComponentEntry::FFComponentEntry(u32 gateId, FFComponentEntry::FFCompFunc type, BooleanFunction func) : BooleanFunctionTableEntry(gateId)
    {
        mType = EntryType::FFComp;
        mSpecificType = type;
        mLeft = enumToString();
        mRight = QString::fromStdString(func.to_string());
    }

    FFComponentEntry::FFComponentEntry(u32 gateId, std::pair<AsyncSetResetBehavior, AsyncSetResetBehavior> behav) : BooleanFunctionTableEntry(gateId)
    {
        mType = EntryType::FFComp;
        mSpecificType = FFCompFunc::SetResetBehav;
        mLeft = enumToString();
        mRight = behaviorToString(behav);
    }

    QString FFComponentEntry::getPythonCode()
    {
        switch (mSpecificType)
        {
            case FFCompFunc::Clock: return PyCodeProvider::pyCodeFFCompClockFunc(mGateId);
            case FFCompFunc::AsyncSet: return PyCodeProvider::pyCodeFFCompAsyncSetFunc(mGateId);
            case FFCompFunc::NextState: return PyCodeProvider::pyCodeFFCompNextStateFunc(mGateId);
            case FFCompFunc::AsyncReset: return PyCodeProvider::pyCodeFFCompAsyncResetFunc(mGateId);
            case FFCompFunc::SetResetBehav: return PyCodeProvider::pyCodeFFCompSetResetBehav(mGateId);
            default: return "";
        }
    }

    QString FFComponentEntry::enumToString()
    {
        static QString types[] = {"clock", "next_state", "async_set", "async_reset", "set_reset_behavior"};
        return types[mSpecificType];
    }

    QString FFComponentEntry::behaviorToString(std::pair<AsyncSetResetBehavior, AsyncSetResetBehavior> behav)
    {
        static QMap<hal::AsyncSetResetBehavior, QString> cPBehaviourToString {
            {hal::AsyncSetResetBehavior::L, "L"},
            {hal::AsyncSetResetBehavior::H, "H"},
            {hal::AsyncSetResetBehavior::N, "N"},
            {hal::AsyncSetResetBehavior::T, "T"},
            {hal::AsyncSetResetBehavior::X, "X"},
            {hal::AsyncSetResetBehavior::undef, "Undefined"},
        };
        return QString(cPBehaviourToString[behav.first] + ", " + cPBehaviourToString[behav.second]);
    }

    LatchComponentEntry::LatchComponentEntry(u32 gateId, LatchComponentEntry::LatchCompFunc type, BooleanFunction func) : BooleanFunctionTableEntry(gateId)
    {
        mType = EntryType::LatchComp;
        mSpecificType = type;
        mLeft = enumToString();
        mRight = QString::fromStdString(func.to_string());
    }

    LatchComponentEntry::LatchComponentEntry(u32 gateId, std::pair<AsyncSetResetBehavior, AsyncSetResetBehavior> behav) : BooleanFunctionTableEntry(gateId)
    {
        mType = EntryType::LatchComp;
        mSpecificType = LatchCompFunc::SetResetBehav;
        mLeft = enumToString();
        mRight = behaviorToString(behav);
    }

    QString LatchComponentEntry::getPythonCode()
    {
        switch (mSpecificType)
        {
            case LatchCompFunc::Enable: return PyCodeProvider::pyCodeLatchCompEnableFunc(mGateId);
            case LatchCompFunc::AsyncSet: return PyCodeProvider::pyCodeLatchCompAsyncSetFunc(mGateId);
            case LatchCompFunc::AsyncReset: return PyCodeProvider::pyCodeLatchCompAsyncResetFunc(mGateId);
            case LatchCompFunc::DataInFunc: return PyCodeProvider::pyCodeLatchCompDataInFunc(mGateId);
            case LatchCompFunc::SetResetBehav: return PyCodeProvider::pyCodeLatchCompSetResetBehav(mGateId);
            default: return "";
        }
    }

    QString LatchComponentEntry::enumToString()
    {
        const QString types[] = {"enable", "data_in_func", "async_set", "async_reset", "set_reset_behav"};
        return types[mSpecificType];
    }

    QString LatchComponentEntry::behaviorToString(std::pair<AsyncSetResetBehavior, AsyncSetResetBehavior> behav)
    {
        static QMap<hal::AsyncSetResetBehavior, QString> cPBehaviourToString {
            {hal::AsyncSetResetBehavior::L, "L"},
            {hal::AsyncSetResetBehavior::H, "H"},
            {hal::AsyncSetResetBehavior::N, "N"},
            {hal::AsyncSetResetBehavior::T, "T"},
            {hal::AsyncSetResetBehavior::X, "X"},
            {hal::AsyncSetResetBehavior::undef, "Undefined"},
        };
        return QString(cPBehaviourToString[behav.first] + ", " + cPBehaviourToString[behav.second]);
    }

}// namespace hal
