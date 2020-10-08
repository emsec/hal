#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void netlist_factory_init(py::module& m)
    {
        m.def_submodule("NetlistFactory")
            .def("create_netlist", &netlist_factory::create_netlist, py::arg("gate_library"), R"(
                Create a new empty netlist using the specified gate library.

                :param str gate_library: The gate library.
                :returns: The netlist on success, None otherwise.
                :rtype: hal_py.Netlist or None
            )")

            .def("load_netlist", py::overload_cast<const std::filesystem::path&, const std::filesystem::path&>(&netlist_factory::load_netlist), py::arg("hdl_file"), py::arg("gate_library_file"), R"(
                Create a netlist from the given file using the specified gate library file.

                :param hdl_file: Path to the file.
                :type hdl_file: hal_py.hal_path
                :param gate_library_file: Path to the gate library file.
                :type gate_library_file: hal_py.hal_path
                :returns: The netlist on success, None otherwise.
                :rtype: hal_py.Netlist
            )")

            .def("load_netlist", py::overload_cast<const std::filesystem::path&>(&netlist_factory::load_netlist), py::arg("hal_file"), R"(
                Create a netlist from the given .hal file.

                :param hal_file: Path to the '.hal' file.
                :type hal_file: hal_py.hal_path
                :returns: The netlist on success, None otherwise.
                :rtype: hal_py.Netlist
            )")

            .def("load_netlists", &netlist_factory::load_netlists, py::arg("hdl_file"), R"(
                Create a netlist from a given file for each matching pre-loaded gate library.

                :param hdl_file: Path to the file.
                :type hdl_file: hal_py.hal_path
                :returns: A list of netlists.
                :rtype: list[hal_py.Netlist]
            )");
    }
}    // namespace hal
