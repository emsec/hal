#include "gui/channel_manager/channel_item.h"

namespace hal
{
    ChannelItem::ChannelItem(QString name)
        : mName(name), mLogEntries(1000){Q_UNUSED(mObserver) Q_UNUSED(mUnread) Q_UNUSED(mUnreadWarnings) Q_UNUSED(mUnreadErrors) Q_UNUSED(mUnreadSuccesses)}

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

    const boost::circular_buffer<ChannelEntry *> *ChannelItem::getBuffer() const
    {
        return &mLogEntries;
    }

    QReadWriteLock* ChannelItem::getLock()
    {
        return &mLock;
    }

    void ChannelItem::appendEntry(ChannelEntry* entry)
    {
        mLogEntries.push_back(entry);
    }
}
