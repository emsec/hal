#include "gui/selection_details_widget/gate_details_widget/parameter_table_model.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/utilities/enums.h"

#include <algorithm>
#include <boost/container/container_fwd.hpp>

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
        beginResetModel();
        mGate = gate;
        mRows.clear();

        for (const auto& [name, declAndValue] : gate->get_parameters())
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

    bool ParameterTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
    {
        if (role != Qt::EditRole || index.column() != 4)
        {
            return false;
        }

        const std::string name = mRows[index.row()].name.toStdString();

        Result<Parameter> param = this->mGate->get_parameter_declaration(name);
        if (param.is_error())
        {
            return false;
        }

        Result<std::monostate> result = this->mGate->set_parameter(param.get(), value.toString().toStdString());
        if (result.is_error())
        {
            return false;
        }

        std::string norm_value = this->mGate->get_parameter_value(name).get();
        this->mRows[index.row()].value = QString::fromStdString(norm_value);

        Q_EMIT dataChanged(index, index);
        return true;
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
