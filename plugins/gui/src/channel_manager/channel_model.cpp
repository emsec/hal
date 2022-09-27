#include "gui/channel_manager/channel_model.h"
#include <iostream>
#include "hal_core/utilities/log.h"

#define ALL_CHANNEL "all"
#define MAX_TEMP_CHANNELS 30

namespace hal
{


    ChannelModel::ChannelModel(QObject* parent) : QAbstractTableModel(parent)
    {
        mChannelToIgnore = {"UserStudy"};
        LogManager::get_instance().get_gui_callback().add_callback("gui",
            std::bind(&ChannelModel::handleLogmanagerCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }

    ChannelModel::~ChannelModel()
    {
        LogManager::get_instance().get_gui_callback().remove_callback("gui");
    }

    ChannelModel* ChannelModel::instance()
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
        return item->name();
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
                if ((unsigned long)row <= (unsigned long)(mPermanentItems.size() + mTemporaryItems.size() - 1))
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

    ChannelItem* ChannelModel::addChannel(const QString name)
    {
        int offset = mPermanentItems.size() + mTemporaryItems.size();

        if (mTemporaryItems.size() == MAX_TEMP_CHANNELS)
        {
            beginRemoveRows(QModelIndex(), offset - 1, offset - 1);
            delete mTemporaryItems.last();
            mTemporaryItems.removeLast();
            endRemoveRows();
        }
        ChannelItem* item = new ChannelItem(name);

        beginInsertRows(QModelIndex(), offset, offset);
        mTemporaryItems.prepend(item);
        endInsertRows();
        return item;
    }

    bool ChannelModel::channelExists(const QString& name) const
    {
        for (const ChannelItem* item : mPermanentItems)
            if (item->name() == name) return true;
        for (const ChannelItem* item : mTemporaryItems)
            if (item->name() == name) return true;
        return false;
    }


    void ChannelModel::handleLogmanagerCallback(const spdlog::level::level_enum& t, const std::string& channel_name, const std::string& msg_text)
    {
        if(mChannelToIgnore.contains(QString::fromStdString(channel_name)))
            return;
        if(msg_text == channel_name + " has manually been added to channellist")
        {
            if(channelExists(QString::fromStdString(channel_name)))
            {
                return;
            }
        }

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
            all_channel = addChannel(QString::fromStdString(ALL_CHANNEL));
        }
        if (item == nullptr)
        {
            item = addChannel(QString::fromStdString(channel_name));
        }

        std::string shown_text = msg_text;
        if (shown_text.length() > 255)
        {
            shown_text = shown_text.substr(0, 255) + "...";
        }
        all_channel->appendEntry(new ChannelEntry(shown_text, t));
        item->appendEntry(new ChannelEntry(shown_text, t));
        Q_EMIT updated(t, ALL_CHANNEL, shown_text);
        Q_EMIT updated(t, channel_name, shown_text);
    }
}    // namespace hal

