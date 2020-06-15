#include "bindings.h"

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

        py_plugin_manager.def("load_all_plugins", &plugin_manager::load_all_plugins, py::arg("file_names") = std::vector<std::filesystem::path>(), R"(
        Load all plugins in the specified directories. If \p directory is empty, the default directories will be searched.

        :param file_names: A list of directory paths.
        :type file_names: hal_py.hal_path
        :returns: True on success.
        :rtype: bool
)");

        py_plugin_manager.def("load", &plugin_manager::load, py::arg("plugin_name"), py::arg("file_name"), R"(
        Load a single plugin by specifying a name and the file path.

        :param str plugin_name: The desired name, unique in the framework.
        :param file_name: The path to the plugin file.
        :type file_name: hal_py.hal_path
        :returns: True on success.
        :rtype: bool
)");

        py_plugin_manager.def("unload_all_plugins", &plugin_manager::unload_all_plugins, R"(
        Releases all plugins and associated resources.

        :returns: True on success.
        :rtype: bool
)");

        py_plugin_manager.def(
            "get_plugin_instance", [](const std::string& plugin_name) -> std::shared_ptr<i_base> { return plugin_manager::get_plugin_instance<i_base>(plugin_name, true); }, py::arg("plugin_name"), R"(
        Gets the basic interface for a plugin specified by name.

        :param str plugin_name: The plugin name.
        :returns: The interface base for the plugin.
        :rtype: hal_py.i_base
)");
    }
}    // namespace hal
