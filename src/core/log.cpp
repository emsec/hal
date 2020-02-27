#include "core/log.h"
#include <iostream>
#include <spdlog/common.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/sinks/ansicolor_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/sinks/stdout_sinks.h>

std::map<std::string, std::shared_ptr<log_manager::log_sink>> log_manager::m_file_sinks;

log_manager::log_manager(const hal::path& file_name)
{
    m_file_path = (file_name.empty()) ? core_utils::get_default_log_directory() / "hal.log" : file_name;
    hal::fs::create_directories(m_file_path.parent_path());

    m_level = {
        {"trace", spdlog::level::level_enum::trace},
        {"debug", spdlog::level::level_enum::debug},
        {"info", spdlog::level::level_enum::info},
        {"warn", spdlog::level::level_enum::warn},
        {"err", spdlog::level::level_enum::err},
        {"critical", spdlog::level::level_enum::critical},
        {"off", spdlog::level::level_enum::off},
    };

    auto gui_sink = log_manager::create_gui_sink();

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
}

log_manager::~log_manager()
{
    for (const auto& it : m_logger)
        it.second->flush();
    spdlog::drop_all();
}

log_manager& log_manager::get_instance(const hal::path& file_name)
{
    static log_manager l(file_name);
    return l;
}

void log_manager::set_format_pattern(const std::string& format)
{
    spdlog::set_pattern(format);
}

std::shared_ptr<spdlog::logger> log_manager::get_channel(const std::string& channel)
{
    auto it = m_logger.find(channel);
    if (it == m_logger.end())
    {
        log_warning("stdout", "log channel '{}' was not registered so far, creating default channel.", channel);
        return add_channel(channel, {log_manager::create_stdout_sink(), log_manager::create_file_sink(), log_manager::create_gui_sink()}, "info");
    }

    return it->second;
}

std::set<std::string> log_manager::get_channels() const
{
    std::set<std::string> channels;
    for (const auto& it : m_logger)
        channels.insert(it.first);
    return channels;
}

std::shared_ptr<spdlog::logger> log_manager::add_channel(const std::string& channel_name, const std::vector<std::shared_ptr<log_sink>>& sinks, const std::string& level)
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
    this->set_level_of_channel(channel_name, level);

    return channel;
}

void log_manager::remove_channel(const std::string& channel_name)
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

std::string log_manager::get_level_of_channel(const std::string& channel_name) const
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

void log_manager::set_level_of_channel(const std::string& channel_name, const std::string& level)
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

void log_manager::activate_channel(const std::string& channel_name)
{
    this->set_level_of_channel(channel_name, "info");
}

void log_manager::deactivate_channel(const std::string& channel_name)
{
    this->set_level_of_channel(channel_name, "off");
}

std::shared_ptr<log_manager::log_sink> log_manager::create_stdout_sink(const bool colored)
{
    auto sink          = std::make_shared<log_sink>();
    sink->is_file_sink = false;

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

std::shared_ptr<log_manager::log_sink> log_manager::create_file_sink(const hal::path& file_name, const bool truncate)
{
    hal::path path = file_name;
    if (file_name.empty())
        path = get_instance().m_file_path;

    auto it = m_file_sinks.find(path.string());
    if (it != m_file_sinks.end())
        return it->second;

    auto sink          = std::make_shared<log_sink>();
    sink->is_file_sink = true;
    sink->truncate     = truncate;

    sink->path        = path;
    sink->spdlog_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path.string(), truncate);

    m_file_sinks[path.string()] = sink;

    return sink;
}

std::shared_ptr<log_manager::log_sink> log_manager::create_gui_sink()
{
    auto sink          = std::make_shared<log_sink>();
    sink->spdlog_sink  = std::make_shared<log_gui_sink>();
    sink->is_file_sink = false;
    return sink;
}

std::set<std::string> log_manager::get_available_log_levels() const
{
    std::set<std::string> levels;
    for (const auto& it : m_level)
        levels.insert(it.first);
    return levels;
}

void log_manager::set_file_name(const hal::path& file_path)
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

program_options& log_manager::get_option_descriptions()
{
    if (m_descriptions.get_options().empty())
    {
        m_descriptions.add("--log.level", "set default log level", {program_options::REQUIRED_PARAM});
        m_descriptions.add("--log.enabled", "default setting for enable logging", {program_options::REQUIRED_PARAM});

        for (const auto& channel : this->get_channels())
        {
            auto start          = "--log." + channel;
            auto level_string   = start + ".level";
            auto enabled_string = start + ".enabled";

            m_descriptions.add(level_string, "set log level for channel: " + channel, {program_options::REQUIRED_PARAM});
            m_descriptions.add(enabled_string, "enable logging level for channel: " + channel, {program_options::REQUIRED_PARAM});
        }
    }

    return m_descriptions;
}

void log_manager::handle_options(program_arguments& args)
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

callback_hook<void(const spdlog::level::level_enum&, const std::string&, const std::string&)>& log_manager::get_gui_callback()
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
    log_manager::get_instance().get_gui_callback()(msg.level, std::string(msg.logger_name.data()), std::string(formatted.data(), formatted.size()));
}

void log_gui_sink::flush_()
{
}
