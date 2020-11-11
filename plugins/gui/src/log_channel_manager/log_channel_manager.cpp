#include "gui/log_channel_manager/log_channel_manager.h"

namespace hal
{
    LogChannelManager::LogChannelManager()
    {
        //LogManager::get_instance().get_gui_callback().add_callback("gui", std::bind(&LogChannelManager::logmanagerCallback, this, std::placeholders::_1));
    }

    void LogChannelManager::logmanagerCallback(const spdlog::details::log_msg& msg)
    {
        Q_UNUSED(msg);
        //    spdlog::level::level_enum t = msg.level;
        //    std::string channel_name    = *msg.logger_name;
        //    std::string msg_text        = msg.formatted.str();

        //    ChannelItem* all_channel = nullptr;
        //    ChannelItem* item        = nullptr;
        //    for (auto element : mPermanentItems)
        //    {
        //        if (element->name().toStdString() == ALL_CHANNEL)
        //        {
        //            all_channel = element;
        //        }
        //        if (element->name().toStdString() == channel_name)
        //        {
        //            item = element;
        //        }
        //    }
        //    if (item == nullptr || all_channel == nullptr)
        //    {
        //        for (auto element : mTemporaryItems)
        //        {
        //            if (element->name().toStdString() == ALL_CHANNEL)
        //            {
        //                all_channel = element;
        //            }
        //            if (element->name().toStdString() == channel_name)
        //            {
        //                item = element;
        //            }
        //        }
        //    }
        //    if (all_channel == nullptr)
        //    {
        //        all_channel = add_channel(QString::fromStdString(ALL_CHANNEL));
        //    }
        //    if (item == nullptr)
        //    {
        //        item = add_channel(QString::fromStdString(channel_name));
        //    }
        //    all_channel->appendEntry(new ChannelEntry(msg_text, t));
        //    item->appendEntry(new ChannelEntry(msg_text, t));
        //    Q_EMIT updated(t, ALL_CHANNEL, msg_text);
        //    Q_EMIT updated(t, channel_name, msg_text);
    }
}
