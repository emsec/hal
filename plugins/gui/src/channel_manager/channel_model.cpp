#include "gui/channel_manager/channel_model.h"

#include "hal_core/utilities/log.h"

#define ALL_CHANNEL "all"
namespace hal
{


    ChannelModel::ChannelModel(QObject* parent) : QAbstractTableModel(parent), mTemporaryItems(30)
    {
        mChannelToIgnore = {"UserStudy"};
        LogManager::get_instance().get_gui_callback().add_callback("gui",
                                                                   std::bind(&ChannelModel::handleLogmanagerCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
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
            if (!mPermanentItems.empty())
            {
                if (row <= mPermanentItems.size() - 1)
                    return createIndex(row, column, mPermanentItems.at(row));
            }
            if (!mTemporaryItems.empty())
            {
                if ((unsigned long)row <= mPermanentItems.size() + mTemporaryItems.size() - 1)
                    return createIndex(row, column, mTemporaryItems.at(row - mPermanentItems.size()));
            }
            return QModelIndex();
        }
    }

    int ChannelModel::rowCount(const QModelIndex& parent) const
    {
        if (parent.isValid())
            return 0;
        else
            return mPermanentItems.size() + mTemporaryItems.size();
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
        int offset = mPermanentItems.size() + mTemporaryItems.size();

        if (mTemporaryItems.full())
        {
            int index = offset + mTemporaryItems.size();
            beginRemoveRows(QModelIndex(), index - 1, index - 1);
            delete mTemporaryItems.back();
            endRemoveRows();
        }
        ChannelItem* item = new ChannelItem(name);

        beginInsertRows(QModelIndex(), offset, offset);
        mTemporaryItems.push_back(item);
        endInsertRows();
        return item;
    }

    void ChannelModel::handleLogmanagerCallback(const spdlog::level::level_enum& t, const std::string& channel_name, const std::string& msg_text)
    {
        if(mChannelToIgnore.contains(QString::fromStdString(channel_name)))
            return;

        ChannelItem* all_channel = nullptr;
        ChannelItem* item        = nullptr;
        for (auto element : mPermanentItems)
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
            for (auto element : mTemporaryItems)
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

        all_channel->appendEntry(new ChannelEntry(msg_text, t));
        item->appendEntry(new ChannelEntry(msg_text, t));
        Q_EMIT updated(t, ALL_CHANNEL, msg_text);
        Q_EMIT updated(t, channel_name, msg_text);
    }
}    // namespace hal
