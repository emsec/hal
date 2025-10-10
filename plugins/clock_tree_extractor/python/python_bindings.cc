#include "hal_core/python_bindings/python_bindings.h"

#include "clock_tree_extractor/clock_tree_extractor.h"
#include "clock_tree_extractor/plugin_clock_tree_extractor.h"
#include "pybind11/pybind11.h"

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

        py::class_<cte::ClockTreeExtractor, RawPtrWrapper<cte::ClockTreeExtractor>> py_clock_tree_extractor(
            m, "ClockTreeExtractor", R"(

    )" );

        py_clock_tree_extractor.def( py::init<const Netlist *>(), py::arg( "netlist" ), R"(

    )" );

        py_clock_tree_extractor.def(
            "analyze",
            []( cte::ClockTreeExtractor &self, const std::string &pathname ) -> std::optional<u32> {
                auto res = self.analyze( pathname );
                if( res.is_ok() )
                {
                    return res.get();
                }
                else
                {
                    log_error( "python_context", "{}", res.get_error().get() );
                    return std::nullopt;
                }
            },
            py::arg( "pathname" ),
            R"(

    )" );

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif  // PYBIND11_MODULE
    }
}  // namespace hal