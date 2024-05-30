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

        py::class_<HawkeyePlugin, RawPtrWrapper<HawkeyePlugin>, BasePluginInterface> py_hawkeye_plugin(m, "HawkeyePlugin");
        py_hawkeye_plugin.def_property_readonly("name", &HawkeyePlugin::get_name, R"(
            The name of the plugin.

            :type: str
        )");

        py_hawkeye_plugin.def("get_name", &HawkeyePlugin::get_name, R"(
            Get the name of the plugin.

            :returns: Plugin name.
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

            :returns: Plugin version.
            :rtype: str
        )");

        py::class_<hawkeye::SBoxDatabase, RawPtrWrapper<hawkeye::SBoxDatabase>> py_hawkeye_sbox_database(m, "SBoxDatabase", R"(
            Database of known S-boxes.
        )");

        py_hawkeye_sbox_database.def(py::init<>(), R"(
            Constructs an empty S-box database.
        )");

        py_hawkeye_sbox_database.def(py::init<const std::map<std::string, std::vector<u8>>&>(), py::arg("sboxes"), R"(
            Constructs an S-box database from the given S-boxes.

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
            Constructs an S-box database from file.

            :param pathlib.Path file_path: The path from which to load the S-box database file.
            :returns: The S-box database on success, an error otherwise.
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
            Add multiple S-boxes to the database.

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
            [](hawkeye::SBoxDatabase& self, const std::filesystem::path& file_path, bool overwrite = true) -> bool {
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
            py::arg("overwrite") = true,
            R"(
            Load S-boxes to the database from a file.

            :param pathlib.Path file_path: The path from which to load the S-box database file.
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

        py::class_<hawkeye::DetectionConfiguration, RawPtrWrapper<hawkeye::DetectionConfiguration>> py_hawkeye_detection_configuration(m, "DetectionConfiguration", R"(TODO)");

        py_hawkeye_detection_configuration.def(py::init<>(), R"(
            Constructs a default DetectionConfiguration.
        )");

        py::enum_<hawkeye::DetectionConfiguration::Control> py_hawkeye_detection_configuration_control(py_hawkeye_detection_configuration, "Control", R"(TODO)");

        py_hawkeye_detection_configuration_control.value("CHECK_FF", hawkeye::DetectionConfiguration::Control::CHECK_FF, R"(TODO)")
            .value("CHECK_TYPE", hawkeye::DetectionConfiguration::Control::CHECK_TYPE, R"(TODO)")
            .value("CHECK_PINS", hawkeye::DetectionConfiguration::Control::CHECK_PINS, R"(TODO)")
            .value("CHECK_NETS", hawkeye::DetectionConfiguration::Control::CHECK_NETS, R"(TODO)")
            .export_values();

        py_hawkeye_detection_configuration.def_readwrite("control", &hawkeye::DetectionConfiguration::control, R"(
            TODO

            :type: hawkeye.DetectionConfiguration.Control
        )");

        py::enum_<hawkeye::DetectionConfiguration::Components> py_hawkeye_detection_configuration_components(py_hawkeye_detection_configuration, "Components", R"(TODO)");

        py_hawkeye_detection_configuration_components.value("NONE", hawkeye::DetectionConfiguration::Components::NONE, R"(TODO)")
            .value("CHECK_SCC", hawkeye::DetectionConfiguration::Components::CHECK_SCC, R"(TODO)")
            .export_values();

        py_hawkeye_detection_configuration.def_readwrite("components", &hawkeye::DetectionConfiguration::components, R"(
            TODO
            
            :type: hawkeye.DetectionConfiguration.Components
        )");

        py_hawkeye_detection_configuration.def_readwrite("equivalent_types", &hawkeye::DetectionConfiguration::equivalent_types, R"(
            TODO

            :type: list[list[str]]
        )");

        py_hawkeye_detection_configuration.def_readwrite("timeout", &hawkeye::DetectionConfiguration::timeout, R"(
            TODO
            
            :type: int
        )");

        py_hawkeye_detection_configuration.def_readwrite("min_register_size", &hawkeye::DetectionConfiguration::min_register_size, R"(
            TODO
            
            :type: int
        )");

        py::class_<hawkeye::RegisterCandidate, RawPtrWrapper<hawkeye::RegisterCandidate>> py_hawkeye_register_candidate(m, "RegisterCandidate", R"(
            Holds all information on a crypto candidate.
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

        py::class_<hawkeye::RoundCandidate, RawPtrWrapper<hawkeye::RoundCandidate>> py_hawkeye_state_candidate(m, "RoundCandidate", R"(
            Holds all information on a crypto candidate.
        )");

        py_hawkeye_state_candidate.def_static(
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
            Computes a state candidate from the previously identified register candidate.
            The netlist of this candidate will be a partial copy of the original netlist, comprising only the gates belonging to the registers and the logic computing the next state.

            :param hawkeye.RegisterCandidate candidate: The register candidate.
            :returns: The state candidate on success, ``None`` otherwise.
            :rtype: hawkeye.RoundCandidate or None
        )");

        py_hawkeye_state_candidate.def("get_netlist", &hawkeye::RoundCandidate::get_netlist, R"(
            Get the netlist of the state candidate. The netlist will be a partial copy of the netlist of the register candidate.

            :returns: The netlist of the candidate.
            :rtype: hal_py.Netlist
        )");

        py_hawkeye_state_candidate.def("get_graph", &hawkeye::RoundCandidate::get_graph, R"(
            Get the netlist graph of the state candidate.

            :returns: The netlist graph of the candidate.
            :rtype: graph_algorithm.NetlistGraph
        )");

        py_hawkeye_state_candidate.def("get_size", &hawkeye::RoundCandidate::get_size, R"(
            Get the size of the candidate, i.e., the width of its registers.

            :returns: The size of the candidate.
            :rtype: int
        )");

        py_hawkeye_state_candidate.def("get_input_reg", &hawkeye::RoundCandidate::get_input_reg, R"(
            Get the candidate's input register.

            :returns: The input register of the candidate.
            :rtype: set[hal_py.Gate]
        )");

        py_hawkeye_state_candidate.def("get_output_reg", &hawkeye::RoundCandidate::get_output_reg, R"(
            Get the candidate's output register.

            :returns: The output register of the candidate.
            :rtype: set[hal_py.Gate]
        )");

        py_hawkeye_state_candidate.def("get_state_logic", &hawkeye::RoundCandidate::get_state_logic, R"(
            Get the candidate's combinational logic computing the next state.

            :returns: The state logic of the candidate.
            :rtype: set[hal_py.Gate]
        )");

        py_hawkeye_state_candidate.def("get_state_inputs", &hawkeye::RoundCandidate::get_state_inputs, R"(
            Get the candidate's state inputs to the logic computing the next state.

            :returns: The state inputs of the candidate.
            :rtype: set[hal_py.Net]
        )");

        py_hawkeye_state_candidate.def("get_control_inputs", &hawkeye::RoundCandidate::get_control_inputs, R"(
            Get the candidate's control inputs to the logic computing the next state.

            :returns: The control inputs of the candidate.
            :rtype: set[hal_py.Net]
        )");

        py_hawkeye_state_candidate.def("get_other_inputs", &hawkeye::RoundCandidate::get_other_inputs, R"(
            Get the candidate's other inputs to the logic computing the next state.

            :returns: The other inputs of the candidate.
            :rtype: set[hal_py.Net]
        )");

        py_hawkeye_state_candidate.def("get_state_outputs", &hawkeye::RoundCandidate::get_state_outputs, R"(
            Get the candidate's state outputs from the logic computing the next state.

            :returns: The state outputs of the candidate.
            :rtype: set[hal_py.Net]
        )");

        py_hawkeye_state_candidate.def("get_input_ffs_of_gate", &hawkeye::RoundCandidate::get_input_ffs_of_gate, R"(
            Get a dict from each combinational gate of the round function to all the input flip-flops it depends on.

            :returns: A dict from gates to sets of input flip-flops.
            :rtype: dict[hal_py.Gate,set[hal_py.Gate]]
        )");

        py_hawkeye_state_candidate.def("get_state_outputs", &hawkeye::RoundCandidate::get_state_outputs, R"(
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
            R"(TODO description

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
            R"(Stores all information related to an S-box candidate such as the ``RoundCandidate`` it belongs to, the connected component it is part of, and its input and output gates.)");

        py_hawkeye_sbox_candidate.def(py::init<>(), R"(
            TODO
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
            R"(Tries to locate S-box candidates within the combinational next-state logic of the round function candidate. 

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
            R"(Tries to identify an S-box candidate by matching it against a database of known S-boxes under affine equivalence. 

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
