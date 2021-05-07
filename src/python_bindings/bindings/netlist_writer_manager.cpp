#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void netlist_writer_manager_init(py::module& m)
    {
        // hdl_file_writer/HDLWriter
        m.def_submodule("NetlistWriterManager",
                        R"(The netlist writer manager keeps track of all netlist writers that are available within HAL. It is used to dispatch writing tasks to the respective writers.)")
            .def("write",
                 py::overload_cast<Netlist*, const std::filesystem::path&>(&netlist_writer_manager::write),
                 py::arg("netlist"),
                 py::arg("file_path"),
                 R"(
                Write the given netlist into a file at the given location.

                :param hal_py.Netlist netlist: The netlist.
                :param hal_py.hal_path file_path: The output path.
                :returns: True on success, false otherwise.
                :rtype: bool
                )");
    }
}    // namespace hal
