#include "hal_core/python_bindings/python_bindings.h"

#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"
#include "xilinx_toolbox/plugin_xilinx_toolbox.h"
#include "xilinx_toolbox/preprocessing.h"

namespace py = pybind11;

namespace hal
{

    // the name in PYBIND11_MODULE/PYBIND11_PLUGIN *MUST* match the filename of the output library (without extension),
    // otherwise you will get "ImportError: dynamic module does not define module export function" when importing the module

#ifdef PYBIND11_MODULE
    PYBIND11_MODULE(xilinx_toolbox, m)
    {
        m.doc() = "A collection of functions specifically designed to operate on Xilinx FPGA netlists.";
#else
    PYBIND11_PLUGIN(xilinx_toolbox)
    {
        py::module m("xilinx_toolbox", "A collection of functions specifically designed to operate on Xilinx FPGA netlists.");
#endif    // ifdef PYBIND11_MODULE

        py::class_<XilinxToolboxPlugin, RawPtrWrapper<XilinxToolboxPlugin>, BasePluginInterface> py_xilinx_toolbox_plugin(
            m, "XilinxToolboxPlugin", R"(This class provides an interface to integrate the Xilinx toolbox as a plugin within the HAL framework.)");

        py_xilinx_toolbox_plugin.def_property_readonly("name", &XilinxToolboxPlugin::get_name, R"(
            The name of the plugin.

            :type: str
        )");

        py_xilinx_toolbox_plugin.def("get_name", &XilinxToolboxPlugin::get_name, R"(
            Get the name of the plugin.

            :returns: The name of the plugin.
            :rtype: str
        )");

        py_xilinx_toolbox_plugin.def_property_readonly("version", &XilinxToolboxPlugin::get_version, R"(
            The version of the plugin.

            :type: str
        )");

        py_xilinx_toolbox_plugin.def("get_version", &XilinxToolboxPlugin::get_version, R"(
            Get the version of the plugin.

            :returns: The version of the plugin.
            :rtype: str
        )");

        py_xilinx_toolbox_plugin.def_property_readonly("description", &XilinxToolboxPlugin::get_description, R"(
            The description of the plugin.

            :type: str
        )");

        py_xilinx_toolbox_plugin.def("get_description", &XilinxToolboxPlugin::get_description, R"(
            Get the description of the plugin.

            :returns: The description of the plugin.
            :rtype: str
        )");

        py_xilinx_toolbox_plugin.def_property_readonly("dependencies", &XilinxToolboxPlugin::get_dependencies, R"(
            A set of plugin names that this plugin depends on.

            :type: set[str]
        )");

        py_xilinx_toolbox_plugin.def("get_dependencies", &XilinxToolboxPlugin::get_dependencies, R"(
            Get a set of plugin names that this plugin depends on.

            :returns: A set of plugin names that this plugin depends on.
            :rtype: set[str]
        )");

        m.def(
            "split_luts",
            [](Netlist* nl) -> std::optional<u32> {
                auto res = xilinx_toolbox::split_luts(nl);
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
            Split LUTs with two outputs into two separate LUT gates.
            Replaces ``LUT6_2`` with a ``LUT6`` and a ``LUT5`` gate if the respective outputs of the ``LUT6_2`` are actually used, i.e., connected to other gates.

            :param hal_py.Netlist nl: The netlist to operate on. 
            :returns: The number of split ``LUT6_2`` gates on success, ``None`` otherwise.
            :rtype: int or None
        )");

        m.def(
            "split_shift_registers",
            [](Netlist* nl) -> std::optional<u32> {
                auto res = xilinx_toolbox::split_shift_registers(nl);
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
            Split shift register primitives and replaces them with equivalent flip-flops chains.
            Currently only implemented for gate type ``SRL16E``.

            :param hal_py.Netlist nl: The netlist to operate on. 
            :returns: The number of split shift registers on success, ``None`` otherwise.
            :rtype: int or None
        )");

        m.def(
            "parse_xdc_file",
            [](Netlist* nl, const std::filesystem::path& xdc_file) -> bool {
                auto res = xilinx_toolbox::parse_xdc_file(nl, xdc_file);
                if (res.is_ok())
                {
                    return true;
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return false;
                }
            },
            py::arg("nl"),
            py::arg("xdc_file"),
            R"(
            Parse an ``.xdc`` file and extract the position LOC and BEL data of each gate.
            Translates the coordinates extracted from the ``.xdc`` file into integer values.

            :param hal_py.Netlist nl: The netlist to operate on. 
            :param path xdc_file: The path to the ``.xdc`` file.
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool 
        )");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
