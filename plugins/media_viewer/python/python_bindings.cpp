#include "hal_core/python_bindings/python_bindings.h"

#include "media_viewer/media_viewer.h"
#include "media_viewer/plugin_media_viewer.h"
#include "pybind11/pybind11.h"

namespace py = pybind11;

namespace hal
{

    // the name in PYBIND11_MODULE/PYBIND11_PLUGIN *MUST* match the filename of the output library (without extension),
    // otherwise you will get "ImportError: dynamic module does not define module export function" when importing the module

#ifdef PYBIND11_MODULE
    PYBIND11_MODULE(media_viewer, m)
    {
        m.doc() = "Plugin to play multimedia within the HAL GUI.";
#else
    PYBIND11_PLUGIN(media_viewer)
    {
        py::module m("media_viewer", "Plugin to play multimedia within the HAL GUI.");
#endif    // ifdef PYBIND11_MODULE

        py::class_<MediaViewerPlugin, RawPtrWrapper<MediaViewerPlugin>, BasePluginInterface> py_mediaviewer_plugin(
            m, "MediaViewerPlugin", R"(This class provides an interface to multimedia player plugin within the HAL framework.)");

        py_mediaviewer_plugin.def_property_readonly("name", &MediaViewerPlugin::get_name, R"(
            The name of the plugin.

            :type: str
        )");

        py_mediaviewer_plugin.def("get_name", &MediaViewerPlugin::get_name, R"(
            Get the name of the plugin.

            :returns: The name of the plugin.
            :rtype: str
        )");

        py_mediaviewer_plugin.def_property_readonly("version", &MediaViewerPlugin::get_version, R"(
            The version of the plugin.

            :type: str
        )");

        py_mediaviewer_plugin.def("get_version", &MediaViewerPlugin::get_version, R"(
            Get the version of the plugin.

            :returns: The version of the plugin.
            :rtype: str
        )");

        py_mediaviewer_plugin.def_property_readonly("description", &MediaViewerPlugin::get_description, R"(
            The description of the plugin.

            :type: str
        )");

        py_mediaviewer_plugin.def("get_description", &MediaViewerPlugin::get_description, R"(
            Get the description of the plugin.

            :returns: The description of the plugin.
            :rtype: str
        )");

        py_mediaviewer_plugin.def_property_readonly("dependencies", &MediaViewerPlugin::get_dependencies, R"(
            A set of plugin names that this plugin depends on.

            :type: set[str]
        )");

        py_mediaviewer_plugin.def("get_dependencies", &MediaViewerPlugin::get_dependencies, R"(
            Get a set of plugin names that this plugin depends on.

            :returns: A set of plugin names that this plugin depends on.
            :rtype: set[str]
        )");

        m.def(
            "load_media_file",
            [](const std::filesystem::path& path) -> bool {
                QString qfilename = QString::fromStdString(path.string());
                MediaViewer* mv     = MediaViewer::getMediaviewerInstance();
                if (mv)
                {
                    return mv->loadMediaFile(qfilename);
                }
                else
                {
                    log_error("python_context", "Cannot find media viewer instance.");
                }
                return false;
            },
            py::arg("path"),
            R"(
            Loads a media file in the video player provided by media_viewer plugin.

            :param pathlib.Path path: The path to the media file.
            :param str creator_plugin: The name of plugin that created the media file. Will try to detect from content or query by popup if empty.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
