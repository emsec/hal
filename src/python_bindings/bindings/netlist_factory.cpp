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
                :returns: The netlist on success, ``None`` otherwise.
                :rtype: hal_py.Netlist or None
            )")

            .def(
                "load_netlist",
                [](const std::filesystem::path& netlist_file, const std::filesystem::path& gate_library_file) {
                    return std::shared_ptr<Netlist>(netlist_factory::load_netlist(netlist_file, gate_library_file));
                },
                py::arg("netlist_file"),
                py::arg("gate_library_file") = "",
                R"(
                Create a netlist from the given file using the specified gate library file.
                Will either deserialize ``.hal`` file or call parser plugin for other formats.
                In the latter case the specified gate library file is mandatory.

                :param pathlib.Path netlist_file: Path to the file.
                :param pathlib.Path gate_library_file: Path to the gate library file.
                :returns: The netlist on success, ``None`` otherwise.
                :rtype: hal_py.Netlist or None
            )")

            .def(
                "load_netlist",
                [](const std::filesystem::path& netlist_file, GateLibrary* gate_library) { return std::shared_ptr<Netlist>(netlist_factory::load_netlist(netlist_file, gate_library)); },
                py::arg("netlist_file"),
                py::arg("gate_library"),
                R"(
                Create a netlist from the given file trying to parse it with the specified gate library.
                Will either deserialize ``.hal`` file or call parser plugin for other formats.

                :param pathlib.Path netlist_file: Path to the file.
                :param pathlib.Path gate_library_file: Path to the gate library file.
                :returns: The netlist on success, ``None`` otherwise.
                :rtype: hal_py.Netlist or None
            )")

            .def(
                "load_netlist_from_string",
                [](const std::string& hdl_string, const std::filesystem::path& gate_library_file) {
                    return std::shared_ptr<Netlist>(netlist_factory::load_netlist_from_string(hdl_string, gate_library_file));
                },
                py::arg("hdl_string"),
                py::arg("gate_library_file") = "",
                R"(
                Create a netlist from the given string. 
                The string must contain a netlist in HAL-(JSON)-format.

                :param pathlib.Path netlist_file: The string containing the netlist.
                :param pathlib.Path gate_library_file: Path to the gate library file.
                :returns: The netlist on success, ``None`` otherwise.
                :rtype: hal_py.Netlist or None
            )")

            .def(
                "load_hal_project", [](const std::filesystem::path& project_dir) { return std::shared_ptr<Netlist>(netlist_factory::load_hal_project(project_dir)); }, py::arg("project_dir"), R"(
                Create a netlist using information specified in command line arguments on startup.
                Will either deserialize ``.hal`` file or call parser plugin for other formats.

                :param pathlib.Path project_dir: Path to the hal project directory.
                :returns: The netlist on success, ``None`` otherwise.
                :rtype: hal_py.Netlist or None
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
                py::arg("netlist_file"),
                R"(
                Create a netlist from a given file for each matching pre-loaded gate library.

                :param pathlib.Path netlist_file: Path to the netlist file.
                :returns: A list of netlists, one for each suitable gate library.
                :rtype: list[hal_py.Netlist]
            )");
    }
}    // namespace hal
