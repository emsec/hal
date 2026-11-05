#include "gui/selection_details_widget/gate_details_widget/parameter_table_model.h"

#include "gui/user_action/action_set_parameter_value.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/utilities/enums.h"

#include <algorithm>

namespace hal
{
    ParameterTableModel::ParameterTableModel(QObject* parent) : QAbstractTableModel(parent)
    {
    }

    int ParameterTableModel::columnCount(const QModelIndex& parent) const
    {
        return 5;
    }

    int ParameterTableModel::rowCount(const QModelIndex& parent) const
    {
        return mRows.size();
    }

    QVariant ParameterTableModel::data(const QModelIndex& index, int role) const
    {
        if (index.row() < 0 || index.row() >= rowCount())
        {
            return QVariant();
        }

        const ParameterRow& row = mRows[index.row()];

        if (role == Qt::DisplayRole && index.column() < columnCount())
        {
            switch (index.column())
            {
                case 0:
                    return row.name;
                case 1:
                    return row.type;
                case 2:
                    return row.size;
                case 3:
                    return row.defaultValue;
                case 4:
                    return row.value;
            }
        }

        if (role == Qt::EditRole && index.column() == 4)
        {
            return row.value;
        }

        if (role == EnumValuesRole && index.column() == 4)
        {
            return row.enumValues;
        }

        if (role == Qt::TextAlignmentRole)
        {
            return Qt::AlignLeft;
        }

        return QVariant();
    }

    QVariant ParameterTableModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        const char* horizontalHeader[] = {"Name", "Type", "Size", "Default", "Value"};
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section < columnCount())
        {
            return QString(horizontalHeader[section]);
        }

        return QVariant();
    }

    void ParameterTableModel::updateData(Gate* gate)
    {
        QVector<ParameterRow> rows;

        for (const auto& [name, declAndValue] : gate->get_parameters())
        {
            const Parameter& decl = declAndValue.first;

            ParameterRow row;
            row.name         = QString::fromStdString(name);
            row.type         = QString::fromStdString(enum_to_string<Parameter::Type>(decl.get_type()));
            row.size         = QString::number(decl.get_size());
            row.defaultValue = QString::fromStdString(decl.get_default_value());
            row.value        = QString::fromStdString(declAndValue.second);

            if (decl.get_type() == Parameter::Type::Enum)
            {
                for (const std::string& enumValue : decl.get_enum_values())
                {
                    row.enumValues.append(QString::fromStdString(enumValue));
                }
            }

            rows.append(row);
        }

        // get_parameters() returns an unordered_map, so sort by name for a stable display order.
        std::sort(rows.begin(), rows.end(), [](const ParameterRow& a, const ParameterRow& b) { return a.name < b.name; });

        // Editing a value triggers this update synchronously from within setData(). A model reset at that point
        // would discard the editor that is still committing, so values of the same gate are updated in place.
        bool sameRows = (gate == mGate && rows.size() == mRows.size());
        for (int i = 0; sameRows && i < rows.size(); ++i)
        {
            sameRows = (rows.at(i).name == mRows.at(i).name);
        }

        if (sameRows)
        {
            mRows = rows;
            if (!mRows.isEmpty())
            {
                Q_EMIT dataChanged(index(0, 0), index(mRows.size() - 1, columnCount() - 1));
            }
            return;
        }

        beginResetModel();
        mGate = gate;
        mRows = rows;
        endResetModel();
    }

    bool ParameterTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
    {
        if (role != Qt::EditRole || index.column() != 4)
        {
            return false;
        }

        auto act = new ActionSetParameterValue(mRows[index.row()].name, value.toString());
        act->setObject(UserActionObject(mGate->get_id(), UserActionObjectType::Gate));
        return act->exec();
    }

    Qt::ItemFlags ParameterTableModel::flags(const QModelIndex& index) const
    {
        if (index.column() == 4)
        {
            return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
        }
        return QAbstractTableModel::flags(index);
    }
}    // namespace hal
