//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include "core/callback_hook.h"
#include "core/program_arguments.h"
#include "core/program_options.h"
#include "core/utils.h"
#include "def.h"

#include <functional>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <tuple>
#include <memory>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#include <spdlog/sinks/base_sink.h>
#include <spdlog/spdlog.h>
#pragma GCC diagnostic pop


// macros to stringify to transform int to const char*
#define STRINGISTRINGIFY(x) #x
#define STRINGIFY(x) STRINGISTRINGIFY(x)

// macro to get the log channel
#define LOG_CHANNEL(channel) log_manager::get_instance().get_channel(channel)

/**
 * @ingroup core
 * @{
 */

/**
 * Logs a message into a specific channel.<br>
 * The function name indicates the severity level.
 *
 * @param[in] channel - The log channel's name.
 * @param[in] ... - The message in python format style.
 */
#define log_info(channel, ...) LOG_CHANNEL(channel)->info(__VA_ARGS__)

#define log_trace(channel, ...) LOG_CHANNEL(channel)->trace("[" __FILE__ ":" STRINGIFY(__LINE__) "] " __VA_ARGS__)

#define log_debug(channel, ...) LOG_CHANNEL(channel)->debug("[" __FILE__ ":" STRINGIFY(__LINE__) "] " __VA_ARGS__)

#define log_warning(channel, ...) LOG_CHANNEL(channel)->warn("[" __FILE__ ":" STRINGIFY(__LINE__) "] " __VA_ARGS__)

#define log_error(channel, ...) LOG_CHANNEL(channel)->error("[" __FILE__ ":" STRINGIFY(__LINE__) "] " __VA_ARGS__)

#define log_critical(channel, ...) LOG_CHANNEL(channel)->critical("[" __FILE__ ":" STRINGIFY(__LINE__) "] " __VA_ARGS__)
///@}

/**
 * @ingroup core
 */

/**
 * Logs a message into a specific channel and then exits with error code 1.<br>
 * Severity level is "critical".
 *
 * @param[in] channel - The log channel's name.
 * @param[in] ... - The message in python format style.
 */
#define die(channel, ...)                                                                      \
    do                                                                                         \
    {                                                                                          \
        LOG_CHANNEL(channel)->critical("[" __FILE__ ":" STRINGIFY(__LINE__) "] " __VA_ARGS__); \
        exit(1);                                                                               \
    } while (0);

class CORE_API log_manager
{
public:
    struct log_sink
    {
        std::shared_ptr<spdlog::sinks::sink> spdlog_sink;
        bool is_file_sink;
        bool truncate;
        hal::path path;
    };

    /**
     * Get the log manager instance.<br>
     * If the desired log file is left empty, the default log path is used.<br>
     * (Singleton pattern)
     *
     * @param[in] file_name - The file where the log is stored.
     * @returns The log manager.
     */
    static log_manager& get_instance(const hal::path& file_name = "");

    /**
     * Set the log file name.<br>
     * If \p file_name is empty, the default log file will be used.
     *
     * @param[in] file_name - The desired log file.
     */
    void set_file_name(const hal::path& file_name);

    /**
     * Set the logging format pattern.<br>
     * Look at the speedlog documentation to get details about the format string.
     *
     * @param[in] format - The format string.
     */
    void set_format_pattern(const std::string& format);

    /**
     * Get a channel specified by name.
     *
     * @param[in] channel_name - Name of the channel.
     * @returns The channel.
     */
    spdlog::logger* get_channel(const std::string& channel_name = "stdout") const;

    /**
     * Returns all channels' names.
     *
     * @returns A set of the names.
     */
    std::set<std::string> get_channels() const;

    /**
     * Add a channel to the log manager.
     *
     * @param[in] channel_name - Name of the channel.
     * @param[in] sinks - All sinks connected to this channel.
     * @param[in] level - The severity level of that channel.
     */
    void add_channel(const std::string& channel_name, std::vector<std::shared_ptr<log_sink>> sinks, const std::string& level = "info");

