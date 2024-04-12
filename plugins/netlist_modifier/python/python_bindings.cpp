#include "hal_core/python_bindings/python_bindings.h"

#include "netlist_modifier/netlist_modifier.h"
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
		PYBIND11_MODULE(netlist_modifier, m)
		{
			m.doc() = "hal netlist modifier python bindings";
	#else
		PYBIND11_PLUGIN(netlist_modifier)
		{
			py::module m("netlist_modifier", "hal netlist modifier python bindings");
	#endif
	
	// define all exposed functions
	py::class_<NetlistModifierPlugin, RawPtrWrapper<NetlistModifierPlugin>, BasePluginInterface>(m, "NetlistModifierPlugin")
            .def_property_readonly("name", &NetlistModifierPlugin::get_name)
            .def("get_name", &NetlistModifierPlugin::get_name)
            .def_property_readonly("version", &NetlistModifierPlugin::get_version)
            .def("get_version", &NetlistModifierPlugin::get_version)
            .def(py::init<>())
            .def(
                "create_modified_netlist",
                &NetlistModifierPlugin::create_modified_netlist
            );

	#ifndef PYBIND11_MODULE
		return m.ptr();
	#endif
	}
} // namespace hal