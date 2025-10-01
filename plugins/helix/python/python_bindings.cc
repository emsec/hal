#include "hal_core/python_bindings/python_bindings.h"

#include "helix/helix.h"
#include "helix/plugin_helix.h"
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
    PYBIND11_MODULE( helix, m )
    {
        m.doc() = "";
#else
    PYBIND11_PLUGIN( helix )
    {
        py::module m( "helix", "" );
#endif  // ifdef PYBIND11_MODULE

        py::class_<HelixPlugin, RawPtrWrapper<HelixPlugin>, BasePluginInterface> py_helix_plugin(
            m, "HelixPlugin", "" );

        py_helix_plugin.def_property_readonly( "name", &HelixPlugin::get_name, R"(
            The name of the plugin.

            :type: str
        )" );

        py_helix_plugin.def( "get_name", &HelixPlugin::get_name, R"(
        Get the name of the plugin.

        :returns: The name of the plugin.
        :rtype: str
    )" );

        py_helix_plugin.def_property_readonly( "version", &HelixPlugin::get_version, R"(
        The version of the plugin.

        :type: str
    )" );

        py_helix_plugin.def( "get_version", &HelixPlugin::get_version, R"(
        Get the version of the plugin.

        :returns: The version of the plugin.
        :rtype: str
    )" );

        py_helix_plugin.def_property_readonly( "description", &HelixPlugin::get_description, R"(
        The description of the plugin.

        :type: str
    )" );

        py_helix_plugin.def( "get_description", &HelixPlugin::get_description, R"(
        Get the description of the plugin.

        :returns: The description of the plugin.
        :rtype: str
    )" );

        py_helix_plugin.def_property_readonly( "dependencies", &HelixPlugin::get_dependencies, R"(
        A set of plugin names that this plugin depends on.

        :type: set[str]
    )" );

        py_helix_plugin.def( "get_dependencies", &HelixPlugin::get_dependencies, R"(
        Get a set of plugin names that this plugin depends on.

        :returns: A set of plugin names that this plugin depends on.
        :rtype: set[str]
    )" );

        py::class_<helix::Helix, RawPtrWrapper<helix::Helix>> py_helix( m, "Helix", R"(

    )" );

        py_helix.def( "instance",
                      &helix::Helix::instance,
                      R"(

    )" );

        py_helix.def(
            "start",
            []( helix::Helix &self,
                const Netlist *netlist,
                const std::string &host,
                const u16 port,
                const std::vector<std::string> &channels ) -> void { self.start( netlist, host, port, channels ); },
            R"(

    )" );

        py_helix.def( "stop",
                      &helix::Helix::stop,
                      R"(

    )" );

        py_helix.def( "get_netlist", &helix::Helix::get_netlist, R"(
        
    )" );

        py_helix.def( "is_running", &helix::Helix::is_running, R"(

    )" );

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif  // PYBIND11_MODULE
    }
}  // namespace hal
