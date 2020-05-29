#include "bindings.h"

void netlist_factory_init(py::module& m)
{
    m.def_submodule("netlist_factory")
        .def("create_netlist", &netlist_factory::create_netlist, py::arg("gate_library_name"), R"(
Creates a new netlist for a specific gate library.

:param str gate_library_name: Name of hardware gate library.
:returns: The new netlist.
:rtype: hal_py.netlist
)")
        .def("load_netlist",
             py::overload_cast<const hal::path&, const std::string&, const std::string&>(&netlist_factory::load_netlist),
             py::arg("hdl_file"),
             py::arg("language"),
             py::arg("gate_library_name"),
             R"(
Creates a new netlist for a specific file.

:param hdl_file: Name of the hdl file.
:type hdl_file: hal_py.hal_path
:param str language: Programming language uses in parameter file_name.
:param gate_library_name: Name of hardware gate library.
:returns: The new netlist.
:rtype: hal_py.netlist
)")
        .def("load_netlist", py::overload_cast<const hal::path&>(&netlist_factory::load_netlist), py::arg("hal_file"), R"(
Creates a new netlist for a specific '.hal' file.

:param hal_file: Name of the '.hal' file.
:type hal_file: hal_py.hal_path
:returns: The new netlist.
:rtype: hal_py.netlist
)");
}