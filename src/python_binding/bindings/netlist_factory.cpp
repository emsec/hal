#include "python_binding/bindings.h"

namespace hal
{
    void netlist_factory_init(py::module& m)
    {
        m.def_submodule("NetlistFactory")
            .def("create_netlist", &netlist_factory::create_netlist, py::arg("gate_library"), R"(
                Creates a new netlist for a specific gate library.

                :param str gate_library: The underlying gate library.
                :returns: The new netlist.
                :rtype: hal_py.Netlist
                )")
            .def("load_netlist",
                 py::overload_cast<const std::filesystem::path&, const std::filesystem::path&>(&netlist_factory::load_netlist),
                 py::arg("hdl_file"),
                 py::arg("gate_library_file"),
                 R"(
                Creates a new netlist for a specific file.

                :param hdl_file: Path to the HDL file.
                :type hdl_file: hal_py.hal_path
                :param gate_library_file: Path to the gate library file.
                :type gate_library_file: hal_py.hal_path
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
            .def("load_netlists", &netlist_factory::load_netlists, py::arg("hdl_file"), R"(
                 Creates a new netlist from an HDL file for each matching pre-loaded gate library.

                 :param hdl_file: Path to the HDL file.
                 :type hdl_file: hal_py.hal_path
                 :returns: A vector of netlists.
                 :rtype: list[hal_py.Netlist]
                 )");
    }
}    // namespace hal
