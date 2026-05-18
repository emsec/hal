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
            "split_lut",
            [](Gate* g, bool create_module) -> bool {
                auto res = xilinx_toolbox::split_lut(g, create_module);
                if (res.is_ok())
                {
                    return true;
                }
                log_error("python_context", "{}", res.get_error().get());
                return false;
            },
            py::arg("g"),
            py::arg("create_module") = false,
            R"(
            Split a single ``LUT6_2`` gate into up to two separate LUT gates.

            Creates replacement gates depending on which outputs are connected:

            - ``O6`` → ``LUT6`` using all 64 bits of the INIT string and all 6 inputs.
            - ``O5`` → ``LUT5`` using bits [0, 31] of the INIT string and inputs I0-I4 (I5 is excluded).

            The original ``LUT6_2`` gate is always deleted. The original INIT string is stored on each replacement gate under ``xilinx_preprocessing_information/original_init``.

            :param hal_py.Gate g: The ``LUT6_2`` gate to split.
            :param bool create_module: If ``True``, a new module named after the original gate is created as a child of its parent module and all replacement gates are placed into it. If ``False`` (default), replacement gates inherit the parent module of the original gate.
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        m.def(
            "split_luts",
            [](const std::vector<Gate*>& gates, bool create_module) -> u32 {
                return xilinx_toolbox::split_luts(gates, create_module).get();
            },
            py::arg("gates"),
            py::arg("create_module") = false,
            R"(
            Split a list of ``LUT6_2`` gates into separate LUT gates.

            Calls :func:`split_lut` for each gate in ``gates``. Gates that cannot be split are skipped
            with a warning; processing always continues with the remaining gates.

            :param list[hal_py.Gate] gates: The ``LUT6_2`` gates to split.
            :param bool create_module: If ``True``, each split gate's replacements are placed into a new module named after the original gate. If ``False`` (default), replacements inherit the parent module of the original gate.
            :returns: The number of successfully split gates.
            :rtype: int
        )");

        m.def(
            "split_luts",
            [](Netlist* nl, bool create_module) -> u32 {
                return xilinx_toolbox::split_luts(nl, create_module).get();
            },
            py::arg("nl"),
            py::arg("create_module") = false,
            R"(
            Split all ``LUT6_2`` gates in the netlist into separate LUT gates.

            Finds all gates of type ``LUT6_2`` in the netlist and calls :func:`split_luts` on them.

            :param hal_py.Netlist nl: The netlist to operate on.
            :param bool create_module: If ``True``, each split gate's replacements are placed into a new module named after the original gate. If ``False`` (default), replacements inherit the parent module of the original gate.
            :returns: The number of successfully split ``LUT6_2`` gates.
            :rtype: int
        )");

        m.def(
            "split_shift_register",
            [](Gate* g, bool create_module) -> bool {
                auto res = xilinx_toolbox::split_shift_register(g, create_module);
                if (res.is_ok())
                    return true;
                log_error("python_context", "{}", res.get_error().get());
                return false;
            },
            py::arg("g"),
            py::arg("create_module") = false,
            R"(
            Split a single shift register primitive into an equivalent chain of ``FDCE`` flip-flops.

            Supported gate types: ``SRL16``, ``SRL16E``, ``SRLC16E``, ``SRLC32E``.

            Every address pin must be connected and driven by a constant (GND/VCC) net; an error is
            returned otherwise.  The number of flip-flops created equals ``select_value + 1``, where
            ``select_value`` is the binary value encoded by the address pins (A0 = bit 0).  When the
            cascade output (``Q15`` for ``SRLC16E``, ``Q31`` for ``SRLC32E``) has downstream consumers,
            all stages up to the maximum depth (15 or 31) are materialised so that the last flip-flop
            correctly drives the cascade net.

            The original gate is always deleted on success.

            :param hal_py.Gate g: The shift register gate to split.
            :param bool create_module: If ``True``, a new module named after the original gate is created as a child of its parent module and all replacement flip-flops are placed into it. If ``False`` (default), replacement flip-flops inherit the parent module of the original gate.
            :returns: ``True`` on success, ``False`` if ``g`` is not a supported type, if any address
                      pin is unconnected or not driven by a constant net, or if the gate cannot be deleted.
            :rtype: bool
        )");

        m.def(
            "split_shift_registers",
            [](const std::vector<Gate*>& gates, bool create_module) -> std::optional<u32> {
                auto res = xilinx_toolbox::split_shift_registers(gates, create_module);
                if (res.is_ok())
                    return res.get();
                log_error("python_context", "{}", res.get_error().get());
                return std::nullopt;
            },
            py::arg("gates"),
            py::arg("create_module") = false,
            R"(
            Split a list of shift register primitives into equivalent ``FDCE`` flip-flop chains.

            Supported gate types: ``SRL16``, ``SRL16E``, ``SRLC16E``, ``SRLC32E``.

            Calls :func:`split_shift_register` for each gate in ``gates``. Gates that cannot be split
            (e.g. because an address pin is not driven by a constant net) are skipped with a warning;
            processing always continues with the remaining gates.

            :param list[hal_py.Gate] gates: The shift register gates to split.
            :param bool create_module: If ``True``, each split gate's replacements are placed into a new module named after the original gate. If ``False`` (default), replacements inherit the parent module of the original gate.
            :returns: The number of successfully split gates.
            :rtype: int
        )");

        m.def(
            "split_shift_registers",
            [](Netlist* nl, bool create_module) -> std::optional<u32> {
                auto res = xilinx_toolbox::split_shift_registers(nl, create_module);
                if (res.is_ok())
                    return res.get();
                log_error("python_context", "{}", res.get_error().get());
                return std::nullopt;
            },
            py::arg("nl"),
            py::arg("create_module") = false,
            R"(
            Split all shift register primitives in the netlist into equivalent ``FDCE`` flip-flop chains.

            Supported gate types: ``SRL16``, ``SRL16E``, ``SRLC16E``, ``SRLC32E``.

            Finds all gates of one of the supported types in the netlist and calls
            :func:`split_shift_registers` on them.

            :param hal_py.Netlist nl: The netlist to operate on.
            :param bool create_module: If ``True``, each split gate's replacements are placed into a new module named after the original gate. If ``False`` (default), replacements inherit the parent module of the original gate.
            :returns: The number of successfully split shift register gates.
            :rtype: int
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
