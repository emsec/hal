#include "hal_core/python_bindings/python_bindings.h"

#include "gui_extension_demo/plugin_gui_extension_demo.h"
#include "gui_extension_demo/gui_extension_python_base.h"
#include "hal_core/plugin_system/plugin_parameter.h"
#include "hal_core/netlist/netlist.h"
#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

namespace py = pybind11;
namespace hal
{
#ifdef PYBIND11_MODULE
    PYBIND11_MODULE(gui_extension_demo, m)
    {
        m.doc() = "hal GuiExtensionDemoPlugin python bindings";
#else
    PYBIND11_PLUGIN(gui_extension_demo)
    {
        py::module m("gui_extension_demo", "hal GuiExtensionDemoPlugin python bindings");
#endif    // ifdef PYBIND11_MODULE
        py::class_<GuiExtensionDemoPlugin, RawPtrWrapper<GuiExtensionDemoPlugin>, BasePluginInterface>(m, "GuiExtensionDemoPlugin")
            .def_property_readonly("name", &GuiExtensionDemoPlugin::get_name, R"(
                The name of the plugin.

                :type: str
                )")
            .def("get_name", &GuiExtensionDemoPlugin::get_name, R"(
                Get the name of the plugin.

                :returns: Plugin name.
                :rtype: str
                )")
            .def_property_readonly("version", &GuiExtensionDemoPlugin::get_version, R"(
                The version of the plugin.

                :type: str
                )")
            .def("get_version", &GuiExtensionDemoPlugin::get_version, R"(
                Get the version of the plugin.

                :returns: Plugin version.
                :rtype: str
                )");

        py::enum_<PluginParameter::ParameterType>(m, "ParameterType", R"(
                Defines the type of a gui input parameter.
                )")
                .value("Absent",      PluginParameter::Absent,      R"(Indicate not used.)")
                .value("Boolean",     PluginParameter::Boolean,     R"('true' or 'false'.)")
                .value("Color",       PluginParameter::Color,       R"(Color value like '#ffe080'.)")
                .value("ComboBox",    PluginParameter::ComboBox,    R"(Combo box to select string from semicolon separated input list.)")
                .value("Dictionary",  PluginParameter::Dictionary,  R"(Key value pairs (string).)")
                .value("ExistingDir", PluginParameter::ExistingDir, R"(Existing directory.)")
                .value("Float",       PluginParameter::Float,       R"(Floating point number.)")
                .value("Gate",        PluginParameter::Gate,        R"(Gate ID.)")
                .value("Integer",     PluginParameter::Integer,     R"(Integer number.)")
                .value("Module",      PluginParameter::Gate,        R"(Module ID.)")
                .value("NewFile",     PluginParameter::NewFile,     R"(New file name.)")
                .value("PushButton",  PluginParameter::PushButton,  R"(Push Button.)")
                .value("Label",       PluginParameter::Label,       R"(Text Label.)")
                .value("String",      PluginParameter::String,      R"(String value.)")
                .value("TabName",     PluginParameter::TabName,     R"(Tab name for structuring other elements.)")
                .export_values();

        py::class_<PluginParameter>(m,"PluginParameter")
            .def(py::init<PluginParameter::ParameterType,std::string,std::string,std::string>())
            .def("get_tagname", &PluginParameter::get_tagname, R"(
                 Get tag name of parameter.

                 :returns: Tag name.
                 :rtype: str
            )")
            .def("get_label", &PluginParameter::get_label, R"(
                 Get labl of parameter.

                 :returns: Parameter label.
                 :rtype: str
             )")
             .def("get_value", &PluginParameter::get_value, R"(
                  Get text value parameter.

                  :returns: Parameter value.
                  :rtype: str
             )")
             .def("get_type", &PluginParameter::get_type, R"(
                  Get labl of parameter.

                  :returns: Parameter type.
                  :rtype: ParameterType
             )")
             .def("set_tagname", &PluginParameter::set_tagname, py::arg("tagname"), R"(
                  Set new tag name.
                  :param str tagname: Tag name for parameter.
             )")
             .def("set_value", &PluginParameter::set_value, py::arg("value"), R"(
                  Set parameter value.

                  :param str value: Value for parameter.
             )");

        py::class_<GuiExtensionPythonBase>(m,"GuiExtensionPythonBase")
            .def(py::init<std::string,std::string>())
            .def("add_main_menu", &GuiExtensionPythonBase::add_main_menu, py::arg("params"), R"(
                 Add form to main menu comprising parameter and push button (special parameter).
                 :param list[hal_py.PluginParameter] params: Parameter to be added.
            )")
            .def("add_module_context", &GuiExtensionPythonBase::add_module_context, py::arg("tagname"), py::arg("label"), R"(
                 Add context extension if single module selected.
                 :param str tagname: Tagname of function called from context menu.
                 :param str label: Entry for context menu.
            )")
            .def("add_gate_context", &GuiExtensionPythonBase::add_gate_context, py::arg("tagname"), py::arg("label"), R"(
                 Add context extension if single gate selected.
                 :param str tagname: Tagname of function called from context menu.
                 :param str label: Entry for context menu.
            )")
            .def("add_net_context", &GuiExtensionPythonBase::add_net_context, py::arg("tagname"), py::arg("label"), R"(
                 Add context extension if single net selected.
                 :param str tagname: Tagname of function called from context menu.
                 :param str label: Entry for context menu.
            )")
            .def("get_parameter", &GuiExtensionPythonBase::get_parameter, R"(
                 Get parameter list.
                 :returns: List of parameter.
                 :rtype: list[hal_py.PluginParameter]
            )")
            .def("get_selected_modules", &GuiExtensionPythonBase::get_selected_modules, R"(
                 Get IDs of selected modules.

                 :returns: List of module IDs.
                 :rtype: list[int]
            )")
            .def("get_selected_gates", &GuiExtensionPythonBase::get_selected_gates, R"(
                 Get IDs of selected gates.

                 :returns: List of gate IDs.
                 :rtype: list[int]
            )")
            .def("get_selected_nets", &GuiExtensionPythonBase::get_selected_nets, R"(
                 Get IDs of selected nets.

                 :returns: List of net IDs.
                 :rtype: list[int]
            )")
            .def("get_function_call", &GuiExtensionPythonBase::get_function_call, R"(
                 Get tag name of function triggered by GUI

                 :returns: Function tag name.
                 :rtype: str
            )")
            .def("clear", &GuiExtensionPythonBase::clear, R"(
                 Clear all registered callbacks and function tags.
            )");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
