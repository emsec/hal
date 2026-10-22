#include "hal_core/utilities/log.h"

#include "hal_core/python_bindings/python_bindings.h"

#include <iostream>

namespace hal
{
    namespace
    {
        /**
         * Get the `"[file:line] "` prefix for the python frame that called into the bindings.
         *
         * The C++ `log_*` macros prefix every severity but `info` with `__FILE__` and `__LINE__`.
         * Expanding those here would yield the location inside these bindings, so the location of
         * the calling python code is used instead. Returns an empty string if there is no python
         * frame, e.g. when called from a C++ thread.
         */
        std::string python_caller_location()
        {
            try
            {
                py::object frame  = py::module_::import("sys").attr("_getframe")();
                const auto file   = py::cast<std::string>(frame.attr("f_code").attr("co_filename"));
                const auto line   = py::cast<int>(frame.attr("f_lineno"));
                return "[" + file + ":" + std::to_string(line) + "] ";
            }
            catch (const py::error_already_set&)
            {
                return std::string();
            }
        }

        void log_message(spdlog::level::level_enum level, const std::string& channel, const std::string& message, bool with_location)
        {
            const std::string text = with_location ? python_caller_location() + message : message;
            LogManager::get_instance()->get_channel(channel)->log(level, text);
        }
    }    // namespace

    void log_init(py::module& m)
    {
        // The python counterparts of the C++ log_* macros. They keep the C++ argument order
        // (channel first) and, like the macros, prefix every severity but 'info' with the source
        // location of the caller.
        m.def(
            "log_trace",
            [](const std::string& channel, const std::string& message) { log_message(spdlog::level::trace, channel, message, true); },
            py::arg("channel"),
            py::arg("message"),
            R"(
            Log a message into a specific channel with severity ``trace``.
            The message is prefixed with the file name and line number of the calling python code.

            :param str channel: The log channel's name.
            :param str message: The message to log.
        )");

        m.def(
            "log_debug",
            [](const std::string& channel, const std::string& message) { log_message(spdlog::level::debug, channel, message, true); },
            py::arg("channel"),
            py::arg("message"),
            R"(
            Log a message into a specific channel with severity ``debug``.
            The message is prefixed with the file name and line number of the calling python code.

            :param str channel: The log channel's name.
            :param str message: The message to log.
        )");

        m.def(
            "log_info",
            [](const std::string& channel, const std::string& message) { log_message(spdlog::level::info, channel, message, false); },
            py::arg("channel"),
            py::arg("message"),
            R"(
            Log a message into a specific channel with severity ``info``.

            :param str channel: The log channel's name.
            :param str message: The message to log.
        )");

        m.def(
            "log_warning",
            [](const std::string& channel, const std::string& message) { log_message(spdlog::level::warn, channel, message, true); },
            py::arg("channel"),
            py::arg("message"),
            R"(
            Log a message into a specific channel with severity ``warning``.
            The message is prefixed with the file name and line number of the calling python code.

            :param str channel: The log channel's name.
            :param str message: The message to log.
        )");

        m.def(
            "log_error",
            [](const std::string& channel, const std::string& message) { log_message(spdlog::level::err, channel, message, true); },
            py::arg("channel"),
            py::arg("message"),
            R"(
            Log a message into a specific channel with severity ``error``.
            The message is prefixed with the file name and line number of the calling python code.

            :param str channel: The log channel's name.
            :param str message: The message to log.
        )");

        m.def(
            "log_critical",
            [](const std::string& channel, const std::string& message) { log_message(spdlog::level::critical, channel, message, true); },
            py::arg("channel"),
            py::arg("message"),
            R"(
            Log a message into a specific channel with severity ``critical``.
            The message is prefixed with the file name and line number of the calling python code.

            :param str channel: The log channel's name.
            :param str message: The message to log.
        )");

        auto log_manager = py::class_<LogManager, RawPtrWrapper<LogManager>>(m, "LogManager", R"(
            The log manager takes care of the log channels of HAL and the sinks (e.g., stdout, log file, GUI) that they write to.
        )");

        log_manager.def(py::init([]() {
            LogManager* lm = LogManager::get_instance();
            return RawPtrWrapper<LogManager>(lm);
        }));

        log_manager.def("set_file_name", &LogManager::set_file_name, py::arg("file_name") = "", R"(
            Set the log file name.
            If file_name is empty, the default log file will be used.

            :param str file_name: The desired log file.
            )");

        log_manager.def("set_level_of_channel", &LogManager::set_level_of_channel, py::arg("channel_name"), py::arg("level"), R"(
            Set a channel's severity level.

            :param str channel_name: Name of the channel.
            :param str level: The severity level.
            )");

        log_manager.def_static(
            "get_channel", [](std::string channel_name) { LogManager::get_instance()->get_channel(channel_name); }, py::arg("channel_name"), R"(
            Ensure that a channel with the given name exists, creating it if it does not exist yet.
            Note that, unlike its C++ counterpart, this does not return the channel itself.

            :param str channel_name: Name of the channel.
            )");

        log_manager.def("get_channels", &LogManager::get_channels, R"(
            Returns all channels' names.

            :returns: A set of channel names.
            :rtype: set[str]
            )");

        log_manager.def("get_available_log_levels", &LogManager::get_available_log_levels, R"(
            Get all available severity levels.

            :returns: A set of severity levels.
            :rtype: set[str]
            )");

        log_manager.def("activate_channel", &LogManager::activate_channel, py::arg("channel_name"), R"(
            Activate a channel.
            By default all channels are active.

            :param str channel_name: The name of the channel.
            )");

        log_manager.def("activate_all_channels", &LogManager::activate_all_channels, R"(
            Activate all logging channels.
            )");

        log_manager.def("deactivate_channel", &LogManager::deactivate_channel, py::arg("channel_name"), R"(
            Deactivate a channel suppressing all output.

            :param str channel_name: The name of the channel.
            )");

        log_manager.def("deactivate_all_channels", &LogManager::deactivate_all_channels, R"(
            Deactivate all logging channels.
            )");

        log_manager.def("get_default_sinks", &LogManager::get_default_sinks, R"(
            Get the list of default sinks that are added to each newly created logger by default.

            :returns: The default sinks.
            :rtype: list[hal_py.log_sink]
            )");

        log_manager.def("remove_sink_from_default", &LogManager::remove_sink_from_default, py::arg("sink_type"), R"(
            Remove the specified sink type from the list of default sinks if present.

            :param str sink_type: The type of sink to remove from default sinks.
            )");

        auto log_sink = py::class_<LogManager::log_sink, RawPtrWrapper<LogManager::log_sink>>(log_manager, "log_sink");

        log_sink.def_readonly("sink_type", &LogManager::log_sink::sink_type, R"(
                The type of the sink.

                :type: str
            )");

        log_sink.def_readonly("is_file_sink", &LogManager::log_sink::is_file_sink, R"(
                Boolean indication whether sink is a file sink.

                :type: bool
            )");

        log_sink.def_readonly("truncate", &LogManager::log_sink::truncate, R"(
                Truncate option passed to sink.

                :type: bool
            )");

        log_sink.def_readonly("path", &LogManager::log_sink::path, R"(
                The file path incase the sink is a file sink.

                :type: pathlib.Path
            )");
    }
}    // namespace hal
