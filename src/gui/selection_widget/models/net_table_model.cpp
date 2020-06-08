#include "gui/selection_widget/models/net_table_model.h"

#include "gui_globals.h"

net_table_model::net_table_model(QObject* parent) : QAbstractTableModel(parent){}

int net_table_model::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)

    return m_selected_net_ids.size();
}

int net_table_model::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)

    return 2;
}

QVariant net_table_model::data(const QModelIndex& index, int role) const
{
    if(!index.isValid())
        return QVariant();

    if(index.row() >= m_selected_net_ids.size() || index.row() < 0)
        return QVariant();

    if(role == Qt::DisplayRole)
    {
        const u32 net_id = m_selected_net_ids.at(index.row());
        const auto& net = g_netlist->get_net_by_id(net_id);

        if(!net)
            return QVariant();

        switch(index.column())
        {
            case 0: return QString::fromStdString(net->get_name()); break;
            case 1: return QString::number(net->get_id()); break;
            default: return QVariant();
        }
    }

    return QVariant();
}

QVariant net_table_model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role != Qt::DisplayRole)
        return QVariant();

    if(orientation == Qt::Horizontal)
    {
        switch(section)
        {
            case 0: return "Name"; break;
            case 1: return "Id"; break;
            default: QVariant();
        }
    }
    else if(orientation == Qt::Vertical)
    {
        return section + 1;
    }

    return QVariant();
}

void net_table_model::fetch_selection()
{
    const auto& net_selection = g_selection_relay.m_selected_nets;

    QVector<u32> net_ids;
    net_ids.reserve(net_selection.size());

    for(const auto id : net_selection)
        net_ids.append(id);

    m_selected_net_ids = net_ids;

    Q_EMIT layoutChanged();
}
