#include "hal_core/utilities/log.h"

#include <iostream>
#include <spdlog/common.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/sinks/ansicolor_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/sinks/stdout_sinks.h>

namespace hal
{
    std::map<std::string, std::shared_ptr<LogManager::log_sink>> LogManager::m_file_sinks;

    LogManager* LogManager::m_instance = nullptr;

    LogManager::LogManager(const std::filesystem::path& file_name)
    {
        m_file_path = (file_name.empty()) ? utils::get_default_log_directory() / "hal.log" : file_name;
        std::filesystem::create_directories(m_file_path.parent_path());

        m_level = {
            {"trace", spdlog::level::level_enum::trace},
            {"debug", spdlog::level::level_enum::debug},
            {"info", spdlog::level::level_enum::info},
            {"warn", spdlog::level::level_enum::warn},
            {"err", spdlog::level::level_enum::err},
            {"critical", spdlog::level::level_enum::critical},
            {"off", spdlog::level::level_enum::off},
        };

        auto gui_sink = LogManager::create_gui_sink();

        spdlog::sinks_init_list stdout_init_list = {std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>(), gui_sink->spdlog_sink};

        m_logger = {
            // initialize null channel
            {"null", spdlog::create<spdlog::sinks::null_sink_mt>("null")},
            // initialize multi-threaded, colored stdout channel logger
            {"stdout", std::make_shared<spdlog::logger>("stdout", stdout_init_list)},
        };

        spdlog::details::registry::instance().initialize_logger(m_logger.at("stdout"));

        spdlog::set_error_handler([](const std::string& msg) { throw std::invalid_argument("[!] internal log error: " + msg); });

        //set_format_pattern("[%c %z] [%n] [%l] %v");
        set_format_pattern("[%n] [%l] %v");

        m_default_sinks = {gui_sink, LogManager::create_stdout_sink(), LogManager::create_file_sink(m_file_path)};
    }

    LogManager::~LogManager()
    {
        std::cerr << "~LogManager" << std::endl;
        for (const auto& it : m_logger)
            it.second->flush();
        spdlog::drop_all();
    }

    LogManager* LogManager::get_instance(const std::filesystem::path& file_name)
    {
        if (!m_instance)
        {
            m_instance = new LogManager(file_name);
        }
        return m_instance;
    }

    void LogManager::set_format_pattern(const std::string& format)
    {
        spdlog::set_pattern(format);
    }

    std::shared_ptr<spdlog::logger> LogManager::get_channel(const std::string& channel)
    {
        auto it = m_logger.find(channel);
        if (it == m_logger.end())
        {
            if (channel != "stdout")    // avoid infinite recursion
            {
                log_warning("stdout", "log channel '{}' was not registered so far, creating default channel.", channel);
            }
            return add_channel(channel, m_default_sinks, "info");
        }

        return it->second;
    }

    std::set<std::string> LogManager::get_channels() const
    {
        std::set<std::string> channels;
        for (const auto& it : m_logger)
        {
            channels.insert(it.first);
        }
        return channels;
    }

    std::shared_ptr<spdlog::logger> LogManager::add_channel(const std::string& channel_name, const std::vector<std::shared_ptr<log_sink>>& sinks, const std::string& level)
    {
        if (auto it = m_logger.find(channel_name); it != m_logger.end())
        {
            return it->second;
        }

        std::vector<std::shared_ptr<spdlog::sinks::sink>> vec;
        for (const auto& sink : sinks)
        {
            vec.push_back(sink->spdlog_sink);
        }

        auto channel           = std::make_shared<spdlog::logger>(channel_name, vec.begin(), vec.end());
        m_logger[channel_name] = channel;
        m_logger[channel_name]->flush_on(spdlog::level::info);

        spdlog::details::registry::instance().initialize_logger(m_logger.at(channel_name));

        m_logger_sinks[channel_name] = sinks;

        if (m_enforce_level.empty())
        {
            this->set_level_of_channel(channel_name, level);
        }
        else
        {
            this->set_level_of_channel(channel_name, m_enforce_level);
        }

        return channel;
    }

    void LogManager::remove_channel(const std::string& channel_name)
    {
        if (m_logger.find(channel_name) == m_logger.end())
        {
            return;
        }
        m_logger[channel_name]->flush();
        spdlog::drop(channel_name);
        m_logger.erase(channel_name);
        m_logger_sinks.erase(channel_name);
    }

