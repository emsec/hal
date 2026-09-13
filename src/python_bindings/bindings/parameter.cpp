#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void parameter_init(py::module& m)
    {
        py::class_<Parameter> py_parameter(m, "Parameter", R"(
            A typed, named parameter declaration.
        )");

        py::enum_<Parameter::Type>(py_parameter, "Type", R"(
            Type tag for typed parameters across the netlist.
        )")
            .value("Boolean", Parameter::Type::Boolean, R"(A logical boolean (``true`` or ``false``).)")
            .value("BitVector", Parameter::Type::BitVector, R"(A multi-bit unsigned integer value (2-state).)")
            .value("LogicVector", Parameter::Type::LogicVector, R"(A multi-bit 9-state logic vector; each bit may be ``0``, ``1``, ``X``, ``Z``, ``U``, ``L``, ``H``, ``W``, or ``-``.)")
            .value("Integer", Parameter::Type::Integer, R"(A signed 64-bit integer in decimal.)")
            .value("String", Parameter::Type::String, R"(An arbitrary string.)")
            .value("Float", Parameter::Type::Float, R"(A double-precision floating-point number.)")
            .value("Time", Parameter::Type::Time, R"(A time value with a unit suffix (``fs``, ``ps``, ``ns``, ``us``, ``ms``, ``s``, ``min``, ``h``).)")
            .value("Enum", Parameter::Type::Enum, R"(A finite, ordered set of named values; encoded as an integer index.)");

        py_parameter.def_static(
            "Boolean",
            [](const std::string& name, const std::string& default_value) -> std::optional<Parameter> {
                auto res = Parameter::Boolean(name, default_value);
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
            py::arg("name"),
            py::arg("default_value"),
            R"(
            Construct a boolean parameter. Accepts only the value strings ``"true"`` and ``"false"``.

            :param str name: Parameter name (must be non-empty).
            :param str default_value: Default value, either ``"true"`` or ``"false"``.
        )");

        py_parameter.def_static(
            "BitVector",
            [](const std::string& name, u16 size, const std::string& default_value) -> std::optional<Parameter> {
                auto res = Parameter::BitVector(name, size, default_value);
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
            py::arg("name"),
            py::arg("size"),
            py::arg("default_value"),
            R"(
            Construct a bit-vector parameter. Values use the prefixes ``0b`` / ``0o`` / ``0x``.

            :param str name: Parameter name (must be non-empty).
            :param int size: Bit-width (must be at least 1; up to 65535).
            :param str default_value: Default value as a string.
        )");

        py_parameter.def_static(
            "LogicVector",
            [](const std::string& name, u16 size, const std::string& default_value) -> std::optional<Parameter> {
                auto res = Parameter::LogicVector(name, size, default_value);
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
            py::arg("name"),
            py::arg("size"),
            py::arg("default_value"),
            R"(
            Construct a 9-state logic-vector parameter. Values use the prefixes ``0b`` / ``0o`` / ``0x``,
            with any of the VHDL ``std_logic`` state characters (``X``, ``Z``, ``U``, ``L``, ``H``, ``W``, ``-``)
            appearing in place of digits (case-insensitive; for octal/hex, a state-character digit expands to
            3/4 copies of that state).

            :param str name: Parameter name (must be non-empty).
            :param int size: Bit-width (must be at least 1; up to 65535).
            :param str default_value: Default value as a 9-state literal string (empty for "no default").
        )");

        py_parameter.def_static(
            "Integer",
            [](const std::string& name, const std::string& default_value) -> std::optional<Parameter> {
                auto res = Parameter::Integer(name, default_value);
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
            py::arg("name"),
            py::arg("default_value"),
            R"(
            Construct a signed-integer parameter. Values are parsed in base 10 and must fit in ``i64``.

            :param str name: Parameter name (must be non-empty).
            :param str default_value: Default value as a decimal string (optional leading ``+``/``-``).
        )");

        py_parameter.def_static(
            "String",
            [](const std::string& name, const std::string& default_value) -> std::optional<Parameter> {
                auto res = Parameter::String(name, default_value);
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
            py::arg("name"),
            py::arg("default_value"),
            R"(
            Construct a string parameter. Any value (including the empty string) is accepted.

            :param str name: Parameter name (must be non-empty).
            :param str default_value: Default value.
        )");

        py_parameter.def_static(
            "Float",
            [](const std::string& name, const std::string& default_value) -> std::optional<Parameter> {
                auto res = Parameter::Float(name, default_value);
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
            py::arg("name"),
            py::arg("default_value"),
            R"(
            Construct a floating-point parameter. Values are parsed via ``std::stod``.

            :param str name: Parameter name (must be non-empty).
            :param str default_value: Default value as a decimal float (e.g. ``3.14``, ``1e-5``).
        )");

        py_parameter.def_static(
            "Time",
            [](const std::string& name, const std::string& default_value) -> std::optional<Parameter> {
                auto res = Parameter::Time(name, default_value);
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
            py::arg("name"),
            py::arg("default_value"),
            R"(
            Construct a time parameter. Values are of the form ``<number><unit>`` with the unit
            in {``fs``, ``ps``, ``ns``, ``us``, ``ms``, ``s``, ``min``, ``h``}; the number is a
            non-negative decimal.

            :param str name: Parameter name (must be non-empty).
            :param str default_value: Default value, e.g. ``10ns``.
        )");

        py_parameter.def_static(
            "Enum",
            [](const std::string& name, const std::vector<std::string>& values, const std::string& default_value) -> std::optional<Parameter> {
                auto res = Parameter::Enum(name, values, default_value);
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
            py::arg("name"),
            py::arg("values"),
            py::arg("default_value"),
            R"(
            Construct an enum parameter. ``size`` is auto-computed as ``ceil(log2(len(values)))``.

            :param str name: Parameter name (must be non-empty).
            :param list[str] values: Ordered list of enum values (at least two, unique, non-empty).
            :param str default_value: Default value, must equal one of `values`.
        )");

        py_parameter.def_property_readonly("name", &Parameter::get_name, R"(
            The name of the parameter (read-only).

            :type: str
        )");

        py_parameter.def("get_name", &Parameter::get_name, R"(
            Get the name of the parameter.

            :returns: The parameter name.
            :rtype: str
        )");

        py_parameter.def_property_readonly("type", &Parameter::get_type, R"(
            The data type of the parameter (read-only).

            :type: hal_py.Parameter.Type
        )");

        py_parameter.def("get_type", &Parameter::get_type, R"(
            Get the data type of the parameter.

            :returns: The parameter type.
            :rtype: hal_py.Parameter.Type
        )");

        py_parameter.def_property_readonly("size", &Parameter::get_size, R"(
            The bit-width of the parameter (read-only).
            Fixed at 1 for ``Boolean``, 64 for ``Integer`` and ``Float``, 0 for ``String`` and ``Time``,
            ``ceil(log2(N))`` for ``Enum`` with N values, and the declared width for ``BitVector`` / ``LogicVector``.

            :type: int
        )");

        py_parameter.def("get_size", &Parameter::get_size, R"(
            Get the bit-width of the parameter.
            Fixed at 1 for ``Boolean``, 64 for ``Integer`` and ``Float``, 0 for ``String`` and ``Time``,
            ``ceil(log2(N))`` for ``Enum`` with N values, and the declared width for ``BitVector`` / ``LogicVector``.

            :returns: The bit-width.
            :rtype: int
        )");

        py_parameter.def_property_readonly("default_value", &Parameter::get_default_value, R"(
            The default value of the parameter as a string (read-only).
            An empty string signals "no default"; all other values satisfy the type's grammar.

            :type: str
        )");

        py_parameter.def("get_default_value", &Parameter::get_default_value, R"(
            Get the default value of the parameter as a string.
            An empty string signals "no default"; all other values satisfy the type's grammar.

            :returns: The default value string.
            :rtype: str
        )");

        py_parameter.def_property_readonly("enum_values", &Parameter::get_enum_values, R"(
            The ordered list of named values for an ``Enum`` parameter (read-only).
            Always empty for every other type.

            :type: list[str]
        )");

        py_parameter.def("get_enum_values", &Parameter::get_enum_values, R"(
            Get the ordered list of named values for an ``Enum`` parameter.
            Always empty for every other type.

            :returns: The enum value list.
            :rtype: list[str]
        )");

        py_parameter.def("validate", &Parameter::validate, py::arg("value"), R"(
            Check whether a candidate value is valid for this parameter declaration.

            Per-type rules:

              * ``Boolean``: value must be ``"true"`` or ``"false"``.
              * ``BitVector``: parses as a non-negative integer (``0b``, ``0o``, ``0x``) that fits in ``size``.
              * ``LogicVector``: parses as a 4-state literal (``0b``, ``0o``, ``0x``) whose total bit-length fits in ``size``.
              * ``Integer``: parses as a signed decimal integer (fits in ``i64``).
              * ``String``: any value (including the empty string) is accepted.
              * ``Float``: parses as a ``double`` via ``std::stod``.
              * ``Time``: matches ``<number><unit>`` with unit in {``fs``, ``ps``, ``ns``, ``us``, ``ms``, ``s``, ``min``, ``h``}.
              * ``Enum``: must equal one of the declared enum values.

            :param str value: The candidate value string.
            :returns: ``True`` if the value is valid, ``False`` otherwise.
            :rtype: bool
        )");

        py_parameter.def(
            "encode_as_int",
            [](Parameter& self, const std::string& value) -> std::optional<u64> {
                auto res = self.encode_as_int(value);
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
            py::arg("value"),
            R"(
            Convert a string value to its canonical integer encoding for this parameter.

            Per-type rules:

              * ``Boolean``: returns 0 for ``false``, 1 for ``true``.
              * ``BitVector``: parses the value (``0b``, ``0o``, ``0x``).
              * ``Integer``: returns the bit-pattern of the signed value reinterpreted as ``u64``.
              * ``LogicVector``, ``String``, ``Float``, ``Time``: no canonical integer encoding; returns ``None``.
              * ``Enum``: returns the index of the value in the declared value list.

            :param str value: The string value to encode.
            :returns: The integer encoding on success, ``None`` otherwise.
            :rtype: int or None
        )");

        py_parameter.def(py::self == py::self);
        py_parameter.def(py::self != py::self);
    }
}    // namespace hal
