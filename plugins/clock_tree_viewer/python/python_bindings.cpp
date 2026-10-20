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

#include "clock_tree_extractor/clock_tree.h"
#include "clock_tree_viewer/clock_tree_layout.h"
#include "clock_tree_viewer/clock_tree_viewer.h"
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

        m.def(
            "show_clock_tree",
            []( const cte::ClockTree &clock_tree,
                std::optional<double> node_radius,
                std::optional<double> edge_width,
                std::optional<double> picture_margin,
                std::optional<double> pixels_per_unit ) -> bool {
                // The viewer hands the work to the GUI thread and waits for it, which must not hold up a GUI thread that
                // waits for the interpreter.
                py::gil_scoped_release release;

                ClockTreeViewer *viewer = ClockTreeViewer::instance();
                if( viewer == nullptr )
                {
                    log_error( "clock_tree_viewer", "Cannot find clock tree viewer instance." );
                    return false;
                }

                return viewer->setClockTree(
                    clock_tree, ctv::StyleOverrides{ node_radius, edge_width, picture_margin, pixels_per_unit } );
            },
            py::arg( "clock_tree" ),
            py::arg( "node_radius" ) = py::none(),
            py::arg( "edge_width" ) = py::none(),
            py::arg( "picture_margin" ) = py::none(),
            py::arg( "pixels_per_unit" ) = py::none(),
            R"(
            Plot a clock tree in the clock tree viewer of the GUI.

            The gates are drawn at the coordinates stored in their gate data, ``('generic', 'X')`` and ``('generic', 'Y')``, the same that ``ClockTree.export`` writes to the DOT file. If no gate has coordinates, nothing is drawn and an error is shown. If only some gates have none, those are left out of the plot and a warning is shown.

            The plot is in the colors of HAL's graph view. The background and the connections follow the theme. A gate has the color of the module it belongs to, the color of its grouping if it is in one, or the orange of a selected item if it is selected, and the plot follows all of them when they change.

            The sizes of the drawing follow from the coordinates, from the length of a typical connection in the clock tree. Provide any of them to choose it yourself. All lengths are in the unit of the coordinates.

            :param clock_tree_extractor.ClockTree clock_tree: The clock tree to plot.
            :param float node_radius: The radius of the dot that represents a gate. Defaults to ``None``, which derives it from the coordinates.
            :param float edge_width: The width of the lines that represent connections. Defaults to ``None``, which derives it from the coordinates.
            :param float picture_margin: The empty border around the tree in a saved picture. Defaults to ``None``, which derives it from the coordinates.
            :param float pixels_per_unit: The resolution of a saved picture. Defaults to ``None``, which derives it from the coordinates.
            :returns: ``True`` if the clock tree is plotted, ``False`` otherwise.
            :rtype: bool
        )" );

        m.def(
            "highlight_gates",
            []( const std::vector<Gate *> &gates ) -> std::size_t {
                // The gates are read here, with the interpreter locked: only their IDs are handed to the viewer.
                std::vector<u32> gate_ids;
                gate_ids.reserve( gates.size() );
                for( const Gate *gate : gates )
                {
                    if( gate == nullptr )
                    {
                        continue;
                    }

                    // The ID of a gate of another netlist says nothing about the netlist that is open.
                    if( gNetlist != nullptr && gate->get_netlist() != gNetlist )
                    {
                        log_warning( "clock_tree_viewer",
                                     "Gate with ID {} is not part of the netlist that is open, ignored.",
                                     gate->get_id() );
                        continue;
                    }

                    gate_ids.push_back( gate->get_id() );
                }

                // The viewer hands the work to the GUI thread and waits for it, see show_clock_tree.
                py::gil_scoped_release release;

                ClockTreeViewer *viewer = ClockTreeViewer::instance();
                if( viewer == nullptr )
                {
                    log_error( "clock_tree_viewer", "Cannot find clock tree viewer instance." );
                    return 0;
                }

                return viewer->highlightGates( gate_ids );
            },
            py::arg( "gates" ),
            R"(
            Highlight gates in the clock tree viewer of the GUI, instead of the ones that were highlighted before.

            A highlighted gate has the color of a selected item and a ring around it that is the same size on the screen at every zoom, so it can be found in a plot of the whole chip. Only gates that are plotted can be highlighted. Gates that are not part of the clock tree are ignored, and so are those of the clock tree that have no coordinates and are therefore not plotted. Nothing is selected in HAL by this.

            The gates that are selected in HAL are highlighted without any call to this function, e.g., after ``gui.select(gates)`` in the Python console of the GUI. The last of the two, a selection in HAL and a call to this function, decides which gates are highlighted.

            :param list[hal_py.Gate] gates: The gates to highlight. An empty list removes the highlight.
            :returns: The number of gates that are highlighted, i.e., those of ``gates`` that are plotted. ``0`` if there is no clock tree viewer or no clock tree is plotted.
            :rtype: int
        )" );

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif  // PYBIND11_MODULE
    }
}  // namespace hal
