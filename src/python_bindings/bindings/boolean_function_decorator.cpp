#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void boolean_function_decorator_init(py::module& m)
    {
        py::class_<BooleanFunctionDecorator> py_boolean_function_decorator(m, "BooleanFunctionDecorator", R"()");

        py_boolean_function_decorator.def(py::init<const BooleanFunction&>(), py::arg("bf"), R"(
            Construct new BooleanFunctionDecorator object.

            :param hal_py.BooleanFunction bf: The Booelan function to operate on.
        )");

        py_boolean_function_decorator.def(
            "substitute_power_ground_nets",
            [](const BooleanFunctionDecorator& self, const Netlist* nl) -> std::optional<BooleanFunction> {
                auto res = self.substitute_power_ground_nets(nl);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while substituting power and ground nets:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("nl"),
            R"(
            Substitute all Boolean function variables fed by power or ground gates by constant '1' and '0'. 

            :param hal_py.Netlist nl: The netlist to operate on.
            :returns: The resulting Boolean function on success, None otherwise.
            :rtype: hal_py.BooleanFunction or None
        )");

        py_boolean_function_decorator.def_static(
            "get_boolean_function_from",
            [](const std::vector<Net*>& nets, u32 extend_to_size = 0, bool sign_extend = false) -> std::optional<BooleanFunction> {
                auto res = BooleanFunctionDecorator::get_boolean_function_from(nets, extend_to_size, sign_extend);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting Boolean function from net vector:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("nets"),
            py::arg("extend_to_size") = 0,
            py::arg("sign_extend")    = false,
            R"(
            Get the Boolean function that is the concatenation of variable names corresponding to nets of a netlist.
            The Boolean function can optionally be extended to any desired size greater the size of the given net vector.

            :param list[hal_py.Net] nets: The nets to concatenate. 
            :param int extend_to_size: The size to which to extend the Boolean function. Set to 0 to prevent extension. Defaults to 0.
            :param bool sign_extend: Set True to sign extend, False to zero extend. Defaults to False.
            :returns: The resulting Boolean function on success, None otherwise.
            :rtype: hal_py.BooleanFunction or None
        )");

        py_boolean_function_decorator.def_static(
            "get_boolean_function_from",
            [](const std::vector<BooleanFunction>& functions, u32 extend_to_size = 0, bool sign_extend = false) -> std::optional<BooleanFunction> {
                auto res = BooleanFunctionDecorator::get_boolean_function_from(functions, extend_to_size, sign_extend);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while getting Boolean function from Boolean function vector:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("functions"),
            py::arg("extend_to_size") = 0,
            py::arg("sign_extend")    = false,
            R"(
            Get the Boolean function that is the concatenation of Boolean functions.
            The Boolean function can optionally be extended to any desired size greater the size of the given net vector.

            :param list[hal_py.BooleanFunction] functions: The Boolean functions to concatenate. 
            :param int extend_to_size: The size to which to extend the Boolean function. Set to 0 to prevent extension. Defaults to 0.
            :param bool sign_extend: Set True to sign extend, False to zero extend. Defaults to False.
            :returns: The resulting Boolean function on success, None otherwise.
            :rtype: hal_py.BooleanFunction or None
        )");
    }
}    // namespace hal