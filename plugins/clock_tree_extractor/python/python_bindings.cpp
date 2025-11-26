#include "hal_core/python_bindings/python_bindings.h"

#include "clock_tree_extractor/clock_tree.h"
#include "clock_tree_extractor/plugin_clock_tree_extractor.h"
#include "pybind11/pybind11.h"

#include <igraph/igraph.h>
#include <pybind11/detail/descr.h>
#include <pybind11/pytypes.h>
#include <set>
#include <string>
#include <vector>

namespace hal
{
    class BasePluginInterface;
}
namespace hal
{
    class Netlist;
}

namespace py = pybind11;

namespace hal
{

    // the name in PYBIND11_MODULE/PYBIND11_PLUGIN *MUST* match the filename of the output library (without extension),
    // otherwise you will get "ImportError: dynamic module does not define module export function" when importing the
    // module

#ifdef PYBIND11_MODULE
    PYBIND11_MODULE( clock_tree_extractor, m )
    {
        m.doc() = "";
#else
    PYBIND11_PLUGIN( clock_tree_extractor )
    {
        py::module m( "clock_tree_extractor", "" );
#endif  // ifdef PYBIND11_MODULE

        py::class_<ClockTreeExtractorPlugin, RawPtrWrapper<ClockTreeExtractorPlugin>, BasePluginInterface>
            py_clock_tree_extractor_plugin( m, "ClockTreeExtractorPlugin", "" );

        py_clock_tree_extractor_plugin.def_property_readonly( "name", &ClockTreeExtractorPlugin::get_name, R"(
        The name of the plugin.

        :type: str
    )" );

        py_clock_tree_extractor_plugin.def( "get_name", &ClockTreeExtractorPlugin::get_name, R"(
    Get the name of the plugin.

    :returns: The name of the plugin.
    :rtype: str
    )" );

        py_clock_tree_extractor_plugin.def_property_readonly( "version", &ClockTreeExtractorPlugin::get_version, R"(
    The version of the plugin.

    :type: str
    )" );

        py_clock_tree_extractor_plugin.def( "get_version", &ClockTreeExtractorPlugin::get_version, R"(
    Get the version of the plugin.

    :returns: The version of the plugin.
    :rtype: str
    )" );

        py_clock_tree_extractor_plugin.def_property_readonly(
            "description", &ClockTreeExtractorPlugin::get_description, R"(
    The description of the plugin.

    :type: str
    )" );

        py_clock_tree_extractor_plugin.def( "get_description", &ClockTreeExtractorPlugin::get_description, R"(
    Get the description of the plugin.

    :returns: The description of the plugin.
    :rtype: str
    )" );

        py_clock_tree_extractor_plugin.def_property_readonly(
            "dependencies", &ClockTreeExtractorPlugin::get_dependencies, R"(
    A set of plugin names that this plugin depends on.

    :type: set[str]
    )" );

        py_clock_tree_extractor_plugin.def( "get_dependencies", &ClockTreeExtractorPlugin::get_dependencies, R"(
    Get a set of plugin names that this plugin depends on.

    :returns: A set of plugin names that this plugin depends on.
    :rtype: set[str]
    )" );

        py::class_<cte::ClockTree, RawPtrWrapper<cte::ClockTree>>( m, "ClockTree", R"()" )
            .def_static(
                "from_netlist",
                []( const Netlist *netlist ) -> std::unique_ptr<cte::ClockTree> {
                    auto result = cte::ClockTree::from_netlist( netlist );
                    if( result.is_ok() )
                    {
                        return result.get();
                    }

                    log_error( "clock_tree_extractor", "{}", result.get_error().get() );
                    return nullptr;
                },
                py::arg( "netlist" ),
                R"()" )
            .def(
                "export",
                []( const cte::ClockTree &self, const std::string &pathname ) -> bool {
                    auto result = self.export_dot( pathname );
                    if( result.is_ok() )
                    {
                        return true;
                    }

                    log_error( "clock_tree_extractor", "{}", result.get_error().get() );
                    return false;
                },
                py::arg( "pathname" ),
                R"()" )
            .def(
                "get_subtree",
                []( const cte::ClockTree &self,
                    const void *ptr,
                    const bool parent ) -> std::unique_ptr<cte::ClockTree> {
                    auto result = self.get_subtree( ptr, parent );
                    if( result.is_ok() )
                    {
                        return result.get();
                    }

                    log_error( "clock_tree_extractor", "{}", result.get_error().get() );
                    return nullptr;
                },
                py::arg( "ptr" ),
                py::arg( "parent" ) = false,
                py::return_value_policy::move,
                R"()" )
            .def(
                "get_all",
                []( const cte::ClockTree &self ) -> py::list {
                    py::list result;
                    const auto &map = self.get_all();
                    for( auto &[ptr, type] : map )
                    {
                        if( type == cte::PtrType::GATE )
                        {
                            result.append( py::cast( (const Gate *) ptr ) );
                        }
                        else if( type == cte::PtrType::NET )
                        {
                            result.append( py::cast( (const Net *) ptr ) );
                        }
                    }
                    return result;
                },
                R"()" )
            .def( "get_vertex_from_ptr", &cte::ClockTree::get_vertex_from_ptr, R"()" )
            .def( "get_gates", &cte::ClockTree::get_gates, R"()" )
            .def( "get_nets", &cte::ClockTree::get_nets, R"()" )
            .def( "get_netlist", &cte::ClockTree::get_netlist, R"()" );

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif  // PYBIND11_MODULE
    }
}  // namespace hal