#include "plugin_manager/plugin_model.h"

#include "core/interface_cli.h"
#include "core/log.h"
#include "core/plugin_manager.h"
#include <QDataStream>
#include <QIODevice>
#include <QMimeData>
#include <functional>

plugin_model::plugin_model(QObject* parent) : QAbstractItemModel(parent), model_changed_callback_id(CALLBACK_HOOK_INVALID_IDX)
{
    for (auto item : plugin_item::get_column_description())
    {
        m_columns.append(item.first);
    }
    model_changed_callback_id =
        plugin_manager::add_model_changed_callback(std::bind(&plugin_model::plugin_manager_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    connect(this, &plugin_model::load_plugin, this, &plugin_model::handle_load_plugin);
    connect(this, &plugin_model::unload_plugin, this, &plugin_model::handle_unload_plugin);
    plugin_manager::load_all_plugins();
}

plugin_model::~plugin_model()
{
    plugin_manager::remove_model_changed_callback(model_changed_callback_id);
}

bool plugin_model::is_valid_index(const QModelIndex& idx)
{
    if (index(idx.row(), idx.column()).isValid())
    {
        if (!m_items.isEmpty())
            return true;
    }
    return false;
}

int plugin_model::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    return m_items.count();
}

int plugin_model::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    return m_columns.count();
}

QModelIndex plugin_model::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid())
        return QModelIndex();

    if ((0 <= row) && (row < rowCount()) && (0 <= column) && (column < columnCount()))
        return createIndex(row, column);

    return QModelIndex();
}

QModelIndex plugin_model::parent(const QModelIndex& child) const
{
    Q_UNUSED(child);

    return QModelIndex();
}

QVariant plugin_model::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if ((Qt::DisplayRole == role) || (Qt::EditRole == role))
    {
        const plugin_item plugin = m_items.at(index.row());
        switch (index.column())
        {
            case static_cast<plugin_item::column_utype>(plugin_item::column_t::name):
                return plugin.name;
                break;
            case static_cast<plugin_item::column_utype>(plugin_item::column_t::path):
                return plugin.path;
                break;
        }
    }
    return QVariant();
}

QVariant plugin_model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((role == Qt::DisplayRole) && (section >= 0))
    {
        if ((orientation == Qt::Horizontal) && (section < columnCount()))
            return m_columns.at(section);

        if ((role == Qt::DisplayRole) && (section < rowCount()))
            return section + 1;
    }
    return QVariant();
}

Qt::ItemFlags plugin_model::flags(const QModelIndex& index) const
{
    Qt::ItemFlags default_flags = QAbstractItemModel::flags(index);

    if (index.isValid())
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | default_flags;
    else
        return Qt::ItemIsDropEnabled | default_flags;
}

bool plugin_model::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || (rowCount() < index.row()) || (columnCount() < index.column()))
        return false;

    if (Qt::EditRole == role)
    {
        plugin_item plugin = m_items.at(index.row());
        switch (index.column())
        {
            case static_cast<plugin_item::column_utype>(plugin_item::column_t::name):
                plugin.name = value.toString();
                break;
            case static_cast<plugin_item::column_utype>(plugin_item::column_t::path):
                plugin.path = value.toString();
                break;
        }
        m_items[index.row()] = plugin;
        Q_EMIT dataChanged(index, index);
        return true;
    }
    return false;
}

void plugin_model::plugin_manager_callback(bool is_load, std::string const& plugin_name, std::string const& plugin_path)
{
    QString name = QString::fromStdString(plugin_name);
    QString path = QString::fromStdString(plugin_path);
    if (is_load)
        Q_EMIT load_plugin(name, path);
    else
        Q_EMIT unload_plugin(name, path);
}

void plugin_model::handle_load_plugin(QString name, QString path)
{
    auto plugin = plugin_manager::get_plugin_instance<i_cli>(name.toStdString(), false);
    if (plugin == nullptr)
    {
        return;
    }
    beginInsertRows(QModelIndex(), this->rowCount(), this->rowCount());
    m_items.append(plugin_item(name, path));
    endInsertRows();
}

void plugin_model::handle_unload_plugin(QString name, QString path)
{
    Q_UNUSED(path);

    int index = 0;
    for (int i = 0; i < m_items.length(); ++i)
    {
        if (m_items.at(i).name == name)
        {
            index = i;
            break;
        }
    }
    beginRemoveRows(QModelIndex(), index, index);
    m_items.removeAt(index);
    endRemoveRows();
}

Qt::DropActions plugin_model::supportedDropActions() const
{
    return Qt::MoveAction;
}

bool plugin_model::moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationChild)
{
    if (count != 1 || sourceParent != destinationParent)
        return false;
    if (destinationChild < 0)
        return false;
    int dst = (destinationChild >= m_items.size()) ? (m_items.size() - 1) : destinationChild;
    if (beginMoveRows(sourceParent, sourceRow, sourceRow, destinationParent, destinationChild))
    {
        m_items.move(sourceRow, dst);
        endMoveRows();
        return true;
    }
    else
    {
        return false;
    }
}

bool plugin_model::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    Q_UNUSED(column);

    if (action == Qt::MoveAction)
    {
        QByteArray encoded = data->data("application/x-qabstractitemmodeldatalist");
        QDataStream stream(&encoded, QIODevice::ReadOnly);
        QList<int> rows;

        while (!stream.atEnd())
        {
            int tmp_row, col;
            QMap<int, QVariant> roleDataMap;
            stream >> tmp_row >> col >> roleDataMap;
            if (!rows.contains(tmp_row))
                rows.append(tmp_row);
        }

        if (rows.length() != 1)
        {
            return false;
        }
        else
        {
            return this->moveRow(parent, rows.at(0), parent, row);
        }
    }
    return false;
}

void plugin_model::request_load_plugin(const QString& name, const QString& path)
{
    std::string p_name = name.toLocal8Bit().constData();
    std::string p_path = path.toLocal8Bit().constData();
    plugin_manager::load(p_name, p_path);
}

void plugin_model::request_unload_plugin(QModelIndexList idx)
{
    if (idx.size() != 1 && !(idx.at(0).isValid()))
        return;

    int row_idx = idx.at(0).row();
    plugin_manager::unload((std::string)m_items.at(row_idx).name.toLocal8Bit().constData());
}

const QList<plugin_item> plugin_model::get_plugin_list()
{
    return m_items;
}

void plugin_model::handle_run_plugin_triggered(const QModelIndex& index)
{
    if (!index.isValid())
        return;
    int row_idx = index.row();
    Q_EMIT run_plugin(m_items.at(row_idx).name);
}
