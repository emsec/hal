#include "netlist/netlist.h"
#include "plugin_igraph.h"
#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

namespace py = pybind11;

// namespace pybind11
// {
//     namespace detail
//     {
//         template<>
//         struct type_caster<igraph_t>
//         {
//         public:
//             /**
//          * This macro establishes the name 'igraph_t' in
//          * function signatures and declares a local variable
//          * 'value' of type igraph_t
//          */
//             PYBIND11_TYPE_CASTER(igraph_t, _("igraph_t"));

//             /**
//          * Conversion part 1 (Python->C++): convert a PyObject into a inty
//          * instance or return false upon failure. The second argument
//          * indicates whether implicit conversions should be applied.
//          */
//             bool load(handle src, bool)
//             {
//                 /* Extract PyObject from handle */
//                 PyObject* source = src.ptr();
//                 /* Try converting into a Python integer value */
//                 auto r = igraphmodule_PyObject_to_igraph_t(sourde, value)

//                     PyObject* tmp = PyNumber_Long(source);
//                 if (!tmp)
//                     return false;
//                 /* Now try to convert into a C++ int */
//                 value.long_value = PyLong_AsLong(tmp);
//                 Py_DECREF(tmp);
//                 /* Ensure return code was OK (to avoid out-of-range errors etc) */
//                 return !(value.long_value == -1 && !PyErr_Occurred());
//             }

//             /**
//          * Conversion part 2 (C++ -> Python): convert an inty instance into
//          * a Python object. The second and third arguments are used to
//          * indicate the return value policy and parent object (for
//          * ``return_value_policy::reference_internal``) and are generally
//          * ignored by implicit casters.
//          */
//             static handle cast(inty src, return_value_policy /* policy */, handle /* parent */)
//             {
//                 return PyLong_FromLong(src.long_value);
//             }
//         };
//     }    // namespace detail
// }    // namespace pybind11

#ifdef PYBIND11_MODULE
PYBIND11_MODULE(libigraph_converter, m)
{
    m.doc() = "hal igraph python bindings";
#else
PYBIND11_PLUGIN(libigraph_converter)
{
    py::module m("libigraph_converter", "hal igraph python bindings");
#endif    // ifdef PYBIND11_MODULE

    py::module::import("igraph");

    py::class_<plugin_igraph, std::shared_ptr<plugin_igraph>, i_base>(m, "igraph_converter")
        .def(py::init<>())
        .def_property_readonly("name", &plugin_igraph::get_name)
        .def("get_name", &plugin_igraph::get_name)
        .def_property_readonly("version", &plugin_igraph::get_version)
        .def("get_version", &plugin_igraph::get_version)
    .def("get_igraph_directed", &plugin_igraph::get_igraph_directed);

#ifndef PYBIND11_MODULE
    return m.ptr();
#endif    // PYBIND11_MODULE
}
