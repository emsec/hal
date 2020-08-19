#include "gui/selection_widget/models/gate_table_model.h"

#include "gui_globals.h"

namespace hal
{
    gate_table_model::gate_table_model(QObject* parent) : QAbstractTableModel(parent){}

    int gate_table_model::rowCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent)

        return m_selected_gate_ids.size();
    }

    int gate_table_model::columnCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent)

        return 3;
    }

    QVariant gate_table_model::data(const QModelIndex& index, int role) const
    {
        if(!index.isValid())
            return QVariant();

        if(index.row() >= m_selected_gate_ids.size() || index.row() < 0)
            return QVariant();

        if(role == Qt::DisplayRole)
        {
            const u32 gate_id = m_selected_gate_ids.at(index.row());
            auto gate = g_netlist->get_gate_by_id(gate_id);

            if(!gate)
                return QVariant();

            switch(index.column())
            {
            case 0: return QString::fromStdString(gate->get_name()); break;
            case 1: return QString::number(gate->get_id()); break;
            case 2: return QString::fromStdString(gate->get_type()->get_name()); break;
            default: return QVariant();
            }
        }

        return QVariant();
    }

    QVariant gate_table_model::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if(role != Qt::DisplayRole)
            return QVariant();

        if(orientation == Qt::Horizontal)
        {
            switch(section)
            {
            case 0: return "Name"; break;
            case 1: return "Id"; break;
            case 2: return "Type"; break;
            default: QVariant();
            }
        }
        else if(orientation == Qt::Vertical)
        {
            return section + 1;
        }

        return QVariant();
    }

    void gate_table_model::fetch_selection()
    {
        auto gate_selection = g_selection_relay.m_selected_gates;

        Q_EMIT layoutAboutToBeChanged();
        QVector<u32> gate_ids;
        gate_ids.reserve(gate_selection.size());

        for(const auto id : gate_selection)
            gate_ids.append(id);

        m_selected_gate_ids = gate_ids;

        Q_EMIT layoutChanged();
    }
}
