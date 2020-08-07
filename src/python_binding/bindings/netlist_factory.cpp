#include "python_binding/bindings.h"

namespace hal
{
    void netlist_factory_init(py::module& m)
    {
        m.def_submodule("NetlistFactory")
            .def("create_netlist", &netlist_factory::create_netlist, py::arg("gate_library_name"), R"(
                Creates a new netlist for a specific gate library.

                :param str gate_library_name: Name of hardware gate library.
                :returns: The new netlist.
                :rtype: hal_py.Netlist
                )")
            .def("load_netlist",
                 py::overload_cast<const std::filesystem::path&, const std::filesystem::path&>(&netlist_factory::load_netlist),
                 py::arg("hdl_file"),
                 py::arg("gate_library_name"),
                 R"(
                Creates a new netlist for a specific file.

                :param hdl_file: Name of the hdl file.
                :type hdl_file: hal_py.hal_path
                :param gate_library_file: Name of hardware gate library file.
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
                )");
    }
}    // namespace hal
