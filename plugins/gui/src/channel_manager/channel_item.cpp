#include "gui/channel_manager/channel_item.h"

#define MAX_CHANNEL_ENTIRES 1000

namespace hal
{
    ChannelItem::ChannelItem(QString name)
        : mName(name)
    {
    }

    const QString ChannelItem::name() const
    {
        return mName;
    }

    const QList<ChannelEntry*>* ChannelItem::getEntries() const
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
