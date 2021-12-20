#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void boolean_function_init(py::module& m)
    {
        py::class_<BooleanFunction> py_boolean_function(
            m,
            "BooleanFunction",
            R"(A BooleanFunction represents a symbolic expression (e.g., "A & B") in order to abstract the (semantic) functionality of a single netlist gate (or even a complex subcircuit comprising multiple gates) in a formal manner. To this end, the BooleanFunction class is able to construct and display arbitrarily-nested expressions, enable symbolic simplification (e.g., simplify "A & 0" to "0"), and translate Boolean functions to the SAT / SMT solver domain to use the solve constraint formulas.)");

        py::enum_<BooleanFunction::Value> py_boolean_function_value(py_boolean_function, "Value", R"(Represents the logic value that a Boolean function operates on.)");
        py_boolean_function_value.value("ZERO", BooleanFunction::ZERO, R"(Represents a logical 0.)")
            .value("ONE", BooleanFunction::ONE, R"(Represents a logical 1.)")
            .value("Z", BooleanFunction::X, R"(Represents a high-impedance value.)")
            .value("X", BooleanFunction::X, R"(Represents an undefined value.)")
            .export_values();

        py_boolean_function_value.def(
            "__str__", [](const BooleanFunction::Value& v) { return BooleanFunction::to_string(v); }, R"(
            Translates the Boolean function value into its string representation.

            :returns: The value as a string.
            :rtype: str
        )");

        py_boolean_function.def(py::init<>(), R"(
            Constructs an empty / invalid Boolean function.
        )");

        py_boolean_function.def_static("build", &BooleanFunction::build, py::arg("nodes"), R"(
            Builds and validates a Boolean function from a list of nodes.
        
            :param list[hal_py.BooleanFunction.Node] nodes: List of Boolean function nodes.
            :returns: The Boolean function on success, a string error message otherwise.
            :rtype: hal_py.BooleanFunction or str
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

        py_boolean_function.def_static("Const", py::overload_cast<const std::vector<BooleanFunction::Value>&>(&BooleanFunction::Const), py::arg("values"), R"(
            Creates a constant multi-bit Boolean function from a vector of values.
        
            :param list[hal_py.BooleanFunction.Value] values: The list of values.
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
            Creates a Boolean function index of the given bit-size from an integer value.
        
            :param int index: The integer value.
            :param int size: The bit-size.
            :returns: The Boolean function.
            :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function.def_static(
            "And", [](BooleanFunction p0, BooleanFunction p1, u16 size) { return BooleanFunction::And(std::move(p0), std::move(p1), size); }, py::arg("p0"), py::arg("p1"), py::arg("size"), R"(
            Joins two Boolean functions by an 'AND' operation. 
            Requires both Boolean functions to be of the specified bit-size.

            :param hal_py.BooleanFunction p0: First Boolean function.
            :param hal_py.BooleanFunction p1: Second Boolean function.
            :param int size: Bit-size of the operation.
            :returns: The joined Boolean function on success, a string error message otherwise.
            :rtype: hal_py.BooleanFunction or str
        )");

        py_boolean_function.def_static(
            "Or", [](BooleanFunction p0, BooleanFunction p1, u16 size) { return BooleanFunction::Or(std::move(p0), std::move(p1), size); }, py::arg("p0"), py::arg("p1"), py::arg("size"), R"(
            Joins two Boolean functions by an 'OR' operation.
            Requires both Boolean functions to be of the specified bit-size.

            :param hal_py.BooleanFunction p0: First Boolean function.
            :param hal_py.BooleanFunction p1: Second Boolean function.
            :param int size: Bit-size of the operation.
            :returns: The joined Boolean function on success, a string error message otherwise.
            :rtype: hal_py.BooleanFunction or str
        )");

        py_boolean_function.def_static(
            "Not", [](BooleanFunction p0, u16 size) { return BooleanFunction::Not(std::move(p0), size); }, py::arg("p0"), py::arg("size"), R"(
            Negates the given Boolean function.
            Requires the Boolean function to be of the specified bit-size.

            :param hal_py.BooleanFunction p0: The Boolean function to negate.
            :param int size: Bit-size of the operation.
            :returns: The negated Boolean function on success, a string error message otherwise.
            :rtype: hal_py.BooleanFunction or str
        )");

        py_boolean_function.def_static(
            "Xor", [](BooleanFunction p0, BooleanFunction p1, u16 size) { return BooleanFunction::Xor(std::move(p0), std::move(p1), size); }, py::arg("p0"), py::arg("p1"), py::arg("size"), R"(
            Joins two Boolean functions by an 'XOR' operation.
            Requires both Boolean functions to be of the specified bit-size.

            :param hal_py.BooleanFunction p0: First Boolean function.
            :param hal_py.BooleanFunction p1: Second Boolean function.
            :param int size: Bit-size of the operation.
            :returns: The joined Boolean function on success, a string error message otherwise.
            :rtype: hal_py.BooleanFunction or str
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

        py_boolean_function.def(py::self != py::self, R"(
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

        py_boolean_function.def("size", &BooleanFunction::size, R"(
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
            Checks whether the Boolean function is a variable.

            :returns: True if the Boolean function is a variable, False otherwise.
            :rtype: bool
        )");

        py_boolean_function.def("is_constant", py::overload_cast<>(&BooleanFunction::is_constant, py::const_), R"(
            Checks whether the Boolean function is a constant.

            :returns: True if the Boolean function is a constant, False otherwise.
            :rtype: bool
        )");

        py_boolean_function.def("is_constant", py::overload_cast<u64>(&BooleanFunction::is_constant, py::const_), R"(
            Checks whether the Boolean function is a constant with a specific value.

            :param int value: The value to check for.
            :returns: True if the Boolean function is a constant with the given value, False otherwise.
            :rtype: bool
        )");

        py_boolean_function.def("get_top_level_node", &BooleanFunction::get_top_level_node, R"(
            Returns the top-level node of the Boolean function.

            WARNING: fails if the Boolean function is empty.

            :returns: The top-level node.
            :rtype: hal_py.BooleanFunction.Node
        )");

        py_boolean_function.def("length", &BooleanFunction::length, R"(
            Returns the number of nodes in the Boolean function.

            :returns: The number of nodes. 
            :rtype: int
        )");

        py_boolean_function.def("get_nodes", &BooleanFunction::get_nodes, R"(
            Returns the reverse polish notation list of the Boolean function nodes.

            :returns: A list of nodes.
            :rtype: list[hal_py.BooleanFunction.Node]
        )");

        py_boolean_function.def("get_parameters", &BooleanFunction::get_parameters, R"(
            Returns the parameter list of the top-level node of the Boolean function.

            :returns: A vector of Boolean functions.
            :rtype: list[hal_py.BooleanFunction]
        )");

        py_boolean_function.def("get_variable_names", &BooleanFunction::get_variable_names, R"(
            Returns the set of variable names used by the Boolean function.

            :returns: A set of variable names.
            :rtype: list[str]
        )");

        py_boolean_function.def(
            "__str__", [](const BooleanFunction& f) { return f.to_string(); }, R"(
            Translates the Boolean function into its string representation.

            :returns: The Boolean function as a string.
            :rtype: str
        )");

        py_boolean_function.def_static("from_string", &BooleanFunction::from_string, py::arg("expression"), R"(
            Parses a Boolean function from a string expression.

            :param str expression: Boolean function string.
            :returns: The Boolean function on success, a string error message otherwise.
            :rtype: hal_py.BooleanFunction or str
        )");

        // TODO lots of functions

        py::class_<BooleanFunction::Node> py_boolean_function_node(py_boolean_function, "Node", R"(
            Node refers to an abstract syntax tree node of a Boolean function. A node is an abstract base class for either an operation (e.g., AND, XOR) or an operand (e.g., a signal name variable).
        )");

        // TODO lots of functions

        /*py_boolean_function.def(
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
        */
    }
}    // namespace hal
