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
            :type directory_names: pathlib.Path
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_plugin_manager.def("load", &plugin_manager::load, py::arg("plugin_name"), py::arg("file_path"), R"(
            Load a single plugin by specifying its name and file path.

            :param str plugin_name: The desired name that is unique in the framework.
            :param file_path: The path to the plugin file.
            :type file_path: pathlib.Path
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

        py_plugin_manager.def("get_cli_plugin_flags", &plugin_manager::get_cli_plugin_flags, R"(
            Get a mapping of flags pointing to their corresponding CLI plugin.

            :returns: A dict from flag to plugin name.
            :rtype: dict[str,str]
        )");

        py_plugin_manager.def("get_ui_plugin_flags", &plugin_manager::get_ui_plugin_flags, R"(
            Get a mapping of flags pointing to their corresponding UI plugin.

            :returns: A dict from flag to plugin name.
            :rtype: dict[str,str]
        )");

        py_plugin_manager.def(
            "get_plugin_instance",
            [](const std::string& plugin_name, bool initialize, bool silent) -> BasePluginInterface* { return plugin_manager::get_plugin_instance(plugin_name, initialize, silent); },
            py::arg("plugin_name"),
            py::arg("initialize") = true,
            py::arg("silent")     = false,
            R"(
            Gets the interface for a plugin specified by name.
            By default calls the initialize() function of the plugin.
            The returned object is of the plugin's own type as soon as the Python module of that plugin has been imported, and of type ``hal_py.BasePluginInterface`` otherwise.

            :param str plugin_name: The name of the plugin.
            :param bool initialize: Set to ``False`` to not call the initialize function of the plugin. Defaults to ``True``.
            :param bool silent: Set to ``True`` to omit the error message if the plugin is not found. Defaults to ``False``.
            :returns: The plugin interface on success, ``None`` otherwise.
            :rtype: hal_py.BasePluginInterface or None
        )");

        py_plugin_manager.def("add_model_changed_callback", &plugin_manager::add_model_changed_callback, py::arg("callback"), R"(
            Add a callback to notify the GUI about loaded or unloaded plugins.
            The callback takes a bool that is ``True`` on load and ``False`` on unload, the plugin name, and the plugin path.

            :param callback: The callback function.
            :type callback: lambda(bool, str, str) -> None
            :returns: The ID of the registered callback.
            :rtype: int
        )");

        py_plugin_manager.def("remove_model_changed_callback", &plugin_manager::remove_model_changed_callback, py::arg("id"), R"(
            Remove a registered callback.

            :param int id: The ID of the registered callback.
        )");
    }
}    // namespace hal
