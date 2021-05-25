#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void boolean_function_init(py::module& m)
    {
        py::class_<BooleanFunction> py_boolean_function(m, "BooleanFunction", R"(Boolean function class.)");

        py::enum_<BooleanFunction::Value>(py_boolean_function, "Value", R"(Represents the logic value that a boolean function operates on.)")
            .value("X", BooleanFunction::X, R"(Represents an undefined value.)")
            .value("ZERO", BooleanFunction::ZERO, R"(Represents a logical 0.)")
            .value("ONE", BooleanFunction::ONE, R"(Represents a logical 1.)")
            .export_values();

        py_boolean_function.def(py::init<>(), R"(
            Construct an empty Boolean function and thus evaluates to ``X`` (undefined).
        )");

        py_boolean_function.def(py::init<const std::string&>(), py::arg("variable_name"), R"(
            Construct a Boolean function comprising a single variable.
            The name of the variable must not be empty.

            :param str variable_name: The name of the variable.
        )");

        py_boolean_function.def(py::init<BooleanFunction::Value>(), py::arg("constant"), R"(
            Construct a Boolean function from a single constant value.

            :param hal_py.BooleanFunction.Value constant: The constant value.
        )");

        py_boolean_function.def(
            "substitute", py::overload_cast<const std::string&, const std::string&>(&BooleanFunction::substitute, py::const_), py::arg("old_variable_name"), py::arg("new_variable_name"), R"(
            Substitute a variable with another one and thus renames the variable.
            The operation is applied to all instances of the variable in the function.

            :param str old_variable_name: The old variable to substitute.
            :param str new_variable_name: The new variable.
            :returns: The resulting Boolean function.
            :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def(
            "substitute", py::overload_cast<const std::string&, const BooleanFunction&>(&BooleanFunction::substitute, py::const_), py::arg("variable_name"), py::arg("function"), R"(
            Substitute a variable with another function.
            The operation is applied to all instances of the variable in the function.

            :param str variable_name: The variable to substitute.
            :param hal_py.BooleanFunction function: The function replace the variable with.
            :returns: The resulting Boolean function.
            :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def("evaluate", &BooleanFunction::evaluate, py::arg("inputs") = std::unordered_map<std::string, BooleanFunction::Value>(), R"(
            Evaluate the Boolean function on the given inputs and returns the result.

            :param dict[str,hal_py.BooleanFunction.Value] inputs: A dict from variable names to values.
            :returns: The value that the function evaluates to.
            :rtype: hal_py.BooleanFunction.Value
        )");

        py_boolean_function.def(
            "__call__", [](const BooleanFunction& f, const std::unordered_map<std::string, BooleanFunction::Value>& values) { return f(values); }, R"(
            Evaluate the function on the given inputs and returns the result.

            :param dict[str,hal_py.BooleanFunction.Value] inputs: A dict from variable names to values.
            :returns: The value that the function evaluates to.
            :rtype: hal_py.BooleanFunction.Value
        )");

        py_boolean_function.def("is_constant_one", &BooleanFunction::is_constant_one, R"(
            Check whether the Boolean function always evaluates to ``ONE``.

            :returns: True if function is constant ``ONE``, false otherwise.
            :rtype: bool
        )");

        py_boolean_function.def("is_constant_zero", &BooleanFunction::is_constant_zero, R"(
            Check whether the Boolean function always evaluates to ``ZERO``.

            :returns: True if function is constant ``ZERO``, false otherwise.
            :rtype: bool
        )");

        py_boolean_function.def("is_empty", &BooleanFunction::is_empty, R"(
            Check whether the function is empty.

            :returns: True if function is empty, false otherwise.
            :rtype: bool
        )");

        py_boolean_function.def_property_readonly("variables", &BooleanFunction::get_variables, R"(
            A list of all variable names utilized in this Boolean function.

            :type: list[str]
        )");

        py_boolean_function.def("get_variables", &BooleanFunction::get_variables, R"(
            Get all variable names utilized in this Boolean function.

            :returns: A list of all variable names.
            :rtype: list[str]
        )");

        py_boolean_function.def_static("from_string", &BooleanFunction::from_string, py::arg("expression"), py::arg("variable_names") = std::vector<std::string>(), R"(
            Parse a function from a string representation.
            Supported operators are  NOT (``!``, ``'``), AND (``&``, ``*``, ``␣``), OR (``|``, ``+``), XOR (``^``) and brackets (``(``, ``)``).
            Operator precedence is ``!`` > ``&`` > ``^`` > ``|``.

            Since, for example, ``(`` is interpreted as a new term, but might also be an intended part of a variable, a vector of known variable names can be supplied, which are extracted before parsing.

            If there is an error during bracket matching, ``X`` is returned for that part.

            :param str expression: String containing a Boolean function.
            :param list[str] variable_names: List of variable names.
            :returns: The Boolean function extracted from the string.
            :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def(
            "__str__", [](const BooleanFunction& f) { return f.to_string(); }, R"(
            Get the boolean function as a string.

            :returns: A string describing the Boolean function.
            :rtype: str
        )");

        py_boolean_function.def(py::self & py::self, R"(
            Combine two Boolean functions using an AND operator.

            :returns: The combined Boolean Bunction.
            :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def(py::self | py::self, R"(
            Combine two Boolean functions using an OR operator.

            :returns: The combined Boolean function.
            :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def(py::self ^ py::self, R"(
            Combine two Boolean functions using an XOR operator.

            :returns: The combined Boolean function.
            :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def(py::self &= py::self, R"(
            Combine two Boolean functions using an AND operator in-place.

            :returns: The combined Boolean function.
            :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def(py::self |= py::self, R"(
            Combine two Boolean functions using an OR operator in-place.

            :returns: The combined Boolean function.
            :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def(py::self ^= py::self, R"(
            Combine two Boolean functions using an XOR operator in-place.

            :returns: The combined Boolean function.
            :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def(~py::self, R"(
            Negate the Boolean function.

            :returns: The negated Boolean function.
            :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def(py::self == py::self, R"(
            Check whether two Boolean functions are equal.

            :returns: True if both Boolean functions are equal, false otherwise.
            :rtype: bool
        )");

        py_boolean_function.def(py::self != py::self, R"(
            Check whether two Boolean functions are unequal.

            :returns: True if both Boolean functions are unequal, false otherwise.
            :rtype: bool
        )");

        py_boolean_function.def("is_dnf", &BooleanFunction::is_dnf, R"(
            Check whether the Boolean function is in disjunctive normal form (DNF).

            :returns: True if in DNF, false otherwise.
            :rtype: bool
        )");

        py_boolean_function.def("to_dnf", &BooleanFunction::to_dnf, R"(
            Get the plain disjunctive normal form (DNF) representation of the Boolean function.

            :returns: The DNF as a Boolean function.
            :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def("get_dnf_clauses", &BooleanFunction::get_dnf_clauses, R"(
            Get the disjunctive normal form (DNF) clauses of the function.
            
            Each clause is a vector of pairs (variable name, Boolean value).
        
            Returns an empty vector if the Boolean function is empty.
        
            :returns: The DNF clauses as a list of lists of pairs (string, bool).
            :rtype: list[list[tuple(str,bool)]]
        )");

        py_boolean_function.def("optimize", &BooleanFunction::optimize, R"(
            Optimize the Boolean function by first converting it to disjunctive normal form (DNF) and then applying the Quine-McCluskey algorithm.

            :returns: The optimized Boolean function.
            :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def("optimize_constants", &BooleanFunction::optimize_constants, R"(
            Removes constant values whenever possible.

            :returns: The optimized Boolean function.
            :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def("get_truth_table", &BooleanFunction::get_truth_table, py::arg("ordered_variables") = std::vector<std::string>(), py::arg("remove_unknown_variables") = false, R"(
            Get the truth table outputs of the Boolean function.

            WARNING: Exponential runtime in the number of variables!

            Output is the vector of output values when walking the truth table from the least significant bit to the most significant one.

            If ordered_variables is empty, all included variables are used and ordered alphabetically.

            :param list[str] ordered_variables: Variables in the order of the inputs.
            :param bool remove_unknown_variables: If true, all given variables that are not found in the function are removed from the truth table.
            :returns: The list of output values.
            :rtype: list[hal_py.BooleanFunction.Value]
        )");
    }
}    // namespace hal
