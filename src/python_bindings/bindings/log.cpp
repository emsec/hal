#include "hal_core/python_bindings/python_bindings.h"
#include "hal_core/utilities/log.h"
#include <iostream>

namespace hal
{
    void log_init(py::module& m)
    {
        py::class_<LogManager, RawPtrWrapper<LogManager>>(m, "LogManager")
                .def(py::init([](){
                    LogManager* lm = LogManager::get_instance();
                    return RawPtrWrapper<LogManager>(lm); }))

                .def("set_level_of_channel", &LogManager::set_level_of_channel, py::arg("channel_name"), py::arg("level"), R"(
                      Set a channel's severity level.

                      :param str channel_name: Name of the channel.
                      :param str level: The severity level, one out of {}.
                     )")

                .def_static("get_channel",  [](std::string channel_name) { LogManager::get_instance()->get_channel(channel_name); } , py::arg("channel_name"), R"(
                      Main purpose of get_channel() method in PYTHON is to create channel if not existing.

                      :param str channel_name: Name of the channel.
                     )")

                .def("get_channels", &LogManager::get_channels, R"(
                      Returns all channels' names.

                      :returns: Set of channel names.
                      :rtype: set[str]
                     )")

                .def("get_available_log_levels", &LogManager::get_available_log_levels, R"(
                      Get all available severity levels.

                      :returns: Set of severity levels.
                      :rtype: set[str]
                     )");
    }
}
