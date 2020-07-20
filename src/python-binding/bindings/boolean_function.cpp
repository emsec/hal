#include "bindings.h"

namespace hal
{
    void boolean_function_init(py::module& m)
    {
        py::class_<BooleanFunction> py_boolean_function(m, "BooleanFunction", R"(Boolean function class.)");

        py::enum_<BooleanFunction::value>(py_boolean_function, "value", R"(
        Represents the logic value that a boolean function operates on. Available are: X, ZERO, and ONE.)")
            .value("X", BooleanFunction::value::X)
            .value("ZERO", BooleanFunction::value::ZERO)
            .value("ONE", BooleanFunction::value::ONE)
            .export_values();

        py_boolean_function.def(py::init<>(), R"(
        Constructor for an empty function. Evaluates to X (undefined). Combining a function with an empty function leaves the other one unchanged.
        )");

        py_boolean_function.def(py::init<const std::string&>(), py::arg("variable"), R"(
        Constructor for a variable, usable in other functions. Variable name must not be empty.

        :param str variable_name: Name of the variable.
        )");

        py_boolean_function.def(py::init<BooleanFunction::value>(), R"(
        Constructor for a constant, usable in other functions.
        The constant can be either X, Zero, or ONE.

        :param hal_py.value constant: A constant value.
        )");

        py_boolean_function.def(
            "substitute", py::overload_cast<const std::string&, const std::string&>(&BooleanFunction::substitute, py::const_), py::arg("old_variable_name"), py::arg("new_variable_name"), R"(
        Substitutes a variable with another variable (i.e., variable renaming).
        Applies to all instances of the variable in the function.
        This is just a shorthand for the generic substitute function.

        :param str old_variable_name:  The old variable to substitute
        :param str new_variable_name:  The new variable name
        :returns: The new boolean function.
        :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def(
            "substitute", py::overload_cast<const std::string&, const BooleanFunction&>(&BooleanFunction::substitute, py::const_), py::arg("variable_name"), py::arg("function"), R"(
        Substitutes a variable with another function (can again be a single variable).
        Applies to all instances of the variable in the function.

        :param str variable_name:  The variable to substitute
        :param hal_py.BooleanFunction function:  The function to take the place of the varible
        :returns: The new boolean function.
        :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def("evaluate", &BooleanFunction::evaluate, py::arg("inputs") = std::map<std::string, BooleanFunction::value>(), R"(
        Evaluates the function on the given inputs and returns the result.

        :param dict[str,value] inputs:  A map from variable names to values.
        :returns: The value that the function evaluates to.
        :rtype: hal_py.value
        )");

        py_boolean_function.def("__call__", [](const BooleanFunction& f, const std::map<std::string, BooleanFunction::value>& values) { return f(values); });

        py_boolean_function.def("is_constant_one", &BooleanFunction::is_constant_one, R"(
        Checks whether the function constantly outputs ONE.

        :returns: True if function is constant ONE, false otherwise.
        :rtype: bool
        )");

        py_boolean_function.def("is_constant_zero", &BooleanFunction::is_constant_zero, R"(
        Checks whether the function constantly outputs ZERO.

        :returns: True if function is constant ZERO, false otherwise.
        :rtype: bool
        )");

        py_boolean_function.def("is_empty", &BooleanFunction::is_empty, R"(
        Checks whether the function is empty.

        :returns: True if function is empty, false otherwise.
        :rtype: bool
        )");

        py_boolean_function.def_property_readonly("variables", &BooleanFunction::get_variables, R"(
        A set of all variable names used in this boolean function.

        :type: set[str]
        )");

        py_boolean_function.def("get_variables", &BooleanFunction::get_variables, R"(
        Get all variable names used in this boolean function.

        :returns: A set of all variable names.
        :rtype: set[str]
        )");

        py_boolean_function.def_static("from_string", &BooleanFunction::from_string, py::arg("expression"), py::arg("variable_names"), R"(
        Parse a function from a string representation.
        Supported operators are  NOT (\"!\", \"'\"), AND (\"&\", \"*\", \" \"), OR (\"|\", \"+\"), XOR (\"^\") and brackets (\"(\", \")\").
        Operator precedence is ! > & > ^ > |

        Since, for example, '(' is interpreted as a new term, but might also be an intended part of a variable,
        a vector of known variable names can be supplied, which are extracted before parsing.
        If there is an error during bracket matching, X is returned for that part.

        :param str expression: String containing a boolean function.
        :param str variable_names: Names of variables to help resolve problematic functions
        :returns: The boolean function extracted from the string.
        :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def("__str__", [](const BooleanFunction& f) { return f.to_string(); });

        py_boolean_function.def(py::self & py::self, R"(
        Combines two boolean functions using an AND operator.

        :returns: The combined boolean function.
        :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def(py::self | py::self, R"(
        Combines two boolean functions using an OR operator.

        :returns: The combined boolean function.
        :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def(py::self ^ py::self, R"(
        Combines two boolean functions using an XOR operator.

        :returns: The combined boolean function.
        :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def(py::self &= py::self, R"(
        Combines two boolean functions using an AND operator.

        :returns: The combined boolean function.
        :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def(py::self |= py::self, R"(
        Combines two boolean functions using an OR operator.

        :returns: The combined boolean function.
        :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def(py::self ^= py::self, R"(
        Combines two boolean functions using an XOR operator.

        :returns: The combined boolean function.
        :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def(!py::self, R"(
        Negates the boolean function.

        :returns: The negated boolean function.
        :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def(py::self == py::self, R"(
        Tests whether two boolean functions are equal.

        :returns: True when both boolean functions are equal, false otherwise.
        :rtype: bool
        )");

        py_boolean_function.def(py::self != py::self, R"(
        Tests whether two boolean functions are unequal.

        :returns: True when both boolean functions are unequal, false otherwise.
        :rtype: bool
        )");

        py_boolean_function.def("is_dnf", &BooleanFunction::is_dnf, R"(
        Tests whether the function is in DNF.

        :returns: True if in DNF, false otherwise.
        :rtype: bool
        )");

        py_boolean_function.def("to_dnf", &BooleanFunction::to_dnf, R"(
        Gets the plain DNF representation of the function.

        :returns: The DNF as a boolean function.
        :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def("optimize", &BooleanFunction::optimize, R"(
        Optimizes the function by first converting it to DNF and then applying the Quine-McCluskey algorithm.

        :returns: The optimized boolean function.
        :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def("get_truth_table", &BooleanFunction::get_truth_table, py::arg("ordered_variables") = std::vector<std::string>(), py::arg("remove_unknown_variables") = false, R"(
        Get the truth table outputs of the function.
        WARNING: Exponential runtime in the number of variables!

        Output is the vector of output values when walking the truth table in ascending order.

        If ordered_variables is empty, all included variables are used and ordered alphabetically.

        :param list[str] ordered_variables: Specific order in which the inputs shall be structured in the truth table.
        :param bool remove_unknown_variables: If true, all given variables that are not found in the function are removed from the truth table.
        :returns: The vector of output values.
        :rtype: list[value]
        )");
    }
}    // namespace hal