    /**
     * Remove a channel from the log manager.
     *
     * @param[in] channel_name - Name of the channel.
     */
    void remove_channel(const std::string& channel_name);

    /**
     * Get a channel's severity level.
     *
     * @param[in] channel_name - Name of the channel.
     * @returns The severity level of that channel.
     */
    std::string get_level_of_channel(const std::string& channel_name) const;

    /**
     * Set a channel's severity level.
     *
     * @param[in] channel_name - Name of the channel.
     * @param[in] level - The severity level.
     */
    void set_level_of_channel(const std::string& channel_name, const std::string& level);

    /**
     * Get all available severity levels.
     *
     * @returns A set of severity levels.
     */
    std::set<std::string> get_available_log_levels() const;

    /**
     * Activate a channel.<br>
     * By default all channels are active.
     *
     * @param[in] channel_name - The name of the channel.
     */
    void activate_channel(const std::string& channel_name);

    /**
     * Deactivate a channel suppressing all output.
     *
     * @param[in] channel_name - The name of the channel.
     */
    void deactivate_channel(const std::string& channel_name);

    /**
     * Get the GUI callback hook for displaying log messages inside the GUI.
     *
     * @returns The GUI callback hook for (level, channel, message).
     */
    callback_hook<void(const spdlog::level::level_enum&, const std::string&, const std::string&)>& get_gui_callback();

    /**
     * Get the program options for the logging system.
     *
     * @returns The program options.
     */
    program_options& get_option_descriptions();

    /**
     * Handle parsed program options for the logging system.
     *
     * @param[in] args - The parsed program options.
     */
    void handle_options(program_arguments& args);

    /**
     * Create a new logging sink which prints to stdout.
     *
     * @param[in] colored - If true, the output will be colored depending on the severity.
     * @returns The new sink.
     */
    static std::shared_ptr<log_sink> create_stdout_sink(const bool colored = true);

    /**
     * Create a new logging sink which prints to a file.<br>
     * If a sink for that file already exists, no new sink will be created.<br>
     * If \p file_name is empty, the default log file will be used.
     *
     * @param[in] file_name - The file name.
     * @param[in] truncate - Flag whether the file should be overwritten(true) or appended to(false).
     * @returns The new sink or the already existing sink.
     */
    static std::shared_ptr<log_sink> create_file_sink(const hal::path& file_name = "", const bool truncate = false);

    /**
     * Create a new logging sink which prints to the GUI.
     *
     * @returns The new sink.
     */
    static std::shared_ptr<log_sink> create_gui_sink();

private:
    static std::map<std::string, std::shared_ptr<log_sink>> m_file_sinks;

    // log_manager class constructor (private due to singleton)
    log_manager(const hal::path& file_name);

    // log_manager class destructor (private due to singleton)
    ~log_manager();

    // log_manager class object non-copyable
    log_manager(const log_manager&) = delete;

    // log_manager class object non-copyable
    log_manager& operator=(const log_manager&) = delete;

    hal::path m_file_path;

    bool m_initialized;

    std::map<std::string, spdlog::level::level_enum> m_level;

    std::map<std::string, std::shared_ptr<spdlog::logger>> m_logger;

    std::map<std::string, std::vector<std::shared_ptr<log_sink>>> m_logger_sinks;

    callback_hook<void(const spdlog::level::level_enum&, const std::string&, const std::string&)> m_gui_callback;

    program_options m_descriptions;
};

class log_gui_sink : public spdlog::sinks::base_sink<std::mutex>
{
public:
    /** constructor (= default) */
    log_gui_sink() = default;
    /** destructor (= default) */
    ~log_gui_sink() = default;

protected:
    /** interface implementation: spdlog::sinks::base_sink */
    void sink_it_(const spdlog::details::log_msg& msg) override;
    void flush_() override;
};
