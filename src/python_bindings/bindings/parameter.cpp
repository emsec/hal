#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void parameter_init(py::module& m)
    {
        py::class_<Parameter> py_parameter(m, "Parameter", R"(
            A typed, named parameter declaration.
        )");

        py::enum_<Parameter::Type> py_parameter_type(py_parameter, "Type", R"(
            Type tag for typed parameters across the netlist.
        )")
            .value("BitVector", Parameter::Type::BitVector, R"(A multi-bit unsigned integer value.)")
            .value("Enum", Parameter::Type::Enum, R"(A finite, ordered set of named values; encoded as an integer index.)")
            .export_values();

        py_parameter.def(py::init<>());

        py_parameter.def_readwrite("name", &Parameter::name, R"(
            Parameter name.

            :type: str
        )");

        py_parameter.def_readwrite("type", &Parameter::type, R"(
            Data type of the parameter.

            :type: hal_py.Parameter.Type
        )");

        py_parameter.def_readwrite("size", &Parameter::size, R"(
            Bit-width of the parameter. For enums this is `ceil(log2(values.size()))` (>= 1).

            :type: int
        )");

        py_parameter.def_readwrite("default_value", &Parameter::default_value, R"(
            Default value.

            :type: str
        )");

        py_parameter.def_readwrite("enum_values", &Parameter::enum_values, R"(
            Ordered list of enum values; empty for non-enum parameters.

            :type: list(str)
        )");

        py_parameter.def(py::self == py::self);

        py_parameter.def(py::self != py::self);   
    }
}    // namespace hal
