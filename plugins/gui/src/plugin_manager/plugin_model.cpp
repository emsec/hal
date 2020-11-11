#include "gui/plugin_manager/plugin_model.h"

#include "hal_core/plugin_system/plugin_interface_cli.h"
#include "hal_core/utilities/log.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include <QDataStream>
#include <QIODevice>
#include <QMimeData>
#include <functional>

namespace hal
{
    PluginModel::PluginModel(QObject* parent) : QAbstractItemModel(parent), m_model_changed_callback_id(CALLBACK_HOOK_INVALID_IDX)
    {
        for (auto item : PluginItem::get_column_description())
        {
            m_columns.append(item.first);
        }
        m_model_changed_callback_id =
            plugin_manager::add_model_changed_callback(std::bind(&PluginModel::plugin_manager_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        connect(this, &PluginModel::load_plugin, this, &PluginModel::handle_load_plugin);
        connect(this, &PluginModel::unload_plugin, this, &PluginModel::handle_unload_plugin);
        // plugin_manager::load_all_plugins(); // already loaded
    }

    PluginModel::~PluginModel()
    {
        plugin_manager::remove_model_changed_callback(m_model_changed_callback_id);
    }

    bool PluginModel::is_valid_index(const QModelIndex& idx)
    {
        if (index(idx.row(), idx.column()).isValid())
        {
            if (!m_items.isEmpty())
                return true;
        }
        return false;
    }

    int PluginModel::rowCount(const QModelIndex& parent) const
    {
        if (parent.isValid())
            return 0;

        return m_items.count();
    }

    int PluginModel::columnCount(const QModelIndex& parent) const
    {
        if (parent.isValid())
            return 0;

        return m_columns.count();
    }

    QModelIndex PluginModel::index(int row, int column, const QModelIndex& parent) const
    {
        if (parent.isValid())
            return QModelIndex();

        if ((0 <= row) && (row < rowCount()) && (0 <= column) && (column < columnCount()))
            return createIndex(row, column);

        return QModelIndex();
    }

    QModelIndex PluginModel::parent(const QModelIndex& child) const
    {
        Q_UNUSED(child);

        return QModelIndex();
    }

    QVariant PluginModel::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        if ((Qt::DisplayRole == role) || (Qt::EditRole == role))
        {
            const PluginItem plugin = m_items.at(index.row());
            switch (index.column())
            {
                case static_cast<PluginItem::column_utype>(PluginItem::ColumnType::Name):
                    return plugin.mName;
                    break;
                case static_cast<PluginItem::column_utype>(PluginItem::ColumnType::Path):
                    return plugin.mPath;
                    break;
            }
        }
        return QVariant();
    }

    QVariant PluginModel::headerData(int section, Qt::Orientation orientation, int role) const
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

    Qt::ItemFlags PluginModel::flags(const QModelIndex& index) const
    {
        Qt::ItemFlags default_flags = QAbstractItemModel::flags(index);

        if (index.isValid())
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | default_flags;
        else
            return Qt::ItemIsDropEnabled | default_flags;
    }

    bool PluginModel::setData(const QModelIndex& index, const QVariant& value, int role)
    {
        if (!index.isValid() || (rowCount() < index.row()) || (columnCount() < index.column()))
            return false;

        if (Qt::EditRole == role)
        {
            PluginItem plugin = m_items.at(index.row());
            switch (index.column())
            {
                case static_cast<PluginItem::column_utype>(PluginItem::ColumnType::Name):
                    plugin.mName = value.toString();
                    break;
                case static_cast<PluginItem::column_utype>(PluginItem::ColumnType::Path):
                    plugin.mPath = value.toString();
                    break;
            }
            m_items[index.row()] = plugin;
            Q_EMIT dataChanged(index, index);
            return true;
        }
        return false;
    }

    void PluginModel::plugin_manager_callback(bool is_load, std::string const& plugin_name, std::string const& plugin_path)
    {
        QString name = QString::fromStdString(plugin_name);
        QString path = QString::fromStdString(plugin_path);
        if (is_load)
            Q_EMIT load_plugin(name, path);
        else
            Q_EMIT unload_plugin(name, path);
    }

    void PluginModel::handle_load_plugin(QString name, QString path)
    {
        auto plugin = plugin_manager::get_plugin_instance<CLIPluginInterface>(name.toStdString(), false);
        if (plugin == nullptr)
        {
            return;
        }
        beginInsertRows(QModelIndex(), this->rowCount(), this->rowCount());
        m_items.append(PluginItem(name, path));
        endInsertRows();
    }

    void PluginModel::handle_unload_plugin(QString name, QString path)
    {
        Q_UNUSED(path);

        int index = 0;
        for (int i = 0; i < m_items.length(); ++i)
        {
            if (m_items.at(i).mName == name)
            {
                index = i;
                break;
            }
        }
        beginRemoveRows(QModelIndex(), index, index);
        m_items.removeAt(index);
        endRemoveRows();
    }

    Qt::DropActions PluginModel::supportedDropActions() const
    {
        return Qt::MoveAction;
    }

    bool PluginModel::moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationChild)
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

    bool PluginModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
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

    void PluginModel::request_load_plugin(const QString& name, const QString& path)
    {
        std::string p_name = name.toLocal8Bit().constData();
        std::string p_path = path.toLocal8Bit().constData();
        plugin_manager::load(p_name, p_path);
    }

    void PluginModel::request_unload_plugin(QModelIndexList idx)
    {
        if (idx.size() != 1 && !(idx.at(0).isValid()))
            return;

        int row_idx = idx.at(0).row();
        plugin_manager::unload((std::string)m_items.at(row_idx).mName.toLocal8Bit().constData());
    }

    const QList<PluginItem> PluginModel::get_plugin_list()
    {
        return m_items;
    }

    void PluginModel::handle_run_plugin_triggered(const QModelIndex& index)
    {
        if (!index.isValid())
            return;
        int row_idx = index.row();
        Q_EMIT run_plugin(m_items.at(row_idx).mName);
    }
}
