
#include "gui/selection_details_widget/data_table_model.h"
#include <algorithm>
#include <QToolTip>
#include <QRandomGenerator>
#include "gui/code_editor/syntax_highlighter/python_qss_adapter.h"


namespace hal
{
    DataTableModel::DataTableModel(QObject* parent) : QAbstractTableModel(parent) 
    {
    }

    int DataTableModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent)
        //return 2;
        return 4;
    }

    int DataTableModel::rowCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent)
        return mDataEntries.size();
    }

    QVariant DataTableModel::data(const QModelIndex &index, int role) const
    {
        DataEntry entry = mDataEntries[index.row()];
        RowStyle style = mEntryToRowStyle[QPair(entry.category, entry.key)];

        if(role == Qt::DisplayRole && index.column() < columnCount())
        {
            switch (index.column())
            {
                case 0: return entry.category;
                case 1: return entry.key;
                case 2: return entry.dataType;
                case 3: return style.valueString;//e.g. 0x at the front if bitvector
            }
        }
        if(role == Qt::ForegroundRole && index.column() == 3 && style.valueColor.isValid())
            return style.valueColor;

        if(role == Qt::TextAlignmentRole)
            return Qt::AlignLeft;

//        if (role == Qt::DisplayRole){
//            if(index.column() == 0){
//                return (entry.key + ":");
//            }
//            else{
//                return style.valueString;
//            }
//        }

//        if (role == Qt::ForegroundRole){
//            if(index.column() == 1){
//                if(style.valueColor.isValid())
//                    return style.valueColor;
//            }
//        }

//        else if (role == Qt::TextAlignmentRole){
//            return Qt::AlignLeft;
//        }

//        else if (role == Qt::ToolTipRole){
//            if (index.column() == 0){
//                if(!style.keyToolTip.isEmpty())
//                    return style.keyToolTip;
//            }
//            else {
//                if(!style.valueToolTip.isEmpty())
//                    return style.valueToolTip;
//            }
//        }

        return QVariant();
    }

    QVariant DataTableModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
//        Q_UNUSED(section)
//        Q_UNUSED(orientation)
//        Q_UNUSED(role)
//        return QVariant();
        const char* horizontalHeader[] = { "Category", "Key", "Type", "Value"};
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section < columnCount())
            return QString(horizontalHeader[section]);

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
        mEntryToRowStyle.clear();
        for(const auto& [key, value] : dataMap)
        {
            DataEntry e;
            e.category  = QString::fromStdString(std::get<0>(key));
            e.key       = QString::fromStdString(std::get<1>(key));
            e.dataType  = QString::fromStdString(std::get<0>(value));
            e.value     = QString::fromStdString(std::get<1>(value));
            
            mDataEntries.append(e);
        }
        // The data is sorted by category first and then by key. However the category generic is always displayed first.
        qSort(mDataEntries.begin(), mDataEntries.end(), 
            [](const DataEntry a, const DataEntry b) -> bool 
                { 

                    if(a.category == b.category)
                    {
                        return (a.key < b.key);
                    }
                    else
                    {
                        if(a.category == "generic")
                            return true;
                        else if(b.category == "generic")
                            return false;
                        else
                            return (a.category < b.category);
                    }
                });

        // Compute the appearance
        int rowIdx = 0;
        for(const DataEntry& entry : mDataEntries)
        {
            mEntryToRowStyle[QPair(entry.category, entry.key)] = getRowStyleByEntry(entry, rowIdx);
            rowIdx++;
        }

        Q_EMIT layoutChanged();
    }

    DataTableModel::RowStyle DataTableModel::getRowStyleByEntry(const DataEntry& entry, int rowIdx) const
    {
        RowStyle style;
        style.keyFont = QFont();
        style.keyToolTip = QString();
        style.valueToolTip = QString();
        style.valueColor = QColor();

        // Sets the value string and color considering its data type
        if(entry.dataType == "string")
        {
            style.valueString = QString("\"%1\"").arg(entry.value);
            style.valueColor = PythonQssAdapter::instance()->doubleQuotedStringColor();
        }
        else if(entry.dataType == "bit_value" || entry.dataType == "bit_vector")
        {
            style.valueString = QString("0x%1").arg(entry.value);
            style.valueColor = PythonQssAdapter::instance()->numberColor();
        }
        else if(entry.dataType == "integer")
        {
            style.valueString = entry.value;
            style.valueColor = PythonQssAdapter::instance()->numberColor();
        }
        else
        {
            style.valueString = entry.value;
        }


        // Hacky solution to prevent that the tooltip in neighboring rows does not change position if the tooltip entry is the same.
        // Therefore, two different space types are used so that Qt thinks these are different entries.
        if(rowIdx%2 == 0)
        {
            style.keyToolTip = entry.category + " ";
            style.valueToolTip = entry.dataType + " ";
        }
        else
        {
            //U+00A0 is a no-break space and therefore another type of space
            style.keyToolTip = entry.category + QChar(0x00A0); 
            style.valueToolTip = entry.dataType + QChar(0x00A0);
        }
        
        return style;
    }

} // namespace hal


