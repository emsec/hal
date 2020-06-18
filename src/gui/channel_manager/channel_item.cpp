#include "channel_manager/channel_item.h"
namespace hal {
channel_item::channel_item(QString name)
    : m_name(name), m_log_entries(1000){Q_UNUSED(m_observer) Q_UNUSED(m_unread) Q_UNUSED(m_unread_warnings) Q_UNUSED(m_unread_errors) Q_UNUSED(m_unread_successes)}

      QVariant channel_item::data(int column) const
{
    switch (column)
    {
        case 0:
            return m_name;
        default:
            return QVariant();
    }
}

const QString channel_item::name() const
{
    return m_name;
}

const boost::circular_buffer<channel_entry *> *channel_item::get_buffer() const
{
    return &m_log_entries;
}

QReadWriteLock* channel_item::get_lock()
{
    return &m_lock;
}

void channel_item::append_entry(channel_entry* entry)
{
    m_log_entries.push_back(entry);
}
}
