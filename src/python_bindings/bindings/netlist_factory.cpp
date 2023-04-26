#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void netlist_factory_init(py::module& m)
    {
        m.def_submodule("NetlistFactory")
            .def(
                "create_netlist", [](const GateLibrary* gate_library) { return std::shared_ptr<Netlist>(netlist_factory::create_netlist(gate_library)); }, py::arg("gate_library"), R"(
                Create a new empty netlist using the specified gate library.

                :param hal_py.GateLibrary gate_library: The gate library.
                :returns: The netlist on success, None otherwise.
                :rtype: hal_py.Netlist or None
            )")

            .def(
                "load_netlist",
                [](const std::filesystem::path& hdl_file, const std::filesystem::path& gate_library_file) {
                    return std::shared_ptr<Netlist>(netlist_factory::load_netlist(hdl_file, gate_library_file));
                },
                py::arg("hdl_file"),
                py::arg("gate_library_file") = "",
                R"(
                Create a netlist from the given file using the specified gate library file.

                :param pathlib.Path hdl_file: Path to the file.
                :param pathlib.Path gate_library_file: Path to the gate library file.
                :returns: The netlist on success, None otherwise.
                :rtype: hal_py.Netlist
            )")

            .def(
                "load_hal_project", [](const std::filesystem::path& project_dir) { return std::shared_ptr<Netlist>(netlist_factory::load_hal_project(project_dir)); }, py::arg("project_dir"), R"(
                Create a netlist from the given .hal file.

                :param pathlib.Path project_dir: Path to the hal project directory.
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

                :param pathlib.Path hdl_file: Path to the file.
                :returns: A list of netlists.
                :rtype: list[hal_py.Netlist]
            )");
    }
}    // namespace hal
