#include "bindings.h"

namespace hal
{
    void hdl_writer_dispatcher_init(py::module& m)
    {
        // hdl_file_writer/HDLWriter
        m.def_submodule("HDLWriterDispatcher")
            .def("get_cli_options", &HDLWriterDispatcher::get_cli_options, R"(
                Returns the command line interface options of the hdl writer dispatcher.

                :returns: The options.
                :rtype: list(list(str, list(str), set(str)))
                )")
            .def("write",
                 py::overload_cast<std::shared_ptr<Netlist>, const std::string&, const std::filesystem::path&>(&HDLWriterDispatcher::write),
                 py::arg("netlist"),
                 py::arg("format"),
                 py::arg("file_name"),
                 R"(
                Writes the netlist into a file with a defined format.

                :param netlist: The netlist.
                :type netlist: hal_py.Netlist
                :param format: The target format of the file, e.g. vhdl, verilog...
                :type format: str
                :param file_name: The input file.
                :type file_name: hal_py.hal_path
                :returns: True on success.
                :rtype: bool
                )");
    }
}    // namespace hal
