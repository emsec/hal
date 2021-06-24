
#include <gui/include/gui/new_selection_details_widget/models/boolean_function_table_model.h>

#include "gui/new_selection_details_widget/models/lut_table_model.h"
#include "hal_core/utilities/log.h"

namespace hal {

    BooleanFunctionTableModel::BooleanFunctionTableModel(QObject *parent) : QAbstractTableModel(parent)
    {
        mEntries.clear();
    }

    int BooleanFunctionTableModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return 2;
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
                return (mEntries[row]->getEntryIdentifier() + " =");
            }
            else
            {
                return (mEntries[row]->getEntryValueString());
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

    QSharedPointer<BooleanFunctionTableEntry> BooleanFunctionTableModel::getEntryAtRow(int row) const
    {
        return mEntries[row];
    }

    void BooleanFunctionTableModel::setEntries(QList<QSharedPointer<BooleanFunctionTableEntry>> entries){
        Q_EMIT layoutAboutToBeChanged();
        mEntries = entries;
        Q_EMIT layoutChanged();
    }

}// namespace hal
