#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void netlist_factory_init(py::module& m)
    {
        m.def_submodule("NetlistFactory")
            .def(
                "create_netlist", [](const GateLibrary* gate_library) { return std::shared_ptr<Netlist>(netlist_factory::create_netlist(gate_library)); }, py::arg("gate_library"), R"(
                Create a new empty netlist using the specified gate library.

                :param str gate_library: The gate library.
                :returns: The netlist on success, None otherwise.
                :rtype: hal_py.Netlist or None
            )")

            .def(
                "load_netlist",
                [](const std::filesystem::path& hdl_file, const std::filesystem::path& gate_library_file) {
                    return std::shared_ptr<Netlist>(netlist_factory::load_netlist(hdl_file, gate_library_file));
                },
                py::arg("hdl_file"),
                py::arg("gate_library_file"),
                R"(
                Create a netlist from the given file using the specified gate library file.

                :param hdl_file: Path to the file.
                :type hdl_file: hal_py.hal_path
                :param gate_library_file: Path to the gate library file.
                :type gate_library_file: hal_py.hal_path
                :returns: The netlist on success, None otherwise.
                :rtype: hal_py.Netlist
            )")

            .def(
                "load_netlist", [](const std::filesystem::path& hal_file) { return std::shared_ptr<Netlist>(netlist_factory::load_netlist(hal_file)); }, py::arg("hal_file"), R"(
                Create a netlist from the given .hal file.

                :param hal_file: Path to the '.hal' file.
                :type hal_file: hal_py.hal_path
                :returns: The netlist on success, None otherwise.
                :rtype: hal_py.Netlist
            )")

            .def(
                "load_netlists",
                [](const std::filesystem::path& hal_file) {
                    std::vector<std::shared_ptr<Netlist>> result;
                    for (auto& ptr : netlist_factory::load_netlists(hal_file))
                    {
                        result.emplace_back(std::move(ptr));
                    }
                    return result;
                },
                py::arg("hdl_file"),
                R"(
                Create a netlist from a given file for each matching pre-loaded gate library.

                :param hdl_file: Path to the file.
                :type hdl_file: hal_py.hal_path
                :returns: A list of netlists.
                :rtype: list[hal_py.Netlist]
            )");
    }
}    // namespace hal
