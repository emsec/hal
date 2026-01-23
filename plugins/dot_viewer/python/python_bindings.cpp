#include "hal_core/python_bindings/python_bindings.h"

#include "dot_viewer/dot_viewer.h"
#include "dot_viewer/plugin_dot_viewer.h"
#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

namespace py = pybind11;

namespace hal
{

    // the name in PYBIND11_MODULE/PYBIND11_PLUGIN *MUST* match the filename of the output library (without extension),
    // otherwise you will get "ImportError: dynamic module does not define module export function" when importing the module

#ifdef PYBIND11_MODULE
    PYBIND11_MODULE(dot_viewer, m)
    {
        m.doc() = "Plugin to visualize .dot graphs within the HAL GUI.";
#else
    PYBIND11_PLUGIN(dot_viewer)
    {
        py::module m("dot_viewer", "Plugin to visualize .dot graphs within the HAL GUI.");
#endif    // ifdef PYBIND11_MODULE

        py::class_<DotViewerPlugin, RawPtrWrapper<DotViewerPlugin>, BasePluginInterface> py_dotviewer_plugin(
            m, "DotViewerPlugin", R"(This class provides an interface to integrate a .dot viewer as a plugin within the HAL framework.)");

        py_dotviewer_plugin.def_property_readonly("name", &DotViewerPlugin::get_name, R"(
            The name of the plugin.

            :type: str
        )");

        py_dotviewer_plugin.def("get_name", &DotViewerPlugin::get_name, R"(
            Get the name of the plugin.

            :returns: The name of the plugin.
            :rtype: str
        )");

        py_dotviewer_plugin.def_property_readonly("version", &DotViewerPlugin::get_version, R"(
            The version of the plugin.

            :type: str
        )");

        py_dotviewer_plugin.def("get_version", &DotViewerPlugin::get_version, R"(
            Get the version of the plugin.

            :returns: The version of the plugin.
            :rtype: str
        )");

        py_dotviewer_plugin.def_property_readonly("description", &DotViewerPlugin::get_description, R"(
            The description of the plugin.

            :type: str
        )");

        py_dotviewer_plugin.def("get_description", &DotViewerPlugin::get_description, R"(
            Get the description of the plugin.

            :returns: The description of the plugin.
            :rtype: str
        )");

        py_dotviewer_plugin.def_property_readonly("dependencies", &DotViewerPlugin::get_dependencies, R"(
            A set of plugin names that this plugin depends on.

            :type: set[str]
        )");

        py_dotviewer_plugin.def("get_dependencies", &DotViewerPlugin::get_dependencies, R"(
            Get a set of plugin names that this plugin depends on.

            :returns: A set of plugin names that this plugin depends on.
            :rtype: set[str]
        )");

        m.def(
            "load_dot_file",
            [](const std::filesystem::path& path, const std::string& creator_plugin = "") -> bool {
                QString qfilename = QString::fromStdString(path.string());
                QString qcreator  = QString::fromStdString(creator_plugin);
                DotViewer* dv     = DotViewer::getDotviewerInstance();
                if (dv)
                {
                    return dv->loadDotFile(qfilename, qcreator);
                }
                else
                {
                    log_error("python_context", "Cannot find dot viewer instance.");
                }
                return false;
            },
            py::arg("path"),
            py::arg("creator_plugin") = std::string(),
            R"(
            Loads a dot file in the graphic viewer provided by dot_viewer plugin.

            :param pathlib.Path path: The path to the ``.dot`` file.
            :param str creator_plugin: The name of plugin that created the ``.dot`` file. Will try to detect from content or query by popup if empty.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
