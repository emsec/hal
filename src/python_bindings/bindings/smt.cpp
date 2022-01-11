#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void smt_init(py::module& m)
    {
        auto py_smt = m.def_submodule("SMT", R"(
            SMT solver functions.
        )");

        py::enum_<SMT::SolverType> py_smt_solver_type(py_smt, "SolverType", R"(
            Identifier for the SMT solver type.
        )");

        py_smt_solver_type.value("Z3", SMT::SolverType::Z3, R"(Z3 SMT solver.)")
            .value("Boolector", SMT::SolverType::Boolector, R"(Boolector SMT solver.)")
            .value("Unknown", SMT::SolverType::Unknown, R"(Unknown (unsupported) SMT solver.)")
            .export_values();

        py::class_<SMT::QueryConfig> py_smt_query_config(py_smt, "QueryConfig", R"(
            Represents the data structure to configure an SMT query.
        )");

        py_smt_query_config.def("with_solver", &SMT::QueryConfig::with_solver, py::arg("solver"), R"(
            Sets the solver type to the desired SMT solver.

            :param hal_py.SMT.SolverType solver: The solver type identifier.
            :returns: The updated SMT query configuration.
            :rtype: hal_py.SMT.QueryConfig
        )");

        py_smt_query_config.def("with_local_solver", &SMT::QueryConfig::with_local_solver, R"(
            Activates local SMT solver execution.

            :returns: The updated SMT query configuration.
            :rtype: hal_py.SMT.QueryConfig
        )");

        py_smt_query_config.def("with_remote_solver", &SMT::QueryConfig::with_remote_solver, R"(
            Activates remote SMT solver execution.

            :returns: The updated SMT query configuration.
            :rtype: hal_py.SMT.QueryConfig
        )");

        py_smt_query_config.def("with_model_generation", &SMT::QueryConfig::with_model_generation, R"(
            Indicates that the SMT solver should generate a model in case the formula is satisfiable.

            :returns: The updated SMT query configuration.
            :rtype: hal_py.SMT.QueryConfig
        )");

        py_smt_query_config.def("without_model_generation", &SMT::QueryConfig::without_model_generation, R"(
            Indicates that the SMT solver should not generate a model.

            :returns: The updated SMT query configuration.
            :rtype: hal_py.SMT.QueryConfig
        )");

        py_smt_query_config.def("with_timeout", &SMT::QueryConfig::with_timeout, py::arg("seconds"), R"(
            Sets a timeout in seconds that terminates an SMT query after the specified time has passed.

            :param int seconds: The timeout in seconds.
            :returns: The updated SMT query configuration.
            :rtype: hal_py.SMT.QueryConfig
        )");

        py::class_<SMT::Constraint> py_smt_constraint(py_smt, "Constraint", R"(
            Represents a constraint to the SMT query, i.e., an assignment of two Boolean function that is true.
        )");

        py_smt_constraint.def_readwrite("lhs", &SMT::Constraint::lhs, R"(
            The left-hand side of the equality constraint.

            :type: hal_py.BooleanFunction    
        )");

        py_smt_constraint.def_readwrite("rhs", &SMT::Constraint::rhs, R"(
            The right-hand side of the equality constraint.

            :type: hal_py.BooleanFunction    
        )");

        py_smt_constraint.def(py::init<const BooleanFunction&, const BooleanFunction&>(), py::arg("lhs"), py::arg("rhs"), R"(
            Constructs a new equality constraint from two Boolean functions.

            :param hal_py.BooleanFunction lhs: The left-hand side of the equality constraint.
            :param hal_py.BooleanFunction rhs: The right-hand side of the equality constraint.
        )");

        py::enum_<SMT::ResultType> py_smt_result_type(py_smt, "ResultType", R"(
            Result type of an SMT solver query.
        )");

        py_smt_result_type.value("Sat", SMT::ResultType::Sat, R"(The list of constraints is satisfiable.)")
            .value("UnSat", SMT::ResultType::UnSat, R"(The list of constraints is not satisfiable.)")
            .value("Unknown", SMT::ResultType::Unknown, R"(A result could not be obtained, e.g., due to a time-out.)")
            .export_values();

        py::class_<SMT::Model> py_smt_model(py_smt, "Model", R"(
            Represents a list of assignments for variable nodes that yield a satisfiable assignment for a given list of constraints.
        )");

        py_smt_model.def(py::init<const std::map<std::string, std::tuple<u64, u16>>&>(), py::arg("model") = std::map<std::string, std::tuple<u64, u16>>(), R"(
            Constructs a new model from a map of variable names to value and bit-size.

            :param dict[str,tuple(int,int)] model: A dict from variable name to value and bit-size.
        )");

        py_smt_model.def(py::self == py::self, R"(
            Checks whether two SMT models are equal.

            :returns: True if both models are equal, False otherwise.
            :rtype: bool
        )");

        py_smt_model.def(py::self != py::self, R"(
            Checks whether two SMT models are unequal.

            :returns: True if both models are unequal, False otherwise.
            :rtype: bool
        )");

        py_smt_model.def_static("parse", &SMT::Model::parse, py::arg("model_str"), py::arg("solver"), R"(
            Parses an SMT-Lib model from a string output by a solver of the given type.

            :param str model_str: The SMT-Lib model string.
            :param hal_py.SMT.SolverType solver: The solver that computed the model.
            :returns: The model on success, a string error message otherwise.
            :rtype: hal_py.SMT.Model or str
        )");

        py::class_<SMT::Result> py_smt_result(py_smt, "Result", R"(
            Represents the result of an SMT query.
        )");

        py_smt_result.def_readwrite("type", &SMT::Result::type, R"(
            Result type of the SMT query.

            :type: hal_py.SMT.ResultType
        )");

        py_smt_result.def_readwrite("model", &SMT::Result::model, R"(
            The (optional) model that is only available if type == SMT.ResultType.Sat and model generation is enabled.

            :type: hal_py.SMT.Model
        )");

        py_smt_result.def_static("Sat", &SMT::Result::Sat, py::arg("model") = std::optional<SMT::Model>(), R"(
            Creates a satisfiable result with an optional model.

            :param hal_py.SMT.Model model: Optional model for satisfiable formula.
            :returns: The result.
            :rtype: hal_py.SMT.Result
        )");

        py_smt_result.def_static("UnSat", &SMT::Result::UnSat, R"(
            Creates an unsatisfiable result.

            :returns: The result.
            :rtype: hal_py.SMT.Result
        )");

        py_smt_result.def_static("Unknown", &SMT::Result::Unknown, R"(
            Creates an unknown result.

            :returns: The result.
            :rtype: hal_py.SMT.Result
        )");

        py_smt_result.def("is", &SMT::Result::is, py::arg("type"), R"(
            Checks whether the result is of a specific type.

            :param hal_py.SMT.ResultType type: The type to check.
            :returns: True in case result matches the given type, False otherwise.
            :rtype: bool
        )");

        py_smt_result.def("is_sat", &SMT::Result::is_sat, R"(
            Checks whether the result is satisfiable.

            :returns: True in case result is satisfiable, False otherwise.
            :rtype: bool
        )");

        py_smt_result.def("is_unsat", &SMT::Result::is_unsat, R"(
            Checks whether the result is unsatisfiable.

            :returns: True in case result is unsatisfiable, False otherwise.
            :rtype: bool
        )");

        py_smt_result.def("is_unknown", &SMT::Result::is_unknown, R"(
            Checks whether the result is unknown.

            :returns: True in case result is unknown, False otherwise.
            :rtype: bool
        )");

        py::class_<SMT::Solver> py_smt_solver(py_smt, "Solver", R"(
            Provides an interface to query SMT solvers for a list of constraints, i.e. statements that have to be equal. To this end, we translate constraints to a SMT-LIB v2 string representation and query solvers with a defined configuration, i.e., chosen solver, model generation etc.
        )");

        py_smt_solver.def(py::init<std::vector<SMT::Constraint>>(), py::arg("constraints") = std::vector<SMT::Constraint>(), R"(
            Constructs an solver with an optional list of constraints.

            :param list[hal_py.SMT.Constraint] constraints: The (optional) list of constraints.
        )");

        py_smt_solver.def_property_readonly("constraints", &SMT::Solver::get_constraints, R"(
            The list of constraints.

            :type: list[hal_py.SMT.Constraint]
        )");

        py_smt_solver.def("get_constraints", &SMT::Solver::get_constraints, R"(
            Returns the list of constraints.

            :returns: The list of constraints.
            :rtype: list[hal_py.SMT.Constraint]
        )");

        py_smt_solver.def("with_constraint", &SMT::Solver::with_constraint, py::arg("constraint"), R"(
            Adds a constraint to the SMT solver.

            :param hal_py.SMT.Constraint constraint: The constraint.
            :returns: The updated SMT solver.
            :rtype: hal_py.SMT.Solver
        )");

        py_smt_solver.def("with_constraints", &SMT::Solver::with_constraints, py::arg("constraints"), R"(
            Adds a list of constraints to the SMT solver.

            :param list[hal_py.SMT.Constraint] constraints: The constraints.
            :returns: The updated SMT solver.
            :rtype: hal_py.SMT.Solver
        )");

        py_smt_solver.def_static("has_local_solver_for", &SMT::Solver::has_local_solver_for, py::arg("type"), R"(
            Checks whether a SMT solver of the given type is available on the local machine.

            :param hal_py.SMT.SolverType type: The SMT solver type.
            :returns: True if an SMT solver of the requested type is available, False otherwise.
            :rtype: bool
        )");

        py_smt_solver.def("query", &SMT::Solver::query, py::arg("config"), R"(
            Queries an SMT solver with the specified query configuration.

            :param hal_py.SMT.QueryConfig config: The SMT solver query configuration.
            :returns: The result on success, a string error message otherwise.
            :rtype: hal_py.SMT.Result or str
        )");

        py_smt_solver.def("query_local", &SMT::Solver::query_local, py::arg("config"), R"(
            Queries a local SMT solver with the specified query configuration.

            :param hal_py.SMT.QueryConfig config: The SMT solver query configuration.
            :returns: The result on success, a string error message otherwise.
            :rtype: hal_py.SMT.Result or str
        )");

        py_smt_solver.def("query_remote", &SMT::Solver::query_remote, py::arg("config"), R"(
            Queries a remote SMT solver with the specified query configuration.

            WARNING: This function is not yet implemented.

            :param hal_py.SMT.QueryConfig config: The SMT solver query configuration.
            :returns: The result on success, a string error message otherwise.
            :rtype: hal_py.SMT.Result or str
        )");

        py::class_<SMT::SymbolicState> py_smt_symbolic_state(py_smt, "SymbolicState", R"(
           Represents the data structure that keeps track of symbolic variable values (e.g., required for symbolic simplification).
        )");

        py_smt_symbolic_state.def(py::init<const std::vector<BooleanFunction>&>(), py::arg("variables") = std::vector<BooleanFunction>(), R"(
            Constructs a symbolic state and (optionally) initializes the variables.

            :param list[hal_py.BooleanFunction] variables: The (optional) list of variables.
        )");

        py_smt_symbolic_state.def("get", &SMT::SymbolicState::get, py::arg("key"), R"(
            Looks up the Boolean function equivalent in the symbolic state.

            :param hal_py.BooleanFunction key: The Boolean function to look up.
            :returns: The Boolean function equivalent from the symbolic state or the key itself if it is not contained in the symbolic state.
            :rtype: hal_py.BooleanFunction
        )");

        py_smt_symbolic_state.def("set", &SMT::SymbolicState::set, py::arg("key"), py::arg("value"), R"(
            Sets a Boolean function equivalent in the symbolic state.

            :param hal_py.BooleanFunction key: The Boolean function.
            :param hal_py.BooleanFunction value: The equivalent Boolean function.
        )");

        py::class_<SMT::SymbolicExecution> py_smt_symbolic_execution(py_smt, "SymbolicExecution", R"(
           Represents the symbolic execution engine that handles the evaluation and simplification of Boolean function abstract syntax trees.
        )");

        py_smt_symbolic_execution.def_readwrite("state", &SMT::SymbolicExecution::state, R"(
            The current symbolic state.

            :type: hal_py.SMT.SymbolicState
        )");

        py_smt_symbolic_execution.def(py::init<const std::vector<BooleanFunction>&>(), py::arg("variables") = std::vector<BooleanFunction>(), R"(
            Creates a symbolic execution engine and (optionally) initializes the variables.

            :param list[hal_py.BooleanFunction] variables: The (optional) list of variables.
        )");

        py_smt_symbolic_execution.def("evaluate", py::overload_cast<const BooleanFunction&>(&SMT::SymbolicExecution::evaluate, py::const_), py::arg("function"), R"(
            Evaluates a Boolean function within the symbolic state of the symbolic execution.

            :param hal_py.BooleanFunction function: The Boolean function to evaluate.
            :returns: The evaluated Boolean function on success, a string error message otherwise.
            :rtype: hal_py.BooleanFunction or str
        )");

        py_smt_symbolic_execution.def("evaluate", py::overload_cast<const SMT::Constraint&>(&SMT::SymbolicExecution::evaluate), py::arg("constraint"), R"(
            Evaluates an equality constraint and applies it to the symbolic state of the symbolic execution.

            :param hal_py.SMT.Constraint constraint: The equality constraint to evaluate.
            :returns: None on success, a string error message otherwise.
            :rtype: None or str
        )");
    }
}    // namespace hal
