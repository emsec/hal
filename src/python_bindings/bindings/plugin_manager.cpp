#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void plugin_manager_init(py::module& m)
    {
        auto py_plugin_manager = m.def_submodule("plugin_manager");

        py_plugin_manager.def("get_plugin_names", &plugin_manager::get_plugin_names, R"(
            Get the names of all loaded plugins.

            :returns: The set of plugin names.
            :rtype: set(str)
        )");

        py_plugin_manager.def("load_all_plugins", &plugin_manager::load_all_plugins, py::arg("directory_names") = std::vector<std::filesystem::path>(), R"(
            Load all plugins in the specified directories. 
            If directory is empty, the default directories will be searched.

            :param directory_names: A list of directory paths.
            :type directory_names: hal_py.hal_path
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_plugin_manager.def("load", &plugin_manager::load, py::arg("plugin_name"), py::arg("file_path"), R"(
            Load a single plugin by specifying its name and file path.

            :param str plugin_name: The desired name that is unique in the framework.
            :param file_path: The path to the plugin file.
            :type file_path: hal_py.hal_path
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_plugin_manager.def("unload_all_plugins", &plugin_manager::unload_all_plugins, R"(
            Releases all plugins and their associated resources.

            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_plugin_manager.def("unload", &plugin_manager::unload, py::arg("plugin_name"), R"(
            Releases a single plugin and its associated ressources.

            :param str plugin_name: The name of the plugin to unload.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_plugin_manager.def(
            "get_plugin_instance",
            [](const std::string& plugin_name) -> BasePluginInterface* { return plugin_manager::get_plugin_instance<BasePluginInterface>(plugin_name, true); },
            py::arg("plugin_name"),
            R"(
            Gets the basic interface for a plugin specified by name.
            By default calls the initialize() function of the plugin.

            :param str plugin_name: The name of the plugin.
            :returns: The basic plugin interface.
            :rtype: hal_py.BasePluginInterface
        )");
    }
}    // namespace hal
