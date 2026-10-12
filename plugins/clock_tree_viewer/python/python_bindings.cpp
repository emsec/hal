// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
// Copyright (c) 2025-2026 Sascha Tommasone. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "hal_core/python_bindings/python_bindings.h"

#include "clock_tree_viewer/plugin_clock_tree_viewer.h"
#include "hal_core/plugin_system/plugin_interface_base.h"

#include <pybind11/detail/internals.h>
#include <pybind11/pytypes.h>
#include <set>
#include <string>

namespace py = pybind11;

namespace hal
{

    // the name in PYBIND11_MODULE/PYBIND11_PLUGIN *MUST* match the filename of the output library (without extension),
    // otherwise you will get "ImportError: dynamic module does not define module export function" when importing the
    // module

#ifdef PYBIND11_MODULE
    PYBIND11_MODULE( clock_tree_viewer, m )
    {
        m.doc() = "A plugin to interactively view clock trees extracted by the clock_tree_extractor plugin.";
#else
    PYBIND11_PLUGIN( clock_tree_viewer )
    {
        py::module m( "clock_tree_viewer",
                      "A plugin to interactively view clock trees extracted by the clock_tree_extractor plugin." );
#endif  // ifdef PYBIND11_MODULE

        py::class_<ClockTreeViewerPlugin, RawPtrWrapper<ClockTreeViewerPlugin>, BasePluginInterface>
            py_clock_tree_viewer_plugin(
                m,
                "ClockTreeViewerPlugin",
                R"(This class provides an interface to integrate the clock tree viewer as a plugin within the HAL framework.)" );

        py_clock_tree_viewer_plugin.def_property_readonly( "name", &ClockTreeViewerPlugin::get_name, R"(
            The name of the plugin.

            :type: str
        )" );

        py_clock_tree_viewer_plugin.def( "get_name", &ClockTreeViewerPlugin::get_name, R"(
            Get the name of the plugin.

            :returns: The name of the plugin.
            :rtype: str
        )" );

        py_clock_tree_viewer_plugin.def_property_readonly( "version", &ClockTreeViewerPlugin::get_version, R"(
            The version of the plugin.

            :type: str
        )" );

        py_clock_tree_viewer_plugin.def( "get_version", &ClockTreeViewerPlugin::get_version, R"(
            Get the version of the plugin.

            :returns: The version of the plugin.
            :rtype: str
        )" );

        py_clock_tree_viewer_plugin.def_property_readonly( "description", &ClockTreeViewerPlugin::get_description, R"(
            The description of the plugin.

            :type: str
        )" );

        py_clock_tree_viewer_plugin.def( "get_description", &ClockTreeViewerPlugin::get_description, R"(
            Get the description of the plugin.

            :returns: The description of the plugin.
            :rtype: str
        )" );

        py_clock_tree_viewer_plugin.def_property_readonly( "dependencies", &ClockTreeViewerPlugin::get_dependencies, R"(
            A set of plugin names that this plugin depends on.

            :type: set[str]
        )" );

        py_clock_tree_viewer_plugin.def( "get_dependencies", &ClockTreeViewerPlugin::get_dependencies, R"(
            Get a set of plugin names that this plugin depends on.

            :returns: A set of plugin names that this plugin depends on.
            :rtype: set[str]
        )" );

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif  // PYBIND11_MODULE
    }
}  // namespace hal
