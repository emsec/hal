#include "gui/channel_manager/channel_model.h"

#include "hal_core/utilities/log.h"

#define ALL_CHANNEL "all"
namespace hal
{

    ChannelModel::ChannelModel(QObject* parent) : QAbstractTableModel(parent), m_temporary_items(30)
    {
        //m_channel_to_ignore = {"PythonContext", "python"};
        LogManager::get_instance().get_gui_callback().add_callback("gui",
                                                                   std::bind(&ChannelModel::handle_logmanager_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }

    ChannelModel::~ChannelModel()
    {
        LogManager::get_instance().get_gui_callback().remove_callback("gui");
    }

    ChannelModel* ChannelModel::get_instance()
    {
        static ChannelModel s_model;
        return &s_model;
    }

    QVariant ChannelModel::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        if (role != Qt::DisplayRole)
            return QVariant();

        ChannelItem* item = static_cast<ChannelItem*>(index.internalPointer());
        return item->data(index.column());
    }

    Qt::ItemFlags ChannelModel::flags(const QModelIndex& index) const
    {
        if (!index.isValid())
            return 0;

        return QAbstractItemModel::flags(index);
    }

    QVariant ChannelModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        {
            switch (section)
            {
                case static_cast<int>(ColumnNumber::NameColumn):
                    return "Channel";
            }
        }
        return QVariant();
    }

    QModelIndex ChannelModel::index(int row, int column, const QModelIndex& parent) const
    {
        if (!hasIndex(row, column, parent))
            return QModelIndex();

        if (parent.isValid())
            return QModelIndex();
        else
        {
            if (!m_permanent_items.empty())
            {
                if (row <= m_permanent_items.size() - 1)
                    return createIndex(row, column, m_permanent_items.at(row));
            }
            if (!m_temporary_items.empty())
            {
                if ((unsigned long)row <= m_permanent_items.size() + m_temporary_items.size() - 1)
                    return createIndex(row, column, m_temporary_items.at(row - m_permanent_items.size()));
            }
            return QModelIndex();
        }
    }

    int ChannelModel::rowCount(const QModelIndex& parent) const
    {
        if (parent.isValid())
            return 0;
        else
            return m_permanent_items.size() + m_temporary_items.size();
    }

    int ChannelModel::columnCount(const QModelIndex& parent) const
    {
        if (parent.isValid())
            return 0;
        else
            return 2;
    }

    ChannelItem* ChannelModel::add_channel(const QString name)
    {
        int offset = m_permanent_items.size() + m_temporary_items.size();

        if (m_temporary_items.full())
        {
            int index = offset + m_temporary_items.size();
            beginRemoveRows(QModelIndex(), index - 1, index - 1);
            delete m_temporary_items.back();
            endRemoveRows();
        }
        ChannelItem* item = new ChannelItem(name);

        beginInsertRows(QModelIndex(), offset, offset);
        m_temporary_items.push_back(item);
        endInsertRows();
        return item;
    }

    void ChannelModel::handle_logmanager_callback(const spdlog::level::level_enum& t, const std::string& channel_name, const std::string& msg_text)
    {
        if(m_channel_to_ignore.contains(QString::fromStdString(channel_name)))
            return;

        ChannelItem* all_channel = nullptr;
        ChannelItem* item        = nullptr;
        for (auto element : m_permanent_items)
        {
            if (element->name().toStdString() == ALL_CHANNEL)
            {
                all_channel = element;
            }
            if (element->name().toStdString() == channel_name)
            {
                item = element;
            }
        }
        if (item == nullptr || all_channel == nullptr)
        {
            for (auto element : m_temporary_items)
            {
                if (element->name().toStdString() == ALL_CHANNEL)
                {
                    all_channel = element;
                }
                if (element->name().toStdString() == channel_name)
                {
                    item = element;
                }
            }
        }
        if (all_channel == nullptr)
        {
            all_channel = add_channel(QString::fromStdString(ALL_CHANNEL));
        }
        if (item == nullptr)
        {
            item = add_channel(QString::fromStdString(channel_name));
        }

        all_channel->append_entry(new channel_entry(msg_text, t));
        item->append_entry(new channel_entry(msg_text, t));
        Q_EMIT updated(t, ALL_CHANNEL, msg_text);
        Q_EMIT updated(t, channel_name, msg_text);
    }
}    // namespace hal
