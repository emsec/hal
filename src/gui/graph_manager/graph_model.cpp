#include "graph_manager/graph_model.h"
#include "netlist/gate.h"
#include <QFont>
#include <gui_globals.h>
#include <set>

graph_model::graph_model(QObject* parent) : QAbstractItemModel(parent)
{
    /*specifies the header-labes of the model*/
    m_columns << "name"
              << "id"
              << "tpye"
              << "location";
}

QVariant graph_model::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        graph_model_item* item = m_items.at(index.row());
        if (item->getItemType() == item_type::gate)
        {
            switch (index.column())
            {
                case 0:
                    return item->getName();
                    break;
                case 1:
                    return item->getID();
                    break;
                case 2:
                    return item->getType();
                    break;
                case 3:
                    return item->getLocation();
                    break;
            }
        }
        else
        {
            switch (index.column())
            {
                case 0:
                    return item->getName();
                    break;
                case 1:
                    return item->getID();
                    break;
                case 2:
                    return QString("Im a Net!");
                    break;    //the default value of the net-items for the type
                case 3:
                    return QString("Im a Net!");
                    break;    //the default value of the net-items for the type
            }
        }
    }
    if (role == Qt::FontRole)    //changes the font of the net-items for a better clarity
    {
        if (m_items.at(index.row())->getItemType() == item_type::net)
        {
            QFont netFont(g_settings.value("font/family").toString());
            netFont.setItalic(true);
            return netFont;
        }
    }
    return QVariant();
}

Qt::ItemFlags graph_model::flags(const QModelIndex& index) const
{
    Qt::ItemFlags default_flags = QAbstractItemModel::flags(index);

    if (index.isValid())
        return Qt::ItemIsSelectable | default_flags;
    else
        return default_flags;
}

QVariant graph_model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((role == Qt::DisplayRole) && (orientation == Qt::Horizontal) && (section >= 0) && (section < columnCount()))
        return m_columns.at(section);
    else if ((role == Qt::DisplayRole) && (orientation == Qt::Vertical) && (section >= 0) && (section < rowCount()))
        return section + 1;

    return QVariant();
}

QModelIndex graph_model::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid())
        return QModelIndex();
    if ((row >= 0) && (column >= 0))
        return createIndex(row, column);
    return QModelIndex();
}

QModelIndex graph_model::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int graph_model::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    else
        return m_items.count();
}

int graph_model::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    else
        return m_columns.count();
}

void graph_model::setupModelData(std::shared_ptr<netlist> g)
{
    auto gates = g->get_gates();
    auto nets  = g->get_nets();

    /*adds the gates to the model*/
    for (const auto& var : gates)
    {
        graph_model_item* m = new graph_model_item(var);
        beginInsertRows(QModelIndex(), this->rowCount(), this->rowCount());
        m_items.append(m);
        endInsertRows();
    }

    /*adds the nets to the model*/
    for (const auto& var : nets)
    {
        graph_model_item* m = new graph_model_item(var);
        beginInsertRows(QModelIndex(), this->rowCount(), this->rowCount());
        m_items.append(m);
        endInsertRows();
    }
}

void graph_model::clear()
{
    for (int i = m_items.count(); i > 0; i--)
    {
        beginRemoveRows(QModelIndex(), i - 1, i - 1);
        m_items.removeAt(i - 1);
        endRemoveRows();
    }
}
