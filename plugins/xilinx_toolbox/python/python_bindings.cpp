#include "hal_core/python_bindings/python_bindings.h"

#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"
#include "xilinx_toolbox/plugin_xilinx_toolbox.h"

namespace py = pybind11;

namespace hal
{

    // the name in PYBIND11_MODULE/PYBIND11_PLUGIN *MUST* match the filename of the output library (without extension),
    // otherwise you will get "ImportError: dynamic module does not define module export function" when importing the module

#ifdef PYBIND11_MODULE
    PYBIND11_MODULE(xilinx_toolbox, m)
    {
        m.doc() = "hal XilinxToolboxPlugin python bindings";
#else
    PYBIND11_PLUGIN(xilinx_toolbox)
    {
        py::module m("xilinx_toolbox", "hal XilinxToolboxPlugin python bindings");
#endif    // ifdef PYBIND11_MODULE

        py::class_<XilinxToolboxPlugin, RawPtrWrapper<XilinxToolboxPlugin>, BasePluginInterface> py_xilinx_toolbox(m, "XilinxToolboxPlugin");

        py_xilinx_toolbox.def_property_readonly("name", &XilinxToolboxPlugin::get_name, R"(
            The name of the plugin.

            :type: str
        )");

        py_xilinx_toolbox.def("get_name", &XilinxToolboxPlugin::get_name, R"(
            Get the name of the plugin.

            :returns: Plugin name.
            :rtype: str
        )");

        py_xilinx_toolbox.def_property_readonly("version", &XilinxToolboxPlugin::get_version, R"(
            The version of the plugin.

            :type: str
        )");

        py_xilinx_toolbox.def("get_version", &XilinxToolboxPlugin::get_version, R"(
            Get the version of the plugin.

            :returns: Plugin version.
            :rtype: str
        )");

        py_xilinx_toolbox.def_static(
            "split_luts",
            [](Netlist* nl) -> std::optional<u32> {
                auto res = XilinxToolboxPlugin::split_luts(nl);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("nl"),
            R"(
            Removes all LUTs with multiple outputs and replaces them with equivalent smaller LUTs.

            :param hal_py.Netlist nl: The netlist to operate on. 
            :returns: The number of removed gates on success, None otherwise.
            :rtype: int or None
        )");

        py_xilinx_toolbox.def_static(
            "parse_xdc_file",
            [](Netlist* nl, const std::filesystem::path& xdc_file) -> std::optional<bool> {
                auto res = XilinxToolboxPlugin::parse_xdc_file(nl, xdc_file);
                if (res.is_ok())
                {
                    return true;
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("nl"),
            py::arg("xdc_file"),
            R"(
            Parses an .xdc file and extracts the position LOC and BEL data.
            Afterwards translates the found LOC and BEL data into integer coordinates.

            :param hal_py.Netlist nl: The netlist to operate on. 
            :param path xdc_file: The netlist to operate on. 
            :returns: True on success, None otherwise.
            :rtype: bool or None
        )");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
