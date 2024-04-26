#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void boolean_function_net_decorator_init(py::module& m)
    {
        py::class_<BooleanFunctionNetDecorator> py_boolean_function_net_decorator(
            m, "BooleanFunctionNetDecorator", R"(A net decorator that provides functionality to translate between nets and Boolean function variables.)");

        py_boolean_function_net_decorator.def(py::init<const Net&>(), py::arg("net"), R"(
            Construct new BooleanFunctionNetDecorator object.

            :param hal_py.Net net: The net to operate on.
        )");

        py_boolean_function_net_decorator.def("get_boolean_variable", &BooleanFunctionNetDecorator::get_boolean_variable, R"(
            Generate a unique Boolean variable representing the given net.

            :returns: The Boolean variable.
            :rtype: hal_py.BooleanFunction
        )");

        py_boolean_function_net_decorator.def("get_boolean_variable_name", &BooleanFunctionNetDecorator::get_boolean_variable_name, R"(
            Generate a unique Boolean variable name representing the given net.

            :returns: The Boolean variable name.
            :rtype: str
        )");

        py_boolean_function_net_decorator.def_static(
            "get_net_from",
            [](const Netlist* netlist, const BooleanFunction& var) -> Net* {
                auto res = BooleanFunctionNetDecorator::get_net_from(netlist, var);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting the net represented by a Boolean variable:\n{}", res.get_error().get());
                    return nullptr;
                }
            },
            py::arg("netlist"),
            py::arg("var"),
            R"(
            Get the net represented by a unique Boolean variable.

            :param hal_py.Netlist netlist: The netlist on which to operate.
            :param hal_py.BooleanFunction var: The Boolean variable.
            :returns: The specified net on success, None otherwise.
            :rtype: hal_py.Net or None
        )");

        py_boolean_function_net_decorator.def_static(
            "get_net_from",
            [](const Netlist* netlist, const std::string& var_name) -> Net* {
                auto res = BooleanFunctionNetDecorator::get_net_from(netlist, var_name);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting the net represented by a Boolean variable name:\n{}", res.get_error().get());
                    return nullptr;
                }
            },
            py::arg("netlist"),
            py::arg("var_name"),
            R"(
            Get the net represented by a unique Boolean variable name.

            :param hal_py.Netlist netlist: The netlist on which to operate.
            :param str var_name: The Boolean variable name.
            :returns: The specified net on success, None otherwise.
            :rtype: hal_py.Net or None
        )");
    }
}    // namespace hal