
#include "gui/new_selection_details_widget/models/data_table_model.h"
#include <algorithm>
#include <QToolTip>


namespace hal
{
    DataTableModel::DataTableModel(QObject* parent) : QAbstractTableModel(parent) 
    {
        mKeyFont = QFont("Iosevka");
        mKeyFont.setBold(true);
        mKeyFont.setPixelSize(13);
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
                return (mDataEntries[index.row()].key + ":");
            }
            else{
                QString valueText = getValueTextByDataType(mDataEntries[index.row()].value, mDataEntries[index.row()].dataType);
                return valueText;
            }
        }

        else if (role == Qt::TextAlignmentRole){
            return Qt::AlignLeft;
        }

        else if (role == Qt::FontRole){
            if (index.column() == 0){
                return mKeyFont;
            }
        }

        else if (role == Qt::ToolTipRole){
            if (index.column() == 0){
                return mDataEntries[index.row()].category;
            }
            else {
                return mDataEntries[index.row()].dataType;
            }
        }

        return QVariant();
    }

    QVariant DataTableModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        Q_UNUSED(section)
        Q_UNUSED(orientation)
        Q_UNUSED(role)
        return QVariant();
    }

    bool DataTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
    {
        Q_UNUSED(index)
        Q_UNUSED(value)
        Q_UNUSED(role)
        return false;
    }

    DataTableModel::DataEntry DataTableModel::getEntryAtRow(int row) const
    {
        return mDataEntries.at(row);
    }

    void DataTableModel::updateData(const std::map<std::tuple<std::string, std::string>, std::tuple<std::string, std::string>>& dataMap)
    {
        Q_EMIT layoutAboutToBeChanged();
        mDataEntries.clear();
        for(const auto& [key, value] : dataMap)
        {
            DataEntry e;
            e.category  = QString::fromStdString(std::get<0>(key));
            e.key       = QString::fromStdString(std::get<1>(key));
            e.dataType  = QString::fromStdString(std::get<0>(value));
            e.value     = QString::fromStdString(std::get<1>(value));
            
            mDataEntries.append(e);
        }
        // The data is sorted by category first and then by key
        /*qSort(mDataEntries.begin(), mDataEntries.end(), 
            [](const DataEntry* a, const DataEntry* b) -> bool 
                { 
                    if(a->category == b->category)
                        return (a->key < b->key);
                    else
                        return (a->category < b->category); 
                });*/
        Q_EMIT layoutChanged();
    }

    QString DataTableModel::getValueTextByDataType(QString value, QString dataType) const
    {
        if(dataType == "string")
        {
            return QString("\"%1\"").arg(value);
        }
        else if(dataType == "bit_value" || dataType == "bit_vector")
        {
            return QString("0x%1").arg(value);
        }
        
        return value;
    }

} // namespace hal


