#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"
#include "hal_core/python_bindings/python_bindings.h"

#include "boolean_influence/plugin_boolean_influence.h"

namespace py = pybind11;

namespace hal
{

    // the name in PYBIND11_MODULE/PYBIND11_PLUGIN *MUST* match the filename of the output library (without extension),
    // otherwise you will get "ImportError: dynamic module does not define module export function" when importing the module

    #ifdef PYBIND11_MODULE
    PYBIND11_MODULE(boolean_influence, m)
    {
        m.doc() = "hal BooleanInfluencePlugin python bindings";
    #else
    PYBIND11_PLUGIN(boolean_influence)
    {
        py::module m("boolean_influence", "hal BooleanInfluencePlugin python bindings");
    #endif    // ifdef PYBIND11_MODULE

        py::class_<BooleanInfluencePlugin, RawPtrWrapper<BooleanInfluencePlugin>, BasePluginInterface>(m, "BooleanInfluencePlugin")
            .def_property_readonly("name", &BooleanInfluencePlugin::get_name)
            .def("get_name", &BooleanInfluencePlugin::get_name)
            .def_property_readonly("version", &BooleanInfluencePlugin::get_version)
            .def("get_version", &BooleanInfluencePlugin::get_version)
            .def("get_boolean_influences_of_gate", &BooleanInfluencePlugin::get_boolean_influences_of_gate, py::arg("gate"), R"(
            Generates the function of the dataport net of the given flip-flop.
            Afterwards the generated function gets translated from a z3::expr to efficent c code, compiled, executed and evalated.

            :param halPy.Gate gate: The flip-flop which data input net is used to build the boolean function..
            :returns: A mapping of the gates that appear in the function of the data net to their boolean influence in said function.
            :rtype: dict
        )");
            ;

    #ifndef PYBIND11_MODULE
        return m.ptr();
    #endif    // PYBIND11_MODULE
    }
}