    std::string LogManager::get_level_of_channel(const std::string& channel_name) const
    {
        auto it_channel = m_logger.find(channel_name);
        if (it_channel == m_logger.end())
        {
            return std::string("");
        }

        for (const auto& it_level : m_level)
        {
            if (it_level.second == it_channel->second->level())
            {
                return it_level.first;
            }
        }

        return std::string("");
    }

    void LogManager::set_level_of_channel(const std::string& channel_name, const std::string& level)
    {
        auto it_channel = m_logger.find(channel_name);
        auto it_level   = m_level.find(level);
        if (it_channel == m_logger.end() || it_level == m_level.end())
        {
            return;
        }

        auto& channel = it_channel->second;
        channel->set_level(it_level->second);
        m_logger[channel_name] = channel;
    }

    void LogManager::activate_channel(const std::string& channel_name)
    {
        this->set_level_of_channel(channel_name, "info");
    }

    void LogManager::activate_all_channels()
    {
        for (const auto& channel : m_logger)
        {
            set_level_of_channel(channel.first, "info");
        }
    }

    void LogManager::deactivate_channel(const std::string& channel_name)
    {
        this->set_level_of_channel(channel_name, "off");
    }

    void LogManager::deactivate_all_channels()
    {
        for (const auto& channel : m_logger)
        {
            set_level_of_channel(channel.first, "off");
        }
    }

    std::vector<std::shared_ptr<hal::LogManager::log_sink>> LogManager::get_default_sinks()
    {
        return m_default_sinks;
    }

    void LogManager::remove_sink_from_default(const std::string& sink_type)
    {
        m_default_sinks.erase(
            std::remove_if(m_default_sinks.begin(), m_default_sinks.end(), [sink_type](const std::shared_ptr<hal::LogManager::log_sink> sink) { return sink->sink_type == sink_type; }),
            m_default_sinks.end());
    }

    std::shared_ptr<LogManager::log_sink> LogManager::create_stdout_sink(const bool colored)
    {
        auto sink          = std::make_shared<log_sink>();
        sink->is_file_sink = false;
        sink->sink_type    = "stdout";

        if (!colored)
        {
            sink->spdlog_sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
        }
        else
        {
#ifdef _WIN32
            // todo: to be implemented for windows
            sink->spdlog_sink = nullptr;
            log_error("stdout", "create_stdout_sink() has to be implemented for Windows.");
#else
            auto stdout_sink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
            stdout_sink->set_color(spdlog::level::trace, stdout_sink->green);
            stdout_sink->set_color(spdlog::level::debug, stdout_sink->blue);
            stdout_sink->set_color(spdlog::level::info, stdout_sink->reset);
            stdout_sink->set_color(spdlog::level::warn, stdout_sink->yellow);
            stdout_sink->set_color(spdlog::level::err, stdout_sink->red);
            stdout_sink->set_color(spdlog::level::critical, stdout_sink->red_bold);
            sink->spdlog_sink = stdout_sink;
#endif
        }

        return sink;
    }

    std::shared_ptr<LogManager::log_sink> LogManager::create_file_sink(const std::filesystem::path& file_name, const bool truncate)
    {
        std::filesystem::path path = file_name;
        if (file_name.empty())
        {
            path = get_instance()->m_file_path;
        }

        auto it = m_file_sinks.find(path.string());
        if (it != m_file_sinks.end())
        {
            return it->second;
        }

        auto sink          = std::make_shared<log_sink>();
        sink->is_file_sink = true;
        sink->sink_type    = "file";
        sink->truncate     = truncate;

        sink->path        = path;
        sink->spdlog_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path.string(), truncate);

        m_file_sinks[path.string()] = sink;

