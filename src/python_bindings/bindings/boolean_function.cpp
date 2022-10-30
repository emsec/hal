#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void boolean_function_init(py::module& m)
    {
        py::class_<BooleanFunction> py_boolean_function(
            m,
            "BooleanFunction",
            R"(A BooleanFunction represents a symbolic expression (e.g., "A & B") in order to abstract the (semantic) functionality of a single netlist gate (or even a complex subcircuit comprising multiple gates) in a formal manner. To this end, the BooleanFunction class is able to construct and display arbitrarily-nested expressions, enable symbolic simplification (e.g., simplify "A & 0" to "0"), and translate Boolean functions to the SAT / SMT solver domain to use the solve constraint formulas.)");

        py::enum_<BooleanFunction::Value> py_boolean_function_value(py_boolean_function, "Value", R"(
            Represents the logic value that a Boolean function operates on.
        )");

        py_boolean_function_value.value("ZERO", BooleanFunction::Value::ZERO, R"(Represents a logical 0.)")
            .value("ONE", BooleanFunction::Value::ONE, R"(Represents a logical 1.)")
            .value("Z", BooleanFunction::Value::Z, R"(Represents a high-impedance value.)")
            .value("X", BooleanFunction::Value::X, R"(Represents an undefined value.)")
            .export_values();

        py_boolean_function_value.def(
            "__str__", [](const BooleanFunction::Value& v) { return BooleanFunction::to_string(v); }, R"(
            Translates the Boolean function value into its string representation.

            :returns: The value as a string.
            :rtype: str
        )");

        py_boolean_function.def_static(
            "to_string",
            [](const std::vector<BooleanFunction::Value>& value, u8 base = 2) -> std::optional<std::string> {
                auto res = BooleanFunction::to_string(value, base);
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
            py::arg("base") = 2,
            R"(
            Convert the given bit-vector to its string representation in the given base.

            :param list[hal_py.BooleanFunction.Value] value: The value as a bit-vector.
            :param int base: The base that the values should be converted to. Valid values are 2 (default), 8, 10, and 16.
            :returns: A string representing the values in the given base on success, None otherwise.
            :rtype: str or None
        )");

        py_boolean_function.def(py::init<>(), R"(
            Constructs an empty / invalid Boolean function.
        )");

        py_boolean_function.def_static("Var", &BooleanFunction::Var, py::arg("name"), py::arg("size") = 1, R"(
            Creates a multi-bit Boolean function of the given bit-size comprising only a variable of the specified name.

            :param str name: The name of the variable.
            :param int size: The bit-size. Defaults to 1.
            :returns: The BooleanFunction.
            :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def_static("Const", py::overload_cast<const BooleanFunction::Value&>(&BooleanFunction::Const), py::arg("value"), R"(
            Creates a constant single-bit Boolean function from a value.

            :param hal_py.BooleanFunction.Value value: The value.
            :returns: The Boolean function.
            :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def_static("Const", py::overload_cast<const std::vector<BooleanFunction::Value>&>(&BooleanFunction::Const), py::arg("value"), R"(
            Creates a constant multi-bit Boolean function from a list of values.

            :param list[hal_py.BooleanFunction.Value] value: The list of values.
            :returns: The Boolean function.
            :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def_static("Const", py::overload_cast<u64, u16>(&BooleanFunction::Const), py::arg("value"), py::arg("size"), R"(
            Creates a constant multi-bit Boolean function of the given bit-size from an integer value.

            :param int value: The integer value.
            :param int size: The bit-size.
            :returns: The Boolean function.
            :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def_static("Index", &BooleanFunction::Index, py::arg("index"), py::arg("size"), R"(
            Creates an index for a Boolean function of the given bit-size from an integer value.

            :param int index: The integer value.
            :param int size: The bit-size.
            :returns: The Boolean function.
            :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def_static(
            "And",
            [](BooleanFunction p0, BooleanFunction p1, u16 size) -> std::optional<BooleanFunction> {
                auto res = BooleanFunction::And(std::move(p0), std::move(p1), size);
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
            py::arg("p0"),
            py::arg("p1"),
            py::arg("size"),
            R"(
            Joins two Boolean functions by applying an 'AND' operation. 
            Requires both Boolean functions to be of the specified bit-size and produces a new Boolean function of the same bit-size.

            :param hal_py.BooleanFunction p0: First Boolean function.
            :param hal_py.BooleanFunction p1: Second Boolean function.
            :param int size: Bit-size of the operation.
            :returns: The joined Boolean function on success, None otherwise.
            :rtype: hal_py.BooleanFunction or None
        )");

        py_boolean_function.def_static(
            "Or",
            [](BooleanFunction p0, BooleanFunction p1, u16 size) -> std::optional<BooleanFunction> {
                auto res = BooleanFunction::Or(std::move(p0), std::move(p1), size);
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
            py::arg("p0"),
            py::arg("p1"),
            py::arg("size"),
            R"(
            Joins two Boolean functions by applying an 'OR' operation. 
            Requires both Boolean functions to be of the specified bit-size and produces a new Boolean function of the same bit-size.

            :param hal_py.BooleanFunction p0: First Boolean function.
            :param hal_py.BooleanFunction p1: Second Boolean function.
            :param int size: Bit-size of the operation.
            :returns: The joined Boolean function on success, None otherwise.
            :rtype: hal_py.BooleanFunction or None
        )");

        py_boolean_function.def_static(
            "Not",
            [](BooleanFunction p0, u16 size) -> std::optional<BooleanFunction> {
                auto res = BooleanFunction::Not(std::move(p0), size);
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
            py::arg("p0"),
            py::arg("size"),
            R"(
            Negates the given Boolean function. 
            Requires the Boolean function to be of the specified bit-size and produces a new Boolean function of the same bit-size.

            :param hal_py.BooleanFunction p0: The Boolean function to negate.
            :param int size: Bit-size of the operation.
            :returns: The negated Boolean function on success, None otherwise.
            :rtype: hal_py.BooleanFunction or None
        )");

        py_boolean_function.def_static(
            "Xor",
            [](BooleanFunction p0, BooleanFunction p1, u16 size) -> std::optional<BooleanFunction> {
                auto res = BooleanFunction::Xor(std::move(p0), std::move(p1), size);
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
            py::arg("p0"),
            py::arg("p1"),
            py::arg("size"),
            R"(
            Joins two Boolean functions by applying an 'XOR' operation. 
            Requires both Boolean functions to be of the specified bit-size and produces a new Boolean function of the same bit-size.

            :param hal_py.BooleanFunction p0: First Boolean function.
            :param hal_py.BooleanFunction p1: Second Boolean function.
            :param int size: Bit-size of the operation.
            :returns: The joined Boolean function on success, None otherwise.
            :rtype: hal_py.BooleanFunction or None
        )");

        py_boolean_function.def_static(
            "Add",
            [](BooleanFunction p0, BooleanFunction p1, u16 size) -> std::optional<BooleanFunction> {
                auto res = BooleanFunction::Add(std::move(p0), std::move(p1), size);
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
            py::arg("p0"),
            py::arg("p1"),
            py::arg("size"),
            R"(
            Joins two Boolean functions by applying an 'ADD' operation. 
            Requires both Boolean functions to be of the specified bit-size and produces a new Boolean function of the same bit-size.

            :param hal_py.BooleanFunction p0: First Boolean function.
            :param hal_py.BooleanFunction p1: Second Boolean function.
            :param int size: Bit-size of the operation.
            :returns: The joined Boolean function on success, None otherwise.
            :rtype: hal_py.BooleanFunction or None
        )");

        py_boolean_function.def_static(
            "Sub",
            [](BooleanFunction p0, BooleanFunction p1, u16 size) -> std::optional<BooleanFunction> {
                auto res = BooleanFunction::Sub(std::move(p0), std::move(p1), size);
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
            py::arg("p0"),
            py::arg("p1"),
            py::arg("size"),
            R"(
            Joins two Boolean functions by applying an 'SUB' operation. 
            Requires both Boolean functions to be of the specified bit-size and produces a new Boolean function of the same bit-size.

            :param hal_py.BooleanFunction p0: First Boolean function.
            :param hal_py.BooleanFunction p1: Second Boolean function.
            :param int size: Bit-size of the operation.
            :returns: The joined Boolean function on success, None otherwise.
            :rtype: hal_py.BooleanFunction or None
        )");

        py_boolean_function.def_static(
            "Mul",
            [](BooleanFunction p0, BooleanFunction p1, u16 size) -> std::optional<BooleanFunction> {
                auto res = BooleanFunction::Mul(std::move(p0), std::move(p1), size);
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
            py::arg("p0"),
            py::arg("p1"),
            py::arg("size"),
            R"(
            Joins two Boolean functions by applying an 'MUL' operation. 
            Requires both Boolean functions to be of the specified bit-size and produces a new Boolean function of the same bit-size.

            :param hal_py.BooleanFunction p0: First Boolean function.
            :param hal_py.BooleanFunction p1: Second Boolean function.
            :param int size: Bit-size of the operation.
            :returns: The joined Boolean function on success, None otherwise.
            :rtype: hal_py.BooleanFunction or None
        )");

        py_boolean_function.def_static(
            "Sdiv",
            [](BooleanFunction p0, BooleanFunction p1, u16 size) -> std::optional<BooleanFunction> {
                auto res = BooleanFunction::Sdiv(std::move(p0), std::move(p1), size);
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
            py::arg("p0"),
            py::arg("p1"),
            py::arg("size"),
            R"(
            Joins two Boolean functions by applying an 'SDIV' operation. 
            Requires both Boolean functions to be of the specified bit-size and produces a new Boolean function of the same bit-size.

            :param hal_py.BooleanFunction p0: First Boolean function.
            :param hal_py.BooleanFunction p1: Second Boolean function.
            :param int size: Bit-size of the operation.
            :returns: The joined Boolean function on success, None otherwise.
            :rtype: hal_py.BooleanFunction or None
        )");

        py_boolean_function.def_static(
            "Udiv",
            [](BooleanFunction p0, BooleanFunction p1, u16 size) -> std::optional<BooleanFunction> {
                auto res = BooleanFunction::Udiv(std::move(p0), std::move(p1), size);
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
            py::arg("p0"),
            py::arg("p1"),
            py::arg("size"),
            R"(
            Joins two Boolean functions by applying an 'UDIV' operation. 
            Requires both Boolean functions to be of the specified bit-size and produces a new Boolean function of the same bit-size.

            :param hal_py.BooleanFunction p0: First Boolean function.
            :param hal_py.BooleanFunction p1: Second Boolean function.
            :param int size: Bit-size of the operation.
            :returns: The joined Boolean function on success, None otherwise.
            :rtype: hal_py.BooleanFunction or None
        )");

        py_boolean_function.def_static(
            "Srem",
            [](BooleanFunction p0, BooleanFunction p1, u16 size) -> std::optional<BooleanFunction> {
                auto res = BooleanFunction::Srem(std::move(p0), std::move(p1), size);
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
            py::arg("p0"),
            py::arg("p1"),
            py::arg("size"),
            R"(
            Joins two Boolean functions by applying an 'SREM' operation. 
            Requires both Boolean functions to be of the specified bit-size and produces a new Boolean function of the same bit-size.

            :param hal_py.BooleanFunction p0: First Boolean function.
            :param hal_py.BooleanFunction p1: Second Boolean function.
            :param int size: Bit-size of the operation.
            :returns: The joined Boolean function on success, None otherwise.
            :rtype: hal_py.BooleanFunction or None
        )");

        py_boolean_function.def_static(
            "Urem",
            [](BooleanFunction p0, BooleanFunction p1, u16 size) -> std::optional<BooleanFunction> {
                auto res = BooleanFunction::Urem(std::move(p0), std::move(p1), size);
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
            py::arg("p0"),
            py::arg("p1"),
            py::arg("size"),
            R"(
            Joins two Boolean functions by applying an 'UREM' operation. 
            Requires both Boolean functions to be of the specified bit-size and produces a new Boolean function of the same bit-size.

            :param hal_py.BooleanFunction p0: First Boolean function.
            :param hal_py.BooleanFunction p1: Second Boolean function.
            :param int size: Bit-size of the operation.
            :returns: The joined Boolean function on success, None otherwise.
            :rtype: hal_py.BooleanFunction or None
        )");

        py_boolean_function.def_static(
            "Slice",
            [](BooleanFunction p0, BooleanFunction p1, BooleanFunction p2, u16 size) -> std::optional<BooleanFunction> {
                auto res = BooleanFunction::Slice(std::move(p0), std::move(p1), std::move(p2), size);
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
            py::arg("p0"),
            py::arg("p1"),
            py::arg("p2"),
            py::arg("size"),
            R"(
            Returns the slice [i:j] of a Boolean function specified by a start index 'i' and an end index 'j' beginning at 0.
            Note that slice [i:j] includes positions 'i' and 'j' as well. 

            :param hal_py.BooleanFunction p0: Boolean function to slice.
            :param hal_py.BooleanFunction p1: Boolean function start index.
            :param hal_py.BooleanFunction p2: Boolean function end index.
            :param int size: Bit-size of the resulting Boolean function slice, i.e., p2 - p1 + 1.
            :returns: The Boolean function slice on success, None otherwise.
            :rtype: hal_py.BooleanFunction or None
        )");

        py_boolean_function.def_static(
            "Concat",
            [](BooleanFunction p0, BooleanFunction p1, u16 size) -> std::optional<BooleanFunction> {
                auto res = BooleanFunction::Concat(std::move(p0), std::move(p1), size);
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
            py::arg("p0"),
            py::arg("p1"),
            py::arg("size"),
            R"(
            Concatenates two Boolean functions of potentially different bit-sizes `n` and `m` to form a single Boolean function of bit-size `n+m`.

            :param hal_py.BooleanFunction p0: First Boolean function (MSBs).
            :param hal_py.BooleanFunction p1: Second Boolean function (LSBs).
            :param int size: Bit-size of the concatenated Boolean function.
            :returns: The concatenated Boolean function on success, None otherwise.
            :rtype: hal_py.BooleanFunction or None
        )");

        py_boolean_function.def_static(
            "Zext",
            [](BooleanFunction p0, BooleanFunction p1, u16 size) -> std::optional<BooleanFunction> {
                auto res = BooleanFunction::Zext(std::move(p0), std::move(p1), size);
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
            py::arg("p0"),
            py::arg("p1"),
            py::arg("size"),
            R"(
            Zero-extends a Boolean function to the specified bit-size.

            :param hal_py.BooleanFunction p0: Boolean function to extend.
            :param hal_py.BooleanFunction p1: Constant Boolean function describing the size of the zero-extended result.
            :param int size: Bit-size of the zero-extended Boolean function.
            :returns: The zero-extended Boolean function on success, None otherwise.
            :rtype: hal_py.BooleanFunction or None
        )");

        py_boolean_function.def_static(
            "Sext",
            [](BooleanFunction p0, BooleanFunction p1, u16 size) -> std::optional<BooleanFunction> {
                auto res = BooleanFunction::Sext(std::move(p0), std::move(p1), size);
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
            py::arg("p0"),
            py::arg("p1"),
            py::arg("size"),
            R"(
            Sign-extends a Boolean function to the specified bit-size.

            :param hal_py.BooleanFunction p0: Boolean function to extend.
            :param hal_py.BooleanFunction p1: Constant Boolean function describing the size of the sign-extended result.
            :param int size: Bit-size of the sign-extended Boolean function.
            :returns: The sign-extended Boolean function on success, None otherwise.
            :rtype: hal_py.BooleanFunction or None
        )");

        py_boolean_function.def_static(
            "Eq",
            [](BooleanFunction p0, BooleanFunction p1, u16 size) -> std::optional<BooleanFunction> {
                auto res = BooleanFunction::Eq(std::move(p0), std::move(p1), size);
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
            py::arg("p0"),
            py::arg("p1"),
            py::arg("size"),
            R"(
            Joins two Boolean functions by an equality check that produces a single-bit result.

            :param hal_py.BooleanFunction p0: First Boolean function.
            :param hal_py.BooleanFunction p1: Second Boolean function.
            :param int size: Bit-size of the operation (always =1).
            :returns: The joined Boolean function on success, None otherwise.
            :rtype: hal_py.BooleanFunction or None
        )");

        py_boolean_function.def_static(
            "Sle",
            [](BooleanFunction p0, BooleanFunction p1, u16 size) -> std::optional<BooleanFunction> {
                auto res = BooleanFunction::Sle(std::move(p0), std::move(p1), size);
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
            py::arg("p0"),
            py::arg("p1"),
            py::arg("size"),
            R"(
            Joins two Boolean functions by a signed less-than-equal check that produces a single-bit result.

            :param hal_py.BooleanFunction p0: First Boolean function.
            :param hal_py.BooleanFunction p1: Second Boolean function.
            :param int size: Bit-size of the operation (always =1).
            :returns: The joined Boolean function on success, None otherwise.
            :rtype: hal_py.BooleanFunction or None
        )");

        py_boolean_function.def_static(
            "Slt",
            [](BooleanFunction p0, BooleanFunction p1, u16 size) -> std::optional<BooleanFunction> {
                auto res = BooleanFunction::Slt(std::move(p0), std::move(p1), size);
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
            py::arg("p0"),
            py::arg("p1"),
            py::arg("size"),
            R"(
            Joins two Boolean functions by a signed less-than check that produces a single-bit result.

            :param hal_py.BooleanFunction p0: First Boolean function.
            :param hal_py.BooleanFunction p1: Second Boolean function.
            :param int size: Bit-size of the operation (always =1).
            :returns: The joined Boolean function on success, None otherwise.
            :rtype: hal_py.BooleanFunction or None
        )");

        py_boolean_function.def_static(
            "Ule",
            [](BooleanFunction p0, BooleanFunction p1, u16 size) -> std::optional<BooleanFunction> {
                auto res = BooleanFunction::Ule(std::move(p0), std::move(p1), size);
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
            py::arg("p0"),
            py::arg("p1"),
            py::arg("size"),
            R"(
            Joins two Boolean functions by an unsigned less-than-equal check that produces a single-bit result.

            :param hal_py.BooleanFunction p0: First Boolean function.
            :param hal_py.BooleanFunction p1: Second Boolean function.
            :param int size: Bit-size of the operation (always =1).
            :returns: The joined Boolean function on success, None otherwise.
            :rtype: hal_py.BooleanFunction or None
        )");

        py_boolean_function.def_static(
            "Ult",
            [](BooleanFunction p0, BooleanFunction p1, u16 size) -> std::optional<BooleanFunction> {
                auto res = BooleanFunction::Ult(std::move(p0), std::move(p1), size);
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
            py::arg("p0"),
            py::arg("p1"),
            py::arg("size"),
            R"(
            Joins two Boolean functions by an unsigned less-than check that produces a single-bit result.

            :param hal_py.BooleanFunction p0: First Boolean function.
            :param hal_py.BooleanFunction p1: Second Boolean function.
            :param int size: Bit-size of the operation (always =1).
            :returns: The joined Boolean function on success, None otherwise.
            :rtype: hal_py.BooleanFunction or None
        )");

        py_boolean_function.def_static(
            "Ite",
            [](BooleanFunction p0, BooleanFunction p1, BooleanFunction p2, u16 size) -> std::optional<BooleanFunction> {
                auto res = BooleanFunction::Ite(std::move(p0), std::move(p1), std::move(p2), size);
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
            py::arg("p0"),
            py::arg("p1"),
            py::arg("p2"),
            py::arg("size"),
            R"(
            Joins three Boolean functions by an if-then-else operation with p0 as the condition, p1 as true-case, and p2 as false-case.
            Requires 'p1' to be of bit-size 1, both Boolean functions 'p1' and 'p2' to be of the specified bit-size, and produces a new Boolean function of the specified bit-size.

            :param hal_py.BooleanFunction p0: Boolean function condition.
            :param hal_py.BooleanFunction p1: Boolean function for true-case.
            :param hal_py.BooleanFunction p1: Boolean function for false-case.
            :param int size: Bit-size of the operation, i.e., size of p1 and p2.
            :returns: The joined Boolean function on success, None otherwise.
            :rtype: hal_py.BooleanFunction or None
        )");

        py_boolean_function.def(py::self & py::self, R"(
            Joins two Boolean functions by an 'AND' operation.
            Requires both Boolean functions to be of the same bit-size.

            WARNING: fails if the Boolean functions have different bit-sizes.

            :returns: The joined Boolean Bunction.
            :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def(py::self &= py::self, R"(
            Joins two Boolean functions by an 'AND' operation in-place.
            Requires both Boolean functions to be of the same bit-size.

            WARNING: fails if the Boolean functions have different bit-sizes.

            :returns: The joined Boolean Bunction.
            :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def(py::self | py::self, R"(
            Joins two Boolean functions by an 'OR' operation.
            Requires both Boolean functions to be of the same bit-size.

            WARNING: fails if the Boolean functions have different bit-sizes.

            :returns: The joined Boolean Bunction.
            :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def(py::self |= py::self, R"(
            Joins two Boolean functions by an 'OR' operation in-place.
            Requires both Boolean functions to be of the same bit-size.

            WARNING: fails if the Boolean functions have different bit-sizes.

            :returns: The joined Boolean Bunction.
            :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def(~py::self, R"(
            Negates the Boolean function.

            :returns: The negated Boolean function.
            :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def(py::self ^ py::self, R"(
            Joins two Boolean functions by an 'XOR' operation.
            Requires both Boolean functions to be of the same bit-size.

            WARNING: fails if the Boolean functions have different bit-sizes.

            :returns: The joined Boolean Bunction.
            :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def(py::self ^= py::self, R"(
            Joins two Boolean functions by an 'OR' operation in-place.
            Requires both Boolean functions to be of the same bit-size.

            WARNING: fails if the Boolean functions have different bit-sizes.

            :returns: The joined Boolean Bunction.
            :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def(py::self == py::self, R"(
            Checks whether two Boolean functions are equal.

            :returns: True if both Boolean functions are equal, False otherwise.
            :rtype: bool
        )");

        py_boolean_function.def(py::self != py::self, R"(
            Checks whether two Boolean functions are unequal.

            :returns: True if both Boolean functions are unequal, False otherwise.
            :rtype: bool
        )");

        py_boolean_function.def(py::self < py::self, R"(
            Checks whether this Boolean function is 'smaller' than the `other` Boolean function.

            :returns: True if this Boolean function is 'smaller', False otherwise.
            :rtype: bool
        )");

        py_boolean_function.def("is_empty", &BooleanFunction::is_empty, R"(
            Checks whether the Boolean function is empty.

            :returns: True if the Boolean function is empty, False otherwise.
            :rtype: bool
        )");

        py_boolean_function.def("clone", &BooleanFunction::clone, R"(
            Clones the Boolean function.

            :returns: The cloned Boolean function.
            :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def_property_readonly("size", &BooleanFunction::size, R"(
            The bit-size of the Boolean function.

            :type: int
        )");

        py_boolean_function.def("get_size", &BooleanFunction::size, R"(
            Returns the bit-size of the Boolean function.

            :returns: The bit-size of the Boolean function.
            :rtype: int
        )");

        py_boolean_function.def("is", &BooleanFunction::is, py::arg("type"), R"(
            Checks whether the top-level node of the Boolean function is of a specific type.

            :param int type: The type to check for.
            :returns: True if the node is of the given type, False otherwise.
            :rtype: bool
        )");

        py_boolean_function.def("is_variable", &BooleanFunction::is_variable, R"(
            Checks whether the top-level node of the Boolean function is of type `Variable` and holds a specific variable name.

            :returns: True if the top-level node of the Boolean function is of type Variable, False otherwise.
            :rtype: bool
        )");

        py_boolean_function.def("has_variable_name", &BooleanFunction::has_variable_name, R"(
            Checks whether the top-level node of the Boolean function is of type Variable and holds a specific variable name.

            :param int value: The variable name to check for.
            :returns: True if the top-level node of the Boolean function is of type Variable and holds the given variable name, False otherwise.
            :rtype: bool
        )");

        py_boolean_function.def(
            "get_variable_name",
            [](const BooleanFunction& self) -> std::optional<std::string> {
                auto res = self.get_variable_name();
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
            R"(
            Get the variable name of the top-level node of the Boolean function of type Variable.

            :returns: The variable name on success, None otherwise.
            :rtype: str or None
        )");

        py_boolean_function.def("is_constant", &BooleanFunction::is_constant, R"(
            Checks whether the top-level node of the Boolean function is of type Constant.

            :returns: True if the top-level node of the Boolean function is of type Constant, False otherwise.
            :rtype: bool
        )");

        py_boolean_function.def("has_constant_value", &BooleanFunction::has_constant_value, R"(
            Checks whether the top-level node of the Boolean function is of type Constant and holds a specific value.

            :param int value: The constant value to check for.
            :returns: True if the top-level node of the Boolean function is of type Constant and holds the given value, False otherwise.
            :rtype: bool
        )");

        py_boolean_function.def(
            "get_constant_value",
            [](const BooleanFunction& self) -> std::optional<u64> {
                auto res = self.get_constant_value();
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
            R"(
            Get the value of the top-level node of the Boolean function of type Constant as long as it has a size <= 64-bit.

            :returns: The constant value on success, None otherwise.
            :rtype: int or None
        )");

        py_boolean_function.def("is_index", &BooleanFunction::is_index, R"(
            Checks whether the top-level node of the Boolean function is of type Index. 

            :returns: True if the top-level node of the Boolean function is of type Index, False otherwise.
            :rtype: bool
        )");

        py_boolean_function.def("has_index_value", &BooleanFunction::has_index_value, R"(
            Checks whether the top-level node of the Boolean function is of type Index and holds a specific value.

            :param int value: The index value to check for.
            :returns: True if the top-level node of the Boolean function is of type Index and holds the given value, False otherwise.
            :rtype: bool
        )");

        py_boolean_function.def(
            "get_index_value",
            [](const BooleanFunction& self) -> std::optional<u16> {
                auto res = self.get_index_value();
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
            R"(
            Get the value of the top-level node of the Boolean function of type Index.

            :returns: The index value on success, None otherwise.
            :rtype: int or None
        )");

        py_boolean_function.def_property_readonly("top_level_node", &BooleanFunction::get_top_level_node, R"(
            The top-level node of the Boolean function.

            WARNING: fails if the Boolean function is empty.

            :type: hal_py.BooleanFunction.Node
        )");

        py_boolean_function.def("get_top_level_node", &BooleanFunction::get_top_level_node, R"(
            Returns the top-level node of the Boolean function.

            WARNING: fails if the Boolean function is empty.

            :returns: The top-level node.
            :rtype: hal_py.BooleanFunction.Node
        )");

        py_boolean_function.def_property_readonly("length", &BooleanFunction::length, R"(
            The number of nodes in the Boolean function.

            :type: int
        )");

        py_boolean_function.def("get_length", &BooleanFunction::length, R"(
            Returns the number of nodes in the Boolean function.

            :returns: The number of nodes.
            :rtype: int
        )");

        py_boolean_function.def("nodes", &BooleanFunction::get_nodes, R"(
            The reverse polish notation list of the Boolean function nodes.

            :type: list[hal_py.BooleanFunction.Node]
        )");

        py_boolean_function.def("get_nodes", &BooleanFunction::get_nodes, R"(
            Returns the reverse polish notation list of the Boolean function nodes.

            :returns: A list of nodes.
            :rtype: list[hal_py.BooleanFunction.Node]
        )");

        py_boolean_function.def_property_readonly("parameters", &BooleanFunction::get_parameters, R"(
            The parameter list of the top-level node of the Boolean function.

            :type: list[hal_py.BooleanFunction]
        )");

        py_boolean_function.def("get_parameters", &BooleanFunction::get_parameters, R"(
            Returns the parameter list of the top-level node of the Boolean function.

            :returns: A vector of Boolean functions.
            :rtype: list[hal_py.BooleanFunction]
        )");

        py_boolean_function.def_property_readonly("variable_names", &BooleanFunction::get_variable_names, R"(
            The set of variable names used by the Boolean function.

            :type: set[str]
        )");

        py_boolean_function.def("get_variable_names", &BooleanFunction::get_variable_names, R"(
            Returns the set of variable names used by the Boolean function.

            :returns: A set of variable names.
            :rtype: set[str]
        )");

        py_boolean_function.def(
            "__str__", [](const BooleanFunction& f) { return f.to_string(); }, R"(
            Translates the Boolean function into its string representation.

            :returns: The Boolean function as a string.
            :rtype: str
        )");

        py_boolean_function.def_static(
            "from_string",
            [](const std::string& expression) -> BooleanFunction {
                auto res = BooleanFunction::from_string(expression);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return BooleanFunction();
                }
            },
            py::arg("expression"),
            R"(
            Parses a Boolean function from a string expression.

            :param str expression: Boolean function string.
            :returns: The Boolean function on success, an empty Boolean function otherwise.
            :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def("simplify", &BooleanFunction::simplify, R"(
            Simplifies the Boolean function.

            :returns: The simplified Boolean function.
            :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def(
            "substitute", py::overload_cast<const std::string&, const std::string&>(&BooleanFunction::substitute, py::const_), py::arg("old_variable_name"), py::arg("new_variable_name"), R"(
            Substitute a variable name with another one, i.e., renames the variable.
            The operation is applied to all instances of the variable in the function.

            :param str old_variable_name: The old variable name to substitute.
            :param str new_variable_name: The new variable name.
            :returns: The resulting Boolean function.
            :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def(
            "substitute",
            [](const BooleanFunction& self, const std::string& variable_name, const BooleanFunction& function) -> std::optional<BooleanFunction> {
                auto res = self.substitute(variable_name, function);
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
            py::arg("variable_name"),
            py::arg("function"),
            R"(
            Substitute a variable with another Boolean function.
            The operation is applied to all instances of the variable in the function.

            :param str variable_name: The variable to substitute.
            :param hal_py.BooleanFunction function: The function replace the variable with.
            :returns: The resulting Boolean function on success, None otherwise.
            :rtype: hal_py.BooleanFunction or None
        )");

        py_boolean_function.def(
            "evaluate",
            [](const BooleanFunction& self, const std::unordered_map<std::string, BooleanFunction::Value>& inputs) -> std::optional<BooleanFunction::Value> {
                auto res = self.evaluate(inputs);
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
            py::arg("inputs"),
            R"(
            Evaluates a Boolean function comprising only single-bit variables using the given input values.

            :param dict[str,hal_pyBooleanFunction.Value] inputs: A dict from variable name to input value.
            :returns: The resulting value on success, None otherwise.
            :rtype: hal_py.BooleanFunction.Value or None
        )");

        py_boolean_function.def(
            "evaluate",
            [](const BooleanFunction& self, const std::unordered_map<std::string, std::vector<BooleanFunction::Value>>& inputs) -> std::optional<std::vector<BooleanFunction::Value>> {
                auto res = self.evaluate(inputs);
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
            py::arg("inputs"),
            R"(
            Evaluates a Boolean function comprising multi-bit variables using the given input values.

            :param dict[str,list[hal_pyBooleanFunction.Value]] inputs:  A dict from variable name to a list of input values.
            :returns: A vector of values on success, None otherwise.
            :rtype: list[hal_py.BooleanFunction.Value] or None
        )");

        py_boolean_function.def(
            "compute_truth_table",
            [](const BooleanFunction& self, const std::vector<std::string>& ordered_variables, bool remove_unknown_variables) -> std::optional<std::vector<std::vector<BooleanFunction::Value>>> {
                auto res = self.compute_truth_table(ordered_variables, remove_unknown_variables);
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
            py::arg("ordered_variables")        = std::vector<std::string>(),
            py::arg("remove_unknown_variables") = false,
            R"(
            Computes the truth table outputs for a Boolean function that comprises <= 10 single-bit variables.

            WARNING: The generation of the truth table is exponential in the number of parameters.

            :param list[str] ordered_variables: A list describing the order of input variables used to generate the truth table. Defaults to an empty list.
            :param bool remove_unknown_variables: Set True to remove variables from the truth table that are not present within the Boolean function, False otherwise. Defaults to False.
            :returns: A list of values representing the truth table output on success, None otherwise.
            :rtype: list[list[hal_py.BooleanFunction.Value]] or None
        )");

        py::class_<BooleanFunction::Node> py_boolean_function_node(py_boolean_function, "Node", R"(
            Node refers to an abstract syntax tree node of a Boolean function. A node is an abstract base class for either an operation (e.g., AND, XOR) or an operand (e.g., a signal name variable).
        )");

        py_boolean_function_node.def_readwrite("type", &BooleanFunction::Node::type, R"(
            The type of the node.

            :type: int
        )");

        py_boolean_function_node.def_readwrite("size", &BooleanFunction::Node::size, R"(
            The bit-size of the node.

            :type: int
        )");

        py_boolean_function_node.def_readwrite("constant", &BooleanFunction::Node::constant, R"(
            The (optional) constant value of the node.

            :type: list[hal_py.BooleanFunction.Value]
        )");

        py_boolean_function_node.def_readwrite("index", &BooleanFunction::Node::index, R"(
            The (optional) index value of the node.

            :type: int
        )");

        py_boolean_function_node.def_readwrite("variable", &BooleanFunction::Node::variable, R"(
            The (optional) variable name of the node.

            :type: str
        )");

        py_boolean_function_node.def_static("Operation", BooleanFunction::Node::Operation, py::arg("type"), py::arg("size"), R"(
            Constructs an 'operation' node.

            :param int type: The type of the operation.
            :param int size: The bit-size of the operation.
            :returns: The node.
            :rtype: hal_py.BooleanFunction.Node
        )");

        py_boolean_function_node.def_static("Constant", BooleanFunction::Node::Constant, py::arg("value"), R"(
            Constructs a 'constant' node.

            :param list[hal_py.BooleanFunction.Value] value: The constant value of the node.
            :returns: The node.
            :rtype: hal_py.BooleanFunction.Node
        )");

        py_boolean_function_node.def_static("Index", BooleanFunction::Node::Index, py::arg("index"), py::arg("size"), R"(
            Constructs an 'index' node.

            :param int value: The index value of the node.
            :param int size: The bit-size of the node.
            :returns: The node.
            :rtype: hal_py.BooleanFunction.Node
        )");

        py_boolean_function_node.def_static("Variable", BooleanFunction::Node::Variable, py::arg("variable"), py::arg("size"), R"(
            Constructs a 'variable' node.

            :param int value: The variable name of the node.
            :param int size: The bit-size of the node.
            :returns: The node.
            :rtype: hal_py.BooleanFunction.Node
        )");

        py_boolean_function_node.def(py::self == py::self, R"(
            Checks whether two Boolean function nodes are equal.

            :returns: True if both Boolean function nodes are equal, False otherwise.
            :rtype: bool
        )");

        py_boolean_function_node.def(py::self != py::self, R"(
            Checks whether two Boolean function nodes are unequal.

            :returns: True if both Boolean function nodes are unequal, False otherwise.
            :rtype: bool
        )");

        py_boolean_function_node.def(py::self < py::self, R"(
            Checks whether this Boolean function node is 'smaller' than the `other` Boolean function node.

            :returns: True if this Boolean function node is 'smaller', False otherwise.
            :rtype: bool
        )");

        py_boolean_function_node.def("clone", &BooleanFunction::Node::clone, R"(
            Clones the Boolean function node.

            :returns: The cloned Boolean function node.
            :rtype: hal_py.BooleanFunction.Node
        )");

        py_boolean_function_node.def(
            "__str__", [](const BooleanFunction::Node& n) { return n.to_string(); }, R"(
            Translates the Boolean function node into its string representation.

            :returns: The Boolean function node as a string.
            :rtype: str
        )");

        py_boolean_function_node.def_property_readonly("arity", &BooleanFunction::Node::get_arity, R"(
            The arity of the Boolean function node, i.e., the number of parameters.

            :type: int
        )");

        py_boolean_function_node.def("get_arity", &BooleanFunction::Node::get_arity, R"(
            Returns the arity of the Boolean function node, i.e., the number of parameters.

            :returns: The arity.
            :rtype: int
        )");

        py_boolean_function_node.def_static("get_arity_of_type", &BooleanFunction::Node::get_arity_of_type, py::arg("type"), R"(
            Returns the arity for a Boolean function node of the given type, i.e., the number of parameters.

            :returns: The arity.
            :rtype: int
        )");

        py_boolean_function_node.def("is", &BooleanFunction::Node::is, py::arg("type"), R"(
            Checks whether the Boolean function node is of a specific type.

            :param int type: The type to check for.
            :returns: True if the node is of the given type, False otherwise.
            :rtype: bool
        )");

        py_boolean_function_node.def("is_constant", &BooleanFunction::Node::is_constant, R"(
            Checks whether the Boolean function node is of type Constant.

            :returns: True if the Boolean function node is of type Constant, False otherwise.
            :rtype: bool
        )");

        py_boolean_function_node.def("has_constant_value", &BooleanFunction::Node::has_constant_value, py::arg("value"), R"(
            Checks whether the Boolean function node is of type Constant and holds a specific value.

            :param int value: The value to check for.
            :returns: True if the Boolean function node is of type Constant and holds the given value, False otherwise.
            :rtype: bool
        )");

        py_boolean_function_node.def("is_index", &BooleanFunction::Node::is_index, R"(
            Checks whether the Boolean function node is of type Index.

            :returns: True if the Boolean function node is of type Index, False otherwise.
            :rtype: bool
        )");

        py_boolean_function_node.def("has_index_value", &BooleanFunction::Node::has_index_value, py::arg("value"), R"(
            Checks whether the Boolean function node is of type Index and holds a specific value.

            :param int value: The value to check for.
            :returns: True if the Boolean function node is of type Index and holds the given value, False otherwise.
            :rtype: bool
        )");

        py_boolean_function_node.def("is_variable", &BooleanFunction::Node::is_variable, R"(
            Checks whether the Boolean function node is of type Variable.

            :returns: True if the Boolean function node is of type Variable, False otherwise.
            :rtype: bool
        )");

        py_boolean_function_node.def("has_variable_name", &BooleanFunction::Node::has_variable_name, py::arg("variable_name"), R"(
            Checks whether the Boolean function node is of type Variable and holds a specific variable name.

            :param str variable_name: The variable name to check for.
            :returns: True if the Boolean function node is of type Variable and holds the given variable name, False otherwise.
            :rtype: bool
        )");

        py_boolean_function_node.def("is_operation", &BooleanFunction::Node::is_operation, R"(
            Checks whether the Boolean function node is an operation node.

            :returns: True if the Boolean function node is an operation node, False otherwise.
            :rtype: bool
        )");

        py_boolean_function_node.def("is_operand", &BooleanFunction::Node::is_operand, R"(
            Checks whether the Boolean function node is an operand node.

            :returns: True if the Boolean function node is an operand node, False otherwise.
            :rtype: bool
        )");

        py_boolean_function_node.def("is_commutative", &BooleanFunction::Node::is_commutative, R"(
            Checks whether the Boolean function node is commutative.

            :returns: True if the Boolean function node is commutative, False otherwise.
            :rtype: bool
        )");

        py::class_<BooleanFunction::NodeType> py_boolean_function_node_type(py_boolean_function, "NodeType", R"(
            Holds all node types available in a Boolean function.
        )");

        py_boolean_function_node_type.def_readonly_static("And", &BooleanFunction::NodeType::And);
        py_boolean_function_node_type.def_readonly_static("Or", &BooleanFunction::NodeType::Or);
        py_boolean_function_node_type.def_readonly_static("Not", &BooleanFunction::NodeType::Not);
        py_boolean_function_node_type.def_readonly_static("Xor", &BooleanFunction::NodeType::Xor);

        py_boolean_function_node_type.def_readonly_static("Add", &BooleanFunction::NodeType::Add);
        py_boolean_function_node_type.def_readonly_static("Sub", &BooleanFunction::NodeType::Sub);
        py_boolean_function_node_type.def_readonly_static("Mul", &BooleanFunction::NodeType::Mul);
        py_boolean_function_node_type.def_readonly_static("Sdiv", &BooleanFunction::NodeType::Sdiv);
        py_boolean_function_node_type.def_readonly_static("Udiv", &BooleanFunction::NodeType::Udiv);
        py_boolean_function_node_type.def_readonly_static("Srem", &BooleanFunction::NodeType::Srem);
        py_boolean_function_node_type.def_readonly_static("Urem", &BooleanFunction::NodeType::Urem);

        py_boolean_function_node_type.def_readonly_static("Concat", &BooleanFunction::NodeType::Concat);
        py_boolean_function_node_type.def_readonly_static("Slice", &BooleanFunction::NodeType::Slice);
        py_boolean_function_node_type.def_readonly_static("Zext", &BooleanFunction::NodeType::Zext);
        py_boolean_function_node_type.def_readonly_static("Sext", &BooleanFunction::NodeType::Sext);

        py_boolean_function_node_type.def_readonly_static("Eq", &BooleanFunction::NodeType::Eq);
        py_boolean_function_node_type.def_readonly_static("Sle", &BooleanFunction::NodeType::Sle);
        py_boolean_function_node_type.def_readonly_static("Slt", &BooleanFunction::NodeType::Slt);
        py_boolean_function_node_type.def_readonly_static("Ule", &BooleanFunction::NodeType::Ule);
        py_boolean_function_node_type.def_readonly_static("Ult", &BooleanFunction::NodeType::Ult);
        py_boolean_function_node_type.def_readonly_static("Ite", &BooleanFunction::NodeType::Ite);

        py_boolean_function_node_type.def_readonly_static("Constant", &BooleanFunction::NodeType::Constant);
        py_boolean_function_node_type.def_readonly_static("Index", &BooleanFunction::NodeType::Index);
        py_boolean_function_node_type.def_readonly_static("Variable", &BooleanFunction::NodeType::Variable);
    }
}    // namespace hal
