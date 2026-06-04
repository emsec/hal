#include "gui/selection_details_widget/gate_details_widget/parameter_table_model.h"

#include "hal_core/utilities/enums.h"

#include <algorithm>

namespace hal
{
    ParameterTableModel::ParameterTableModel(QObject* parent) : QAbstractTableModel(parent)
    {
    }

    int ParameterTableModel::columnCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent)
        return 5;
    }

    int ParameterTableModel::rowCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent)
        return mRows.size();
    }

    QVariant ParameterTableModel::data(const QModelIndex& index, int role) const
    {
        if (index.row() < 0 || index.row() >= mRows.size())
            return QVariant();

        const ParameterRow& row = mRows[index.row()];

        if (role == Qt::DisplayRole && index.column() < columnCount())
        {
            switch (index.column())
            {
                case 0: return row.name;
                case 1: return row.type;
                case 2: return row.size;
                case 3: return row.defaultValue;
                case 4: return row.value;
            }
        }

        if (role == Qt::TextAlignmentRole)
            return Qt::AlignLeft;

        return QVariant();
    }

    QVariant ParameterTableModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        const char* horizontalHeader[] = {"Name", "Type", "Size", "Default", "Value"};
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section < columnCount())
            return QString(horizontalHeader[section]);

        return QVariant();
    }

    void ParameterTableModel::updateData(const std::unordered_map<std::string, std::pair<Parameter, std::string>>& parameters)
    {
        beginResetModel();
        mRows.clear();

        for (const auto& [name, declAndValue] : parameters)
        {
            const Parameter& decl = declAndValue.first;

            ParameterRow row;
            row.name         = QString::fromStdString(name);
            row.type         = QString::fromStdString(enum_to_string<Parameter::Type>(decl.get_type()));
            row.size         = QString::number(decl.get_size());
            row.defaultValue = QString::fromStdString(decl.get_default_value());
            row.value        = QString::fromStdString(declAndValue.second);
            mRows.append(row);
        }

        // get_parameters() returns an unordered_map, so sort by name for a stable display order.
        std::sort(mRows.begin(), mRows.end(), [](const ParameterRow& a, const ParameterRow& b) { return a.name < b.name; });

        endResetModel();
    }
}
