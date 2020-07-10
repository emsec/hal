#include "bindings.h"

namespace hal
{
    void netlist_factory_init(py::module& m)
    {
        m.def_submodule("netlist_factory")
            .def("create_netlist", &netlist_factory::create_netlist, py::arg("gate_library"), R"(
                Creates a new netlist for a specific gate library.

                :param gate_library: The gate library.
                :type gate_library: hal_py.GateLibrary
                :returns: The new netlist.
                :rtype: hal_py.Netlist
                )")
            .def("load_netlist",
                 py::overload_cast<const std::filesystem::path&, const std::filesystem::path&, const std::string&>(&netlist_factory::load_netlist),
                 py::arg("hdl_file"),
                 py::arg("gate_library_name"),
                 py::arg("parser_name"),
                 R"(
                Creates a new netlist from an HDL file by matching it with a pre-loaded gate library.

                :param hdl_file: The HDL file.
                :type hdl_file: hal_py.hal_path
                :param gate_library_file: Path to the gate library file.
                :type gate_library_file: hal_py.hal_path
                :param str parser_name: Name of the parser to be used.
                :returns: The new netlist.
                :rtype: hal_py.Netlist
                )")
            .def("load_netlist", py::overload_cast<const std::filesystem::path&>(&netlist_factory::load_netlist), py::arg("hal_file"), R"(
                Creates a new netlist for a specific '.hal' file.

                :param hal_file: Name of the '.hal' file.
                :type hal_file: hal_py.hal_path
                :returns: The new netlist.
                :rtype: hal_py.Netlist
                )")
            .def("load_netlists", &netlist_factory::load_netlists, py::arg("hdl_file"), py::arg("parser_name"), R"(
                Creates a new netlist from an HDL file for each matching pre-loaded gate library.

                :param hdl_file: The HDL file.
                :type hdl_file: hal_py.hal_path
                :param str parser_name: Name of the parser to be used.
                :returns: A vector of netlists.
                :rtype: list[hal_py.Netlist]
                )");
    }
}    // namespace hal
