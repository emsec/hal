#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void plugin_manager_init(py::module& m)
    {
        py::class_<FacExtensionInterface> py_fac_extension_interface(m, "FacExtensionInterface", R"(
            An extension interface that lets a plugin provide a parser or writer for a file type.
        )");

        py::enum_<FacExtensionInterface::Feature>(py_fac_extension_interface, "Feature", R"(
            The feature that is provided by the extension.
        )")
            .value("FacUnknown", FacExtensionInterface::Feature::FacUnknown, R"(No known feature.)")
            .value("FacNetlistParser", FacExtensionInterface::Feature::FacNetlistParser, R"(A netlist parser.)")
            .value("FacNetlistWriter", FacExtensionInterface::Feature::FacNetlistWriter, R"(A netlist writer.)")
            .value("FacGatelibParser", FacExtensionInterface::Feature::FacGatelibParser, R"(A gate library parser.)")
            .value("FacGatelibWriter", FacExtensionInterface::Feature::FacGatelibWriter, R"(A gate library writer.)")
            .export_values();

        py_fac_extension_interface.def("get_feature", &FacExtensionInterface::get_feature, R"(
            Get the feature provided by the extension.

            :returns: The feature.
            :rtype: hal_py.FacExtensionInterface.Feature
        )");

        py_fac_extension_interface.def("get_description", &FacExtensionInterface::get_description, R"(
            Get the description of the extension.

            :returns: The description.
            :rtype: str
        )");

        py_fac_extension_interface.def("get_supported_file_extensions", &FacExtensionInterface::get_supported_file_extensions, R"(
            Get the file extensions supported by the extension.

            :returns: A list of file extensions.
            :rtype: list[str]
        )");

        auto py_plugin_manager = m.def_submodule("plugin_manager");

        py::class_<plugin_manager::PluginFeature> py_plugin_feature(py_plugin_manager, "PluginFeature", R"(
            A feature provided by a plugin, such as a parser or a writer.
        )");

        py_plugin_feature.def_readwrite("feature", &plugin_manager::PluginFeature::feature, R"(
            The kind of feature that is provided.

            :type: hal_py.FacExtensionInterface.Feature
        )");

        py_plugin_feature.def_readwrite("args", &plugin_manager::PluginFeature::args, R"(
            Additional arguments of the feature, holding the supported file extensions if a parser or writer is registered.

            :type: list[str]
        )");

        py_plugin_feature.def_readwrite("description", &plugin_manager::PluginFeature::description, R"(
            The description of the feature.

            :type: str
        )");

        py_plugin_manager.def("get_plugin_names", &plugin_manager::get_plugin_names, R"(
            Get the names of all loaded plugins.

            :returns: The set of plugin names.
            :rtype: set(str)
        )");

        py_plugin_manager.def("load_all_plugins", &plugin_manager::load_all_plugins, py::arg("directory_names") = std::vector<std::filesystem::path>(), R"(
            Load all plugins in the specified directories.
            If ``directory_names`` is empty, the default directories will be searched.

            :param list[pathlib.Path] directory_names: A list of directory paths.
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        py_plugin_manager.def("load", &plugin_manager::load, py::arg("plugin_name"), py::arg("file_path"), R"(
            Load a single plugin by specifying its name and file path.

            :param str plugin_name: The desired name that is unique in the framework.
            :param pathlib.Path file_path: The path to the plugin file.
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        py_plugin_manager.def("unload_all_plugins", &plugin_manager::unload_all_plugins, R"(
            Releases all plugins and their associated resources.

            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        py_plugin_manager.def("unload", &plugin_manager::unload, py::arg("plugin_name"), R"(
            Releases a single plugin and its associated ressources.

            :param str plugin_name: The name of the plugin to unload.
            :returns: ``True`` on success, ``False`` otherwise.
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

        py_plugin_manager.def("add_existing_options_description", &plugin_manager::add_existing_options_description, py::arg("existing_options"), R"(
            Register existing program options to avoid reuse by plugins.

            :param hal_py.ProgramOptions existing_options: The program options.
        )");

        py_plugin_manager.def("get_cli_plugin_options", &plugin_manager::get_cli_plugin_options, R"(
            Get the command line interface options of all plugins.

            :returns: The program options.
            :rtype: hal_py.ProgramOptions
        )");

        py_plugin_manager.def("get_plugin_path", &plugin_manager::get_plugin_path, py::arg("plugin_name"), R"(
            Get the full path of a plugin.
            On macOS, several possible file extensions are probed.

            :param str plugin_name: The name of the plugin.
            :returns: The full path to the plugin within the HAL build directory.
            :rtype: pathlib.Path
        )");

        py_plugin_manager.def("has_valid_file_extension", &plugin_manager::has_valid_file_extension, py::arg("file_name"), R"(
            Check whether a file has an extension that is legal for a plugin on the current operating system.

            :param pathlib.Path file_name: The path to the file.
            :returns: ``False`` if the extension indicates that the file cannot be a plugin, ``True`` otherwise.
            :rtype: bool
        )");

        py_plugin_manager.def("get_plugin_features", &plugin_manager::get_plugin_features, py::arg("name"), R"(
            Get the features of a plugin identified by the stem of its file name.

            :param str name: The stem of the file name of the plugin.
            :returns: A list of features such as parsers or writers.
            :rtype: list[hal_py.plugin_manager.PluginFeature]
        )");
    }
}    // namespace hal