        return sink;
    }

    std::shared_ptr<LogManager::log_sink> LogManager::create_gui_sink()
    {
        auto sink          = std::make_shared<log_sink>();
        sink->spdlog_sink  = std::make_shared<log_gui_sink>();
        sink->is_file_sink = false;
        sink->sink_type    = "gui";
        return sink;
    }

    std::set<std::string> LogManager::get_available_log_levels() const
    {
        std::set<std::string> levels;
        for (const auto& it : m_level)
            levels.insert(it.first);
        return levels;
    }

    void LogManager::set_file_name(const std::filesystem::path& file_path)
    {
        log_info("core", "setting log file to '{}'.", file_path.string());

        // search through all existing channels
        // mark every channel for recreation which contains a file sink that pointed to the old path
        std::vector<std::string> channels_to_recreate;
        for (const auto& it : m_logger_sinks)
        {
            auto name  = it.first;
            auto sinks = it.second;

            for (const auto& sink : sinks)
            {
                if (sink->is_file_sink && (sink->path == m_file_path))
                {
                    channels_to_recreate.push_back(name);
                    break;
                }
            }
        }

        // recreate all marked channels
        // keep all sinks and rebuild file sinks with the new path
        for (const auto& name : channels_to_recreate)
        {
            auto level = get_level_of_channel(name);

            std::vector<std::shared_ptr<log_sink>> new_sinks;
            for (const auto& sink : m_logger_sinks[name])
            {
                if (sink->is_file_sink && (sink->path == m_file_path))
                    new_sinks.push_back(create_file_sink(file_path, sink->truncate));
                else
                    new_sinks.push_back(sink);
            }
            remove_channel(name);
            add_channel(name, new_sinks, level);
        }

        // update default sinks
        for (u32 sink_idx = 0; sink_idx < m_default_sinks.size(); sink_idx++)
        {
            if (m_default_sinks.at(sink_idx)->is_file_sink && m_default_sinks.at(sink_idx)->path == m_file_path)
            {
                m_default_sinks.at(sink_idx) = create_file_sink(file_path, m_default_sinks.at(sink_idx)->truncate);
            }
        }

        // close all unused file sinks
        for (auto it = m_file_sinks.cbegin(); it != m_file_sinks.cend(); /* no increment */)
        {
            if (it->second.unique())
                m_file_sinks.erase(it++);
            else
                ++it;
        }

        // adjust path
        m_file_path = file_path;
    }

    ProgramOptions& LogManager::get_option_descriptions()
    {
        if (m_descriptions.get_options().empty())
        {
            m_descriptions.add("--log.level", "set default log level", {ProgramOptions::A_REQUIRED_PARAMETER});
            m_descriptions.add("--log.enabled", "default setting for enable logging", {ProgramOptions::A_REQUIRED_PARAMETER});

            for (const auto& channel : this->get_channels())
            {
                auto start          = "--log." + channel;
                auto level_string   = start + ".level";
                auto enabled_string = start + ".enabled";

                m_descriptions.add(level_string, "set log level for channel: " + channel, {ProgramOptions::A_REQUIRED_PARAMETER});
                m_descriptions.add(enabled_string, "enable logging level for channel: " + channel, {ProgramOptions::A_REQUIRED_PARAMETER});
            }
        }

        return m_descriptions;
    }

    void LogManager::handle_options(ProgramArguments& args)
    {
        bool default_enabled = true;
        if (args.is_option_set("--log.enabled"))
        {
            auto arg        = args.get_parameter("--log.enabled");
            default_enabled = (arg == "true" || arg == "1");
            for (const auto& channel : this->get_channels())
            {
                if (default_enabled)
                    this->activate_channel(channel);
                else
                    this->deactivate_channel(channel);
            }
        }
        if (args.is_option_set("--log.level"))
        {
            auto level = args.get_parameter("--log.level");
            if (m_level.find(level) != m_level.end())
            {
                m_enforce_level = level;
                for (const auto& channel : this->get_channels())
                {
                    if (default_enabled)
                        this->set_level_of_channel(channel, level);
                }
            }
            else
                log_warning("core", "default log level {} provided is not valid.", level);
        }
        for (const auto& channel : this->get_channels())
        {
            bool enabled = true;
            if (args.is_option_set("--log." + channel + ".enabled"))
            {
                auto arg = args.get_parameter("--log." + channel + ".enabled");
                enabled  = (arg == "true" || arg == "1");
                if (enabled)
                    this->activate_channel(channel);
                else
                    this->deactivate_channel(channel);
            }

            if (args.is_option_set("--log." + channel + ".level"))
            {
                auto level = args.get_parameter("--log." + channel + ".level");
                if (m_level.find(level) != m_level.end())
                {
                    if (enabled)
                        this->set_level_of_channel(channel, level);
                }
                else
                    log_warning("core", "log level {} provided for {} is not valid.", level, channel);
            }
        }
    }

    CallbackHook<void(const spdlog::level::level_enum&, const std::string&, const std::string&)>& LogManager::get_gui_callback()
    {
        return m_gui_callback;
    }

    /*
     * log gui sink implementation
     */
    void log_gui_sink::sink_it_(const spdlog::details::log_msg& msg)
    {
        spdlog::memory_buf_t formatted;
        formatter_->format(msg, formatted);
        LogManager::get_instance()->get_gui_callback()(msg.level, std::string(msg.logger_name.data()), std::string(formatted.data(), formatted.size()));
    }

    void log_gui_sink::flush_()
    {
    }
}    // namespace hal
