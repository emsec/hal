#include "hal_core/utilities/log.h"

#include "hal_core/python_bindings/python_bindings.h"

#include <iostream>

namespace hal
{
    void log_init(py::module& m)
    {
        auto log_manager = py::class_<LogManager, RawPtrWrapper<LogManager>>(m, "LogManager");

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
            :param str level: The severity level, one out of {}.
            )");

        log_manager.def_static(
            "get_channel", [](std::string channel_name) { LogManager::get_instance()->get_channel(channel_name); }, py::arg("channel_name"), R"(
            Main purpose of get_channel() method in PYTHON is to create channel if not existing.

            :param str channel_name: Name of the channel.
            )");

        log_manager.def("get_channels", &LogManager::get_channels, R"(
            Returns all channels' names.

            :returns: Set of channel names.
            :rtype: set[str]
            )");

        log_manager.def("get_available_log_levels", &LogManager::get_available_log_levels, R"(
            Get all available severity levels.

            :returns: Set of severity levels.
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

            :returns: list of default sinks.
            :rtype: list[hal_py.log_sink]
            )");

        log_manager.def("remove_sink_from_default", &LogManager::remove_sink_from_default, py::arg("sink_type"), R"(
            Removes all sinks of the specified type from the default sinks that get a added to each newly created channel by default.

            :param str sink_type: The type of sink to remove.
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
