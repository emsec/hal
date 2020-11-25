#include "gui/channel_manager/channel_item.h"

#define MAX_CHANNEL_ENTIRES 1000

namespace hal
{
    ChannelItem::ChannelItem(QString name)
        : mName(name)
    {
    }

    QVariant ChannelItem::data(int column) const
    {
        switch (column)
        {
            case 0:
                return mName;
            default:
                return QVariant();
        }
    }

    const QString ChannelItem::name() const
    {
        return mName;
    }

    const QList<ChannelEntry*>* ChannelItem::getList() const
    {
        return &mLogEntries;
    }

    QReadWriteLock* ChannelItem::getLock()
    {
        return &mLock;
    }

    void ChannelItem::appendEntry(ChannelEntry* entry)
    {
        if (mLogEntries.size() == MAX_CHANNEL_ENTIRES)
            mLogEntries.removeFirst();

        mLogEntries.append(entry);
    }
}
