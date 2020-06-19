#pragma once

#include "core/log.h"

#include "log_channel_manager/log_channel_manager_constants.h"

#include <QString>

#include <vector>
namespace hal{
struct log_entry
{
    QString m_message;
    spdlog::level::level_enum m_type;
};

struct log_channel
{
    QString m_name;

    log_entry m_entries[log_channel_manager_constants::max_entries];

    int m_entry_count;
    int m_first_entry;

    int unread_entries;
    int unread_warnings;
    int unread_errors;
    int unread_successes;
};

class log_channel_manager
{
public:
    log_channel_manager();

    void logmanager_callback(const spdlog::details::log_msg& msg);

private:
    std::vector<log_channel*> m_fixed_channels;
    log_channel* m_temporary_channels[log_channel_manager_constants::max_channels];
};
}
