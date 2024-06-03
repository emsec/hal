#include "hal_core/python_bindings/python_bindings.h"

#include "hawkeye/candidate_search.h"
#include "hawkeye/plugin_hawkeye.h"
#include "hawkeye/round_candidate.h"
#include "hawkeye/sbox_database.h"
#include "hawkeye/sbox_lookup.h"
#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

namespace py = pybind11;

namespace hal
{

    // the name in PYBIND11_MODULE/PYBIND11_PLUGIN *MUST* match the filename of the output library (without extension),
    // otherwise you will get "ImportError: dynamic module does not define module export function" when importing the module

#ifdef PYBIND11_MODULE
    PYBIND11_MODULE(hawkeye, m)
    {
        m.doc() = "hal HawkeyePlugin python bindings";
#else
    PYBIND11_PLUGIN(hawkeye)
    {
        py::module m("hawkeye", "hal HawkeyePlugin python bindings");
#endif    // ifdef PYBIND11_MODULE

        py::class_<HawkeyePlugin, RawPtrWrapper<HawkeyePlugin>, BasePluginInterface> py_hawkeye_plugin(
            m, "HawkeyePlugin", R"(This class provides an interface to integrate the HAWKEYE tool as a plugin within the HAL framework.)");

        py_hawkeye_plugin.def_property_readonly("name", &HawkeyePlugin::get_name, R"(
            The name of the plugin.

            :type: str
        )");

        py_hawkeye_plugin.def("get_name", &HawkeyePlugin::get_name, R"(
            Get the name of the plugin.

            :returns: The name of the plugin.
            :rtype: str
        )");

        py_hawkeye_plugin.def_property_readonly("description", &HawkeyePlugin::get_description, R"(
            The description of the plugin.

            :type: str
        )");

        py_hawkeye_plugin.def("get_description", &HawkeyePlugin::get_description, R"(
            Get the description of the plugin.

            :returns: The description of the plugin.
            :rtype: str
        )");

        py_hawkeye_plugin.def_property_readonly("version", &HawkeyePlugin::get_version, R"(
            The version of the plugin.

            :type: str
        )");

        py_hawkeye_plugin.def("get_version", &HawkeyePlugin::get_version, R"(
            Get the version of the plugin.

            :returns: The version of the plugin.
            :rtype: str
        )");

        py_hawkeye_plugin.def_property_readonly("dependencies", &HawkeyePlugin::get_dependencies, R"(
            A set of plugin names that this plugin depends on.

            :type: set[str]
        )");

        py_hawkeye_plugin.def("get_dependencies", &HawkeyePlugin::get_dependencies, R"(
            Get a set of plugin names that this plugin depends on.

            :returns: A set of plugin names that this plugin depends on.
            :rtype: set[str]
        )");

        py::class_<hawkeye::SBoxDatabase, RawPtrWrapper<hawkeye::SBoxDatabase>> py_hawkeye_sbox_database(m, "SBoxDatabase", R"(
            This class holds and manages known S-boxes and allows to perform efficient S-box lookups in the database.
        )");

        py_hawkeye_sbox_database.def(py::init<>(), R"(
            Construct an empty S-box database.
        )");

        py_hawkeye_sbox_database.def(py::init<const std::map<std::string, std::vector<u8>>&>(), py::arg("sboxes"), R"(
            Construct an S-box database from the given S-boxes.

            :param dict[str,list[int]] sboxes: A dict from S-box name to the respective S-box.
        )");

        py_hawkeye_sbox_database.def_static(
            "from_file",
            [](const std::filesystem::path& file_path) -> std::optional<hawkeye::SBoxDatabase> {
                auto res = hawkeye::SBoxDatabase::from_file(file_path);
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
            py::arg("file_path"),
            R"(
            Construct an S-box database from file.

            :param pathlib.Path file_path: The path from which to load the S-box database file.
            :returns: The S-box database on success, ``None`` otherwise.
            :rtype: hawkeye.SBoxDatabase or None
        )");

        py_hawkeye_sbox_database.def_static("compute_linear_representative", &hawkeye::SBoxDatabase::compute_linear_representative, py::arg("sbox"), R"(
            Compute the linear representative of the given S-box.

            :param list[int] sbox: The S-box.
            :returns: The linear representative.
            :rtype: list[int]
        )");

        py_hawkeye_sbox_database.def(
            "add",
            [](hawkeye::SBoxDatabase& self, const std::string& name, const std::vector<u8>& sbox) -> bool {
                auto res = self.add(name, sbox);
                if (res.is_ok())
                {
                    return true;
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return false;
                }
            },
            py::arg("name"),
            py::arg("sbox"),
            R"(
            Add an S-box to the database.

            :param str name: The name of the S-box.
            :patam list[int] sbox: The S-box.
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        py_hawkeye_sbox_database.def(
            "add",
            [](hawkeye::SBoxDatabase& self, const std::map<std::string, std::vector<u8>>& sboxes) -> bool {
                auto res = self.add(sboxes);
                if (res.is_ok())
                {
                    return true;
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return false;
                }
            },
            py::arg("sboxes"),
            R"(
            Add multiple S-boxes to the database.

            :param dict[str,list[int]] sboxes: A dict from S-box name to the respective S-box.
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        py_hawkeye_sbox_database.def(
            "load",
            [](hawkeye::SBoxDatabase& self, const std::filesystem::path& file_path, bool overwrite = false) -> bool {
                auto res = self.load(file_path, overwrite);
                if (res.is_ok())
                {
                    return true;
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return false;
                }
            },
            py::arg("file_path"),
            py::arg("overwrite") = false,
            R"(
            Load S-boxes from a file and add them to the existing database.

            :param pathlib.Path file_path: The path from which to load the S-box database file.
            :param bool overwrite: Set ``True`` to overwrite existing database, ``False`` otherwise. Defaults to ``False``.
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        py_hawkeye_sbox_database.def(
            "store",
            [](const hawkeye::SBoxDatabase& self, const std::filesystem::path& file_path) -> bool {
                auto res = self.store(file_path);
                if (res.is_ok())
                {
                    return true;
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return false;
                }
            },
            py::arg("file_path"),
            R"(
            Store the S-box database to a database file.

            :param pathlib.Path file_path: The path to where to store the S-box database file.
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        py_hawkeye_sbox_database.def(
            "lookup",
            [](const hawkeye::SBoxDatabase& self, const std::vector<u8>& sbox) -> std::optional<std::string> {
                auto res = self.lookup(sbox);
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
            py::arg("sbox"),
            R"(
            Attempt to look up an S-box in the database.

            :param list[int] sbox: The S-box to look for.
            :returns: The S-box name on success, ``None`` otherwise.
            :rtype: str or None
        )");

        py_hawkeye_sbox_database.def("print", &hawkeye::SBoxDatabase::print, R"(
            Print the database.
        )");

        py::class_<hawkeye::DetectionConfiguration, RawPtrWrapper<hawkeye::DetectionConfiguration>> py_hawkeye_detection_configuration(
            m, "DetectionConfiguration", R"(This class holds important parameters that configure the candidate search of HAWKEYE.)");

        py_hawkeye_detection_configuration.def(py::init<>(), R"(
            Constructs a default DetectionConfiguration.
        )");

        py::enum_<hawkeye::DetectionConfiguration::Control> py_hawkeye_detection_configuration_control(
            py_hawkeye_detection_configuration,
            "Control",
            R"(This enum specifies the checks that are to be performed on the flip-flops of the netlist to determine whether there should be an edge between two flip-flops or not.)");

        py_hawkeye_detection_configuration_control
            .value("CHECK_FF",
                   hawkeye::DetectionConfiguration::Control::CHECK_FF,
                   R"(If two flip-flops ``ff1`` and ``ff2`` are connected through combinational logic, an edge is added such that ``(ff1,ff2)`` is part of the graph.)")
            .value("CHECK_TYPE",
                   hawkeye::DetectionConfiguration::Control::CHECK_TYPE,
                   R"(If two flip-flops ``ff1`` and ``ff2`` are connected through combinational logic and are of the same gate type, an edge is added such that ``(ff1,ff2)`` is part of the graph.)")
            .value(
                "CHECK_PINS",
                hawkeye::DetectionConfiguration::Control::CHECK_PINS,
                R"(If two flip-flops ``ff1`` and ``ff2`` are connected through combinational logic and are controlled through the same input pins, an edge is added such that ``(ff1,ff2)`` is part of the graph.)")
            .value(
                "CHECK_NETS",
                hawkeye::DetectionConfiguration::Control::CHECK_NETS,
                R"(If two flip-flops ``ff1`` and ``ff2`` are connected through combinational logic and are controlled through the same input nets, an edge is added such that ``(ff1,ff2)`` is part of the graph.)")
            .export_values();

        py_hawkeye_detection_configuration.def_readwrite("control", &hawkeye::DetectionConfiguration::control, R"(
            Checks to be performed on flip-flop control inputs during candidate search.

            :type: hawkeye.DetectionConfiguration.Control
        )");

        py::enum_<hawkeye::DetectionConfiguration::Components> py_hawkeye_detection_configuration_components(py_hawkeye_detection_configuration,
                                                                                                             "Components",
                                                                                                             R"(
                This enum specifies whether SCC detection should be used to refine the results of neighborhood discovery. If SCC detection is used, the exploration only stops if the size of the largest discovered SCC saturates. Specifically, it does no longer require the size of the entire neighborhood to saturate.
        )");

        py_hawkeye_detection_configuration_components
            .value("NONE", hawkeye::DetectionConfiguration::Components::NONE, R"(Do not use SCC detection and instead resort to the simple neighborhood discovery algorithm.)")
            .value("CHECK_SCC", hawkeye::DetectionConfiguration::Components::CHECK_SCC, R"(Use SCC detection within the currently explored neighborhood of a start flip-flop.)")
            .export_values();

        py_hawkeye_detection_configuration.def_readwrite("components", &hawkeye::DetectionConfiguration::components, R"(
            Determines whether to use SCC detection as part of neighborhood discovery.
            
            :type: hawkeye.DetectionConfiguration.Components
        )");

        py_hawkeye_detection_configuration.def_readwrite("equivalent_types", &hawkeye::DetectionConfiguration::equivalent_types, R"(
            A list of a list of gate types that are treated as identical types by the candidate search, i.e., when checking equality of the types of two gates that are different but declared equivalent, ``True`` is returned.

            :type: list[list[str]]
        )");

        py_hawkeye_detection_configuration.def_readwrite("timeout", &hawkeye::DetectionConfiguration::timeout, R"(
            Neighborhood discovery iteration timeout.
            
            :type: int
        )");

        py_hawkeye_detection_configuration.def_readwrite("min_register_size", &hawkeye::DetectionConfiguration::min_register_size, R"(
            Minimum number of flip-flops for a register candidate to be created.
            
            :type: int
        )");

        py::class_<hawkeye::RegisterCandidate, RawPtrWrapper<hawkeye::RegisterCandidate>> py_hawkeye_register_candidate(m, "RegisterCandidate", R"(
            This class holds all information belonging to a register candidate discovered by HAWKEYE's candidate search and makes these information accessible through getters.
        )");

        py_hawkeye_register_candidate.def(py::init<>(), R"(Default constructor for ``RegisterCandidate``.)");

        py_hawkeye_register_candidate.def(py::init<const std::set<Gate*>&>(), py::arg("round_reg"), R"(
            Construct a state register candidate from the state register of a round-based implementation.

            :param set[hal_py.Gate] round_reg: The state register.            
        )");

        py_hawkeye_register_candidate.def(py::init<const std::set<Gate*>&, const std::set<Gate*>&>(), py::arg("in_reg"), py::arg("out_reg"), R"(
            Construct a state register candidate from the input and output registers from one round of a pipelined implementation.

            :param set[hal_py.Gate] in_reg: The input register.  
            :param set[hal_py.Gate] out_reg: The output register.            
        )");

        py_hawkeye_register_candidate.def("get_netlist", &hawkeye::RegisterCandidate::get_netlist, R"(
            Get the netlist associated with the candidate.

            :returns: The netlist of the candidate.
            :rtype: hal_py.Netlist
        )");

        py_hawkeye_register_candidate.def("get_size", &hawkeye::RegisterCandidate::get_size, R"(
            Get the size of the candidate, i.e., the width of its registers.

            :returns: The size of the candidate.
            :rtype: int
        )");

        py_hawkeye_register_candidate.def("is_round_based", &hawkeye::RegisterCandidate::is_round_based, R"(
            Check if the candidate is round-based, i.e., input and output register are the same.

            :returns: ``True`` if the candidate is round-based, ``False`` otherwise. 
            :rtype: bool
        )");

        py_hawkeye_register_candidate.def("get_input_reg", &hawkeye::RegisterCandidate::get_input_reg, R"(
            Get the candidate's input register.

            :returns: The input register of the candidate.
            :rtype: set[hal_py.Gate]
        )");

        py_hawkeye_register_candidate.def("get_output_reg", &hawkeye::RegisterCandidate::get_output_reg, R"(
            Get the candidate's output register.

            :returns: The output register of the candidate.
            :rtype: set[hal_py.Gate]
        )");

        py::class_<hawkeye::RoundCandidate, RawPtrWrapper<hawkeye::RoundCandidate>> py_hawkeye_round_candidate(m, "RoundCandidate", R"(
            This class holds all information belonging to a round candidate. Round candidates are constructed from register candidates by copying the sub-circuit consisting of the input and (if pipelined) output registers as well as the next-state/round-function logic in between these registers.
            For round-based implementations, commonly only a single register exists that acts as an input and output register at the same time.
            In such cases, this register is considered to be the input register of the round function and an exact copy of the register will be appended to the round function outputs so that input and output register are guaranteed to be distinct.
        )");

        py_hawkeye_round_candidate.def(py::init<>(), R"(Default constructor for ``RoundCandidate``.)");

        py_hawkeye_round_candidate.def_static(
            "from_register_candidate",
            [](hawkeye::RegisterCandidate* candidate) -> std::unique_ptr<hawkeye::RoundCandidate> {
                auto res = hawkeye::RoundCandidate::from_register_candidate(candidate);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return nullptr;
                }
            },
            py::arg("candidate"),
            R"(
            Compute a round candidate from a previously identified register candidate.
            The netlist of this candidate will be a partial copy of the original netlist, comprising only the gates belonging to the registers and the logic computing the next state.
            In case of a round-based implementation, the output register will be a copy of the input register.
            All data structures of the round candidate will be initialized in the process.

            :param hawkeye.RegisterCandidate candidate: The register candidate.
            :returns: The round candidate on success, ``None`` otherwise.
            :rtype: hawkeye.RoundCandidate or None
        )");

        py_hawkeye_round_candidate.def("get_netlist", &hawkeye::RoundCandidate::get_netlist, R"(
            Get the netlist of the round candidate. The netlist is a partial copy of the netlist of the register candidate.

            :returns: The netlist of the candidate.
            :rtype: hal_py.Netlist
        )");

        py_hawkeye_round_candidate.def("get_graph", &hawkeye::RoundCandidate::get_graph, R"(
            Get the netlist graph of the round candidate.

            :returns: The netlist graph of the candidate.
            :rtype: graph_algorithm.NetlistGraph
        )");

        py_hawkeye_round_candidate.def("get_size", &hawkeye::RoundCandidate::get_size, R"(
            Get the size of the candidate, i.e., the width of its registers.

            :returns: The size of the candidate.
            :rtype: int
        )");

        py_hawkeye_round_candidate.def("get_input_reg", &hawkeye::RoundCandidate::get_input_reg, R"(
            Get the candidate's input register.

            :returns: The input register of the candidate.
            :rtype: set[hal_py.Gate]
        )");

        py_hawkeye_round_candidate.def("get_output_reg", &hawkeye::RoundCandidate::get_output_reg, R"(
            Get the candidate's output register.

            :returns: The output register of the candidate.
            :rtype: set[hal_py.Gate]
        )");

        py_hawkeye_round_candidate.def("get_state_logic", &hawkeye::RoundCandidate::get_state_logic, R"(
            Get the candidate's combinational logic computing the next state.

            :returns: The state logic of the candidate.
            :rtype: set[hal_py.Gate]
        )");

        py_hawkeye_round_candidate.def("get_state_inputs", &hawkeye::RoundCandidate::get_state_inputs, R"(
            Get the candidate's state inputs to the logic computing the next state.

            :returns: The state inputs of the candidate.
            :rtype: set[hal_py.Net]
        )");

        py_hawkeye_round_candidate.def("get_control_inputs", &hawkeye::RoundCandidate::get_control_inputs, R"(
            Get the candidate's control inputs to the logic computing the next state.

            :returns: The control inputs of the candidate.
            :rtype: set[hal_py.Net]
        )");

        py_hawkeye_round_candidate.def("get_other_inputs", &hawkeye::RoundCandidate::get_other_inputs, R"(
            Get the candidate's other inputs to the logic computing the next state.

            :returns: The other inputs of the candidate.
            :rtype: set[hal_py.Net]
        )");

        py_hawkeye_round_candidate.def("get_state_outputs", &hawkeye::RoundCandidate::get_state_outputs, R"(
            Get the candidate's state outputs from the logic computing the next state.

            :returns: The state outputs of the candidate.
            :rtype: set[hal_py.Net]
        )");

        py_hawkeye_round_candidate.def("get_input_ffs_of_gate", &hawkeye::RoundCandidate::get_input_ffs_of_gate, R"(
            Get a dict from each combinational gate of the round function to all the input flip-flops it depends on.

            :returns: A dict from gates to sets of input flip-flops.
            :rtype: dict[hal_py.Gate,set[hal_py.Gate]]
        )");

        py_hawkeye_round_candidate.def("get_longest_distance_to_gate", &hawkeye::RoundCandidate::get_longest_distance_to_gate, R"(
            Get a dict from an integer distance to all gates that are reachable within at most that distance when starting at any input flip-flop.

            :returns: A dict from longest distance to a set of gates being reachable in at most that distance.
            :rtype: dict[int,set[hal_py.Gate]]
        )");

        m.def(
            "detect_candidates",
            [](Netlist* nl, const std::vector<hawkeye::DetectionConfiguration>& configs, u32 min_state_size = 40, const std::vector<Gate*>& start_ffs = {})
                -> std::optional<std::vector<hawkeye::RegisterCandidate>> {
                auto res = hawkeye::detect_candidates(nl, configs, min_state_size, start_ffs);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "cannot detect crypto candidates:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("nl"),
            py::arg("configs"),
            py::arg("min_state_size") = 40,
            py::arg("start_ffs")      = std::vector<Gate*>(),
            R"(
            Attempt to locate candidates for symmetric cryptographic implementations within a gate-level netlist.
            Search operates only on an abstraction of the netlist that contains only flip-flops as nodes and connections through combinational logic as edges.
            The algorithm computes the k-neighborhood of each flip-flop for ``k = 1, ..., config.timeout`` and stops when the neighborhood size saturates.
            Depending on the ``config``, additional criteria are used to narrow down the search space, see ``DetectionConfiguration.Control`` and ``DetectionConfiguration.Components`` for details.
            When the neighborhood size saturates, a register candidate is created if the last neighborhood size is larger than ``config.min_register_size``.
            After the candidates have been identified, they are reduced further to produce the final set of register candidates.
            To this end, large candidates that fully contain a smaller candidate and candidates that are smaller than ``min_state_size`` are discarded.

            :param hal_py.Netlist nl: The netlist to operate on.
            :param list[hawkeye.DetectionConfiguration] configs: The configurations of the detection approaches to be executed one after another on each start flip-flop.
            :param int min_state_size: The minimum size of a register candidate to be considered a cryptographic state register. Defaults to ``40``.
            :param list[hal_py.Gate] start_ffs: The flip-flops to analyze. Defaults to an empty list, i.e., all flip-flops in the netlist will be analyzed.
            :returns: A list of candidates on success, ``None`` otherwise.
            :rtype: list[hawkeye.RegisterCandidate] or None
        )");

        py::class_<hawkeye::SBoxCandidate, RawPtrWrapper<hawkeye::SBoxCandidate>> py_hawkeye_sbox_candidate(
            m,
            "SBoxCandidate",
            R"(This class stores all information related to an S-box candidate discovered within the round function of a round candidate, such as the ``RoundCandidate`` it belongs to, the connected component it is part of, and its input and output gates.)");

        py_hawkeye_sbox_candidate.def(py::init<>(), R"(
            Default constructor for ``SBoxCandidate``.
        )");

        py_hawkeye_sbox_candidate.def_readonly("m_candidate", &hawkeye::SBoxCandidate::m_candidate, R"(The ``RoundCandidate`` that the S-box candidate belongs to.)");

        py_hawkeye_sbox_candidate.def_readonly("m_component", &hawkeye::SBoxCandidate::m_component, R"(The gates of the component which the S-box candidate is part of.)");

        py_hawkeye_sbox_candidate.def_readonly("m_input_gates", &hawkeye::SBoxCandidate::m_input_gates, R"(The input gates of the S-box candidate (will be flip-flops).)");

        py_hawkeye_sbox_candidate.def_readonly(
            "m_output_gates", &hawkeye::SBoxCandidate::m_output_gates, R"(The output gates of the S-box candidate (usually combinational logic that is input to the linear layer).)");

        m.def(
            "locate_sboxes",
            [](const hawkeye::RoundCandidate* candidate) -> std::optional<std::vector<hawkeye::SBoxCandidate>> {
                auto res = hawkeye::locate_sboxes(candidate);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "cannot locate S-boxes:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("candidate"),
            R"(
            Try to locate S-box candidates within the combinational next-state logic of the round function candidate. 
            Computes an initial set of connected components within the round function extracted between the input and output register of the round candidate.
            If these initial components are reasonably small and their input and output sizes match, construct S-box candidates for further analysis right away.
            Otherwise, iteratively consider more combinational gates starting from the components' input gates and search for sub-components.
            Create S-box candidates for these sub-components after determining the respective S-box output gates.

            :param hawkeye.RoundCandidate candidate: A round function candidate.
            :returns: A list of S-box candidates on success, ``None`` otherwise.
            :rtype: list[hawkeye.SBoxCandidate] or None
        )");

        m.def(
            "identify_sbox",
            [](const hawkeye::SBoxCandidate& sbox_candidate, const hawkeye::SBoxDatabase& db) -> std::optional<std::string> {
                auto res = hawkeye::identify_sbox(sbox_candidate, db);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "cannot identify S-box:\n{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("sbox_candidate"),
            py::arg("db"),
            R"(
            Try to identify an S-box candidate by matching it against a database of known S-boxes under affine equivalence. 

            :param hawkeye.SBoxCandidate sbox_candidate: An S-box candidate.
            :param hawkeye.SBoxDatabase db: A database of known S-boxes.
            :returns: The name of the S-box on success, ``None`` otherwise.
            :rtype: str or None
        )");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
