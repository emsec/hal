

#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void subgraph_netlist_decorator_init(py::module& m)
    {
        py::class_<SubgraphNetlistDecorator> py_subgraph_netlist_decorator(m, "SubgraphNetlistDecorator", R"()");

        py_subgraph_netlist_decorator.def(py::init<const Netlist&>(), py::arg("netlist"), R"(
            Construct new SubgraphNetlistDecorator object.

            :param hal_py.Netlist netlist: The netlist to operate on.
        )");

        py_subgraph_netlist_decorator.def(
            "copy_subgraph_netlist",
            [](SubgraphNetlistDecorator& self, const std::vector<const Gate*>& subgraph_gates, const bool outside_conncetions_as_global_io) -> std::shared_ptr<Netlist> {
                auto res = self.copy_subgraph_netlist(subgraph_gates, outside_conncetions_as_global_io);
                if (res.is_ok())
                {
                    return std::shared_ptr<Netlist>(res.get());
                }
                else
                {
                    log_error("python_context", "error encountered while copying subgraph netlist:\n{}", res.get_error().get());
                    return nullptr;
                }
            },
            py::arg("subgraph_gates"),
            py::arg("outside_conncetions_as_global_io"),
            R"(
            Get a deep copy of a netlist subgraph including all of its gates and nets, but excluding modules and groupings.

            :param list[hal_py.Gate] subgraph_gates: The gates making up the subgraph that shall be copied from the netlist.
            :param bool outside_conncetions_as_global_io: Option determining what nets are considered as global inout. Either only nets with no source or destination or all nets that lost any conneciton to a source or destination.
            :returns: The copied subgraph netlist on success, None otherwise.
            :rtype: hal_py.Netlist or None
        )");

        py_subgraph_netlist_decorator.def(
            "copy_subgraph_netlist",
            [](SubgraphNetlistDecorator& self, const Module* subgraph_module, const bool outside_conncetions_as_global_io) -> std::shared_ptr<Netlist> {
                auto res = self.copy_subgraph_netlist(subgraph_module, outside_conncetions_as_global_io);
                if (res.is_ok())
                {
                    return std::shared_ptr<Netlist>(res.get());
                }
                else
                {
                    log_error("python_context", "error encountered while copying subgraph netlist:\n{}", res.get_error().get());
                    return nullptr;
                }
            },
            py::arg("subgraph_module"),
            py::arg("outside_conncetions_as_global_io"),
            R"(
            Get a deep copy of a netlist subgraph including all of its gates and nets, but excluding modules and groupings.

            :param hal_py.Module subgraph_module: The module making up the subgraph that shall be copied from the netlist.
            :param bool outside_conncetions_as_global_io: Option determining what nets are considered as global inout. Either only nets with no source or destination or all nets that lost any conneciton to a source or destination.
            :returns: The copied subgraph netlist on success, None otherwise.
            :rtype: hal_py.Netlist or None
        )");

        py_subgraph_netlist_decorator.def(
            "get_subgraph_function",
            [](SubgraphNetlistDecorator& self, const std::vector<const Gate*>& subgraph_gates, const Net* subgraph_output, std::map<std::pair<u32, const GatePin*>, BooleanFunction>& cache)
                -> std::optional<BooleanFunction> {
                auto res = self.get_subgraph_function(subgraph_gates, subgraph_output, cache);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while generating subgraph function:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("subgraph_gates"),
            py::arg("subgraph_output"),
            py::arg("cache"),
            R"(
            Get the combined Boolean function of a subgraph of combinational gates starting at the source of the provided subgraph output net.
            The variables of the resulting Boolean function are created from the subgraph input nets using 'BooleanFunctionNetDecorator.get_boolean_variable'.
            Utilizes a cache for speedup on consecutive calls.

            :param list[hal_py.Gate] subgraph_gates: The gates making up the subgraph to consider.
            :param hal_py.Net subgraph_output: The subgraph oputput net for which to generate the Boolean function.
            :param dict[tuple(int,hal_py.GatePin),hal_py.BooleanFunction] cache: Cache to speed up computations. The cache is filled by this function.
            :returns: The combined Boolean function of the subgraph on success, None otherwise.
            :rtype: hal_py.BooleanFunction or None
        )");

        py_subgraph_netlist_decorator.def(
            "get_subgraph_function",
            [](SubgraphNetlistDecorator& self, const std::vector<const Gate*>& subgraph_gates, const Net* subgraph_output) -> std::optional<BooleanFunction> {
                auto res = self.get_subgraph_function(subgraph_gates, subgraph_output);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while generating subgraph function:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("subgraph_gates"),
            py::arg("subgraph_output"),
            R"(
            Get the combined Boolean function of a subgraph of combinational gates starting at the source of the provided subgraph output net.
            The variables of the resulting Boolean function are created from the subgraph input nets using 'BooleanFunctionNetDecorator.get_boolean_variable'.

            :param list[hal_py.Gate] subgraph_gates: The gates making up the subgraph to consider.
            :param hal_py.Net subgraph_output: The subgraph oputput net for which to generate the Boolean function.
            :returns: The combined Boolean function of the subgraph on success, None otherwise.
            :rtype: hal_py.BooleanFunction or None
        )");

        py_subgraph_netlist_decorator.def(
            "get_subgraph_function",
            [](SubgraphNetlistDecorator& self, const Module* subgraph_module, const Net* subgraph_output, std::map<std::pair<u32, const GatePin*>, BooleanFunction>& cache)
                -> std::optional<BooleanFunction> {
                auto res = self.get_subgraph_function(subgraph_module, subgraph_output, cache);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while generating subgraph function:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("subgraph_module"),
            py::arg("subgraph_output"),
            py::arg("cache"),
            R"(
            Get the combined Boolean function of a subgraph of combinational gates starting at the source of the provided subgraph output net.
            The variables of the resulting Boolean function are created from the subgraph input nets using 'BooleanFunctionNetDecorator.get_boolean_variable'.
            Utilizes a cache for speedup on consecutive calls.

            :param hal_py.Module subgraph_module: The module making up the subgraph to consider.
            :param hal_py.Net subgraph_output: The subgraph oputput net for which to generate the Boolean function.
            :param dict[tuple(int,hal_py.GatePin),hal_py.BooleanFunction] cache: Cache to speed up computations. The cache is filled by this function.
            :returns: The combined Boolean function of the subgraph on success, None otherwise.
            :rtype: hal_py.BooleanFunction or None
        )");

        py_subgraph_netlist_decorator.def(
            "get_subgraph_function",
            [](SubgraphNetlistDecorator& self, const Module* subgraph_module, const Net* subgraph_output) -> std::optional<BooleanFunction> {
                auto res = self.get_subgraph_function(subgraph_module, subgraph_output);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while generating subgraph function:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("subgraph_module"),
            py::arg("subgraph_output"),
            R"(
            Get the combined Boolean function of a subgraph of combinational gates starting at the source of the provided subgraph output net.
            The variables of the resulting Boolean function are created from the subgraph input nets using 'BooleanFunctionNetDecorator.get_boolean_variable'.

            :param hal_py.Module subgraph_module:The module making up the subgraph to consider.
            :param hal_py.Net subgraph_output: The subgraph oputput net for which to generate the Boolean function.
            :returns: The combined Boolean function of the subgraph on success, None otherwise.
            :rtype: hal_py.BooleanFunction or None
        )");

        py_subgraph_netlist_decorator.def(
            "get_subgraph_function_inputs",
            [](SubgraphNetlistDecorator& self, const std::vector<Gate*>& subgraph_gates, const Net* subgraph_output) -> std::optional<std::set<const Net*>> {
                auto res = self.get_subgraph_function_inputs(subgraph_gates, subgraph_output);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while generating subgraph function:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("subgraph_gates"),
            py::arg("subgraph_output"),
            R"(
            Get the inputs of the combined Boolean function of a subgraph of combinational gates starting at the source of the provided subgraph output net.
            This does not actually build the boolean function but only determines the inputs the subgraph function would have, which is a lot faster.

            :param list[hal_py.Gate] subgraph_gates: The subgraph_gates making up the subgraph to consider.
            :param hal_py.Net subgraph_output: The subgraph oputput net from which to start the back propagation from.
            :returns: The input nets that would be the input for the subgraph function
            :rtype: set(hal_py.Net) or None
        )");

        py_subgraph_netlist_decorator.def(
            "get_subgraph_function_inputs",
            [](SubgraphNetlistDecorator& self, const Module* subgraph_module, const Net* subgraph_output) -> std::optional<std::set<const Net*>> {
                auto res = self.get_subgraph_function_inputs(subgraph_module, subgraph_output);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "error encountered while generating subgraph function:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("subgraph_module"),
            py::arg("subgraph_output"),
            R"(
            Get the inputs of the combined Boolean function of a subgraph of combinational gates starting at the source of the provided subgraph output net.
            This does not actually build the boolean function but only determines the inputs the subgraph function would have, which is a lot faster.

            :param hal_py.Module subgraph_module:The module making up the subgraph to consider.
            :param hal_py.Net subgraph_output: The subgraph oputput net from which to start the back propagation from.
            :returns: The input nets that would be the input for the subgraph function
            :rtype: set(hal_py.Net) or None
        )");
    }
}    // namespace hal