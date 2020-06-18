#include "channel_manager/channel_model.h"
#include "core/log.h"

#define ALL_CHANNEL "all"
namespace hal {

channel_model channel_model::s_model;

channel_model::channel_model(QObject* parent) : QAbstractTableModel(parent), m_temporary_items(30)
{
    LogManager::get_instance().get_gui_callback().add_callback("gui", std::bind(&channel_model::handle_logmanager_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

channel_model* channel_model::get_instance()
{
    return &s_model;
}

QVariant channel_model::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    channel_item* item = static_cast<channel_item*>(index.internalPointer());
    return item->data(index.column());
}

Qt::ItemFlags channel_model::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index);
}

QVariant channel_model::headerData(int section, Qt::Orientation orientation, int role) const
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

QModelIndex channel_model::index(int row, int column, const QModelIndex& parent) const
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

int channel_model::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    else
        return m_permanent_items.size() + m_temporary_items.size();
}

int channel_model::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    else
        return 2;
}

channel_item* channel_model::add_channel(const QString name)
{
    int offset = m_permanent_items.size() + m_temporary_items.size();

    if (m_temporary_items.full())
    {
        int index = offset + m_temporary_items.size();
        beginRemoveRows(QModelIndex(), index - 1, index - 1);
        delete m_temporary_items.back();
        endRemoveRows();
    }
    channel_item* item = new channel_item(name);

    beginInsertRows(QModelIndex(), offset, offset);
    m_temporary_items.push_back(item);
    endInsertRows();
    return item;
}

void channel_model::handle_logmanager_callback(const spdlog::level::level_enum& t, const std::string& channel_name, const std::string& msg_text)
{
    channel_item* all_channel = nullptr;
    channel_item* item        = nullptr;
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
}
