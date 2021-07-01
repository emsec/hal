
#include "gui/new_selection_details_widget/models/data_table_model.h"
#include <algorithm>


namespace hal
{
    DataTableModel::DataTableModel(QObject* parent) : QAbstractTableModel(parent) 
    {
        DataEntry emptyEntry;
    }

    int DataTableModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent)
        return 2;
    }

    int DataTableModel::rowCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent)
        return mDataEntries.size();
    }

    QVariant DataTableModel::data(const QModelIndex &index, int role) const
    {
        if (role == Qt::DisplayRole){
            if(index.column() == 0){
                return mDataEntries[index.row()].key;
            }
            else{
                return mDataEntries[index.row()].value;
            }
        }

        else if (role == Qt::TextAlignmentRole){
            return Qt::AlignLeft;
        }

        return QVariant();
    }

    QVariant DataTableModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        return QVariant();
    }

    bool DataTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
    {
        Q_UNUSED(index)
        Q_UNUSED(value)
        Q_UNUSED(role)
        return false;
    }

    void DataTableModel::updateData(const std::map<std::tuple<std::string, std::string>, std::tuple<std::string, std::string>>& dc)
    {
        //TODO

        Q_EMIT layoutAboutToBeChanged();
        // ...
        Q_EMIT layoutChanged();
    }

} // namespace hal


