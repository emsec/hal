#include "channel_manager/channel_item.h"
namespace hal
{
    ChannelItem::ChannelItem(QString name)
        : m_name(name), m_log_entries(1000){Q_UNUSED(m_observer) Q_UNUSED(m_unread) Q_UNUSED(m_unread_warnings) Q_UNUSED(m_unread_errors) Q_UNUSED(m_unread_successes)}

          QVariant ChannelItem::data(int column) const
    {
        switch (column)
        {
            case 0:
                return m_name;
            default:
                return QVariant();
        }
    }

    const QString ChannelItem::name() const
    {
        return m_name;
    }

    const boost::circular_buffer<channel_entry *> *ChannelItem::get_buffer() const
    {
        return &m_log_entries;
    }

    QReadWriteLock* ChannelItem::get_lock()
    {
        return &m_lock;
    }

    void ChannelItem::append_entry(channel_entry* entry)
    {
        m_log_entries.push_back(entry);
    }
}
