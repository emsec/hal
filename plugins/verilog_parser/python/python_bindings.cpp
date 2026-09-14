#include "hal_core/python_bindings/python_bindings.h"

#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"
#include "verilog_parser/plugin_verilog_parser.h"
#include "verilog_parser/verilog_parser.h"

namespace py = pybind11;

namespace hal
{
    // the name in PYBIND11_MODULE/PYBIND11_PLUGIN *MUST* match the filename of the output library (without extension),
    // otherwise you will get "ImportError: dynamic module does not define module export function" when importing the module
#ifdef PYBIND11_MODULE
    PYBIND11_MODULE(verilog_parser, m)
    {
        m.doc() = "The Verilog netlist parser built on the netlist IR.";
#else
    PYBIND11_PLUGIN(verilog_parser)
    {
        py::module m("verilog_parser", "The Verilog netlist parser built on the netlist IR.");
#endif    // ifdef PYBIND11_MODULE

        py::class_<VerilogParserPlugin, RawPtrWrapper<VerilogParserPlugin>, BasePluginInterface> py_verilog_parser_plugin(
            m, "VerilogParserPlugin", R"(This class provides an interface to integrate the Verilog parser as a plugin within the HAL framework.)");

        py_verilog_parser_plugin.def_property_readonly("name", &VerilogParserPlugin::get_name, R"(
            The name of the plugin.

            :type: str
        )");

        py_verilog_parser_plugin.def("get_name", &VerilogParserPlugin::get_name, R"(
            Get the name of the plugin.

            :returns: The name of the plugin.
            :rtype: str
        )");

        py_verilog_parser_plugin.def_property_readonly("version", &VerilogParserPlugin::get_version, R"(
            The version of the plugin.

            :type: str
        )");

        py_verilog_parser_plugin.def("get_version", &VerilogParserPlugin::get_version, R"(
            Get the version of the plugin.

            :returns: The version of the plugin.
            :rtype: str
        )");

        m.def(
            "parse_to_ir",
            [](const std::filesystem::path& file) -> std::optional<netlist_ir::Design> {
                auto res = verilog::parse_to_ir(file);
                if (res.is_ok())
                {
                    return res.get();
                }
                log_error("python_context", "{}", res.get_error().get());
                return std::nullopt;
            },
            py::arg("file"),
            R"(
            Read a structural Verilog file into a netlist IR design, which ``hal_py.netlist_ir.instantiate`` turns into a netlist.

            :param pathlib.Path file: The path of the file.
            :returns: The design on success, ``None`` otherwise (the error is logged).
            :rtype: hal_py.netlist_ir.Design or None
        )");

        m.def(
            "parse_text_to_ir",
            [](const std::string& text, const std::filesystem::path& file) -> std::optional<netlist_ir::Design> {
                auto res = verilog::parse_text_to_ir(text, file);
                if (res.is_ok())
                {
                    return res.get();
                }
                log_error("python_context", "{}", res.get_error().get());
                return std::nullopt;
            },
            py::arg("text"),
            py::arg("file") = std::filesystem::path("<string>"),
            R"(
            Read structural Verilog text into a netlist IR design.

            :param str text: The text.
            :param pathlib.Path file: The path to report in messages and to resolve `include against.
            :returns: The design on success, ``None`` otherwise (the error is logged).
            :rtype: hal_py.netlist_ir.Design or None
        )");

        m.def(
            "parse_and_instantiate",
            [](const std::filesystem::path& file, const GateLibrary* gate_library) -> std::shared_ptr<Netlist> {
                VerilogParser parser;
                auto res = parser.parse_and_instantiate(file, gate_library);
                if (res.is_ok())
                {
                    return std::shared_ptr<Netlist>(res.get().release());
                }
                log_error("python_context", "{}", res.get_error().get());
                return nullptr;
            },
            py::arg("file"),
            py::arg("gate_library"),
            R"(
            Parse a Verilog file and instantiate it against a gate library in one step.

            :param pathlib.Path file: The path of the file.
            :param hal_py.GateLibrary gate_library: The gate library.
            :returns: The netlist on success, ``None`` otherwise (the error is logged).
            :rtype: hal_py.Netlist or None
        )");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
