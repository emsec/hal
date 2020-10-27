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

#include "hal_core/defines.h"
#include "hal_core/utilities/callback_hook.h"
#include "hal_core/utilities/program_arguments.h"
#include "hal_core/utilities/program_options.h"
#include "hal_core/utilities/utils.h"

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <tuple>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#include <spdlog/sinks/base_sink.h>
#include <spdlog/spdlog.h>
#pragma GCC diagnostic pop

namespace hal
{
// macros to stringify to transform int to const char*
#define STRINGISTRINGIFY(x) #x
#define STRINGIFY(x) STRINGISTRINGIFY(x)

// macro to get the log channel
#define LOG_CHANNEL(channel) LogManager::get_instance().get_channel(channel)

/**
 * @ingroup utilities
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
 * @ingroup utilities
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

    /**
     * @ingroup utilities 
     */
    class CORE_API LogManager
    {
    public:
        struct log_sink
        {
            std::shared_ptr<spdlog::sinks::sink> spdlog_sink;
            bool is_file_sink;
            bool truncate;
            std::filesystem::path path;
        };

        /**
         * Get the log manager instance.<br>
         * If the desired log file is left empty, the default log path is used.<br>
         * (Singleton pattern)
         *
         * @param[in] file_name - The file where the log is stored.
         * @returns The log manager.
         */
        static LogManager& get_instance(const std::filesystem::path& file_name = "");

        /**
         * Set the log file name.<br>
         * If \p file_name is empty, the default log file will be used.
         *
         * @param[in] file_name - The desired log file.
         */
        void set_file_name(const std::filesystem::path& file_name);

        /**
         * Set the logging format pattern.<br>
         * Look at the speedlog documentation to get details about the format string.
         *
         * @param[in] format - The format string.
         */
        void set_format_pattern(const std::string& format);

        /**
         * Get a channel specified by name.
         * Creates the channel if it does not exist yet.
         *
         * @param[in] channel_name - Name of the channel.
         * @returns A pointer to the channel.
         */
        std::shared_ptr<spdlog::logger> get_channel(const std::string& channel_name = "stdout");

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
         * @returns A pointer to the channel.
         */
        std::shared_ptr<spdlog::logger> add_channel(const std::string& channel_name, const std::vector<std::shared_ptr<log_sink>>& sinks, const std::string& level = "info");

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
         * Activate all logging channels.
         */
        void activate_all_channels();

        /**
         * Deactivate a channel suppressing all output.
         *
         * @param[in] channel_name - The name of the channel.
         */
        void deactivate_channel(const std::string& channel_name);

        /**
         * Deactivate all logging channels.
         */
        void deactivate_all_channels();

        /**
         * Get the GUI callback hook for displaying log messages inside the GUI.
         *
         * @returns The GUI callback hook for (level, channel, message).
         */
        CallbackHook<void(const spdlog::level::level_enum&, const std::string&, const std::string&)>& get_gui_callback();

        /**
         * Get the program options for the logging system.
         *
         * @returns The program options.
         */
        ProgramOptions& get_option_descriptions();

        /**
         * Handle parsed program options for the logging system.
         *
         * @param[in] args - The parsed program options.
         */
        void handle_options(ProgramArguments& args);

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
        static std::shared_ptr<log_sink> create_file_sink(const std::filesystem::path& file_name = "", const bool truncate = false);

        /**
         * Create a new logging sink which prints to the GUI.
         *
         * @returns The new sink.
         */
        static std::shared_ptr<log_sink> create_gui_sink();

    private:
        static std::map<std::string, std::shared_ptr<log_sink>> m_file_sinks;

        // LogManager class constructor (private due to singleton)
        LogManager(const std::filesystem::path& file_name);

        // LogManager class destructor (private due to singleton)
        ~LogManager();

        // LogManager class object non-copyable
        LogManager(const LogManager&) = delete;

        // LogManager class object non-copyable
        LogManager& operator=(const LogManager&) = delete;

        std::filesystem::path m_file_path;

        std::map<std::string, spdlog::level::level_enum> m_level;

        std::map<std::string, std::shared_ptr<spdlog::logger>> m_logger;

        std::map<std::string, std::vector<std::shared_ptr<log_sink>>> m_logger_sinks;

        CallbackHook<void(const spdlog::level::level_enum&, const std::string&, const std::string&)> m_gui_callback;

        ProgramOptions m_descriptions;
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
}    // namespace hal
