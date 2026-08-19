#include "hal_core/python_bindings/python_bindings.h"

#include "graph_algorithm/netlist_graph.h"

#include "hawkeye/candidate_search.h"
#include "hawkeye/cipher_candidate.h"
#include "hawkeye/plugin_hawkeye.h"
#include "hawkeye/sbox_database.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"

namespace py = pybind11;

namespace hal
{

    // the name in PYBIND11_MODULE/PYBIND11_PLUGIN *MUST* match the filename of the output library (without extension),
    // otherwise you will get "ImportError: dynamic module does not define module export function" when importing the module

#ifdef PYBIND11_MODULE
    PYBIND11_MODULE(hawkeye, m)
    {
        m.doc() = "Automated tool to locate arbitrary symmetric cryptographic implementations in gate-level netlists.";
#else
    PYBIND11_PLUGIN(hawkeye)
    {
        py::module m("hawkeye", "Automated tool to locate arbitrary symmetric cryptographic implementations in gate-level netlists.");
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

        py_hawkeye_plugin.def_property_readonly("version", &HawkeyePlugin::get_version, R"(
            The version of the plugin.

            :type: str
        )");

        py_hawkeye_plugin.def("get_version", &HawkeyePlugin::get_version, R"(
            Get the version of the plugin.

            :returns: The version of the plugin.
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
            m, "DetectionConfiguration", R"(This class holds important parameters that configure the candidate search of HAWKEYE, see ``CipherCandidate.detect``.)");

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
            Minimum number of flip-flops of a register for a candidate to be created from it.
            
            :type: int
        )");

        py::enum_<hawkeye::SBoxStatus> py_hawkeye_sbox_status(m, "SBoxStatus", R"(The outcome of trying to identify an S-box, see ``CipherCandidate.identify_sboxes``.)");

        py_hawkeye_sbox_status.value("unidentified", hawkeye::SBoxStatus::unidentified, R"(Identification ran but the S-box is not contained in the database.)")
            .value("identified", hawkeye::SBoxStatus::identified, R"(Identification ran and matched, see ``SBox.identified_as``.)")
            .value("superseded", hawkeye::SBoxStatus::superseded, R"(Identification did not run, another variant of the same S-box was identified before.)")
            .export_values();

        py::class_<hawkeye::SBox> py_hawkeye_sbox(m, "SBox", R"(
            An S-box located within the round function of a ``CipherCandidate``.

            Owned by the candidate it was located in, which also owns every gate it refers to, so an S-box is only valid for as long as its candidate is.

            The exact size and shape of an S-box is not known in advance, so the search deliberately produces more S-boxes than the round function actually contains, among them smaller ones nested inside larger ones. Identification resolves that, see ``SBoxStatus``.
        )");

        py_hawkeye_sbox.def_readonly("component", &hawkeye::SBox::component, R"(
            The gates of the connected component that the S-box was located in, including its input flip-flops.

            :type: list[hal_py.Gate]
        )");

        py_hawkeye_sbox.def_readonly("input_gates", &hawkeye::SBox::input_gates, R"(
            The input flip-flops of the S-box, ordered by gate ID.

            These are the flip-flops of the state register that the S-box reads, and hence the only link between the identified S-box and the state bits it operates on. They are **not** ordered by S-box input bit: the database matches under affine equivalence, which absorbs any permutation of the input and output bits, so no bit correspondence is established during identification.

            :type: list[hal_py.Gate]
        )");

        py_hawkeye_sbox.def_readonly("output_gates", &hawkeye::SBox::output_gates, R"(
            The output gates of the S-box, ordered by gate ID. Usually combinational gates feeding the linear layer.

            :type: list[hal_py.Gate]
        )");

        py_hawkeye_sbox.def_readonly("identified_as", &hawkeye::SBox::identified_as, R"(
            The name of the S-box in the database it was identified as, empty unless ``status`` is ``identified``.

            :type: str
        )");

        py_hawkeye_sbox.def_readonly("status", &hawkeye::SBox::status, R"(
            The outcome of trying to identify the S-box, ``unidentified`` until ``identify_sboxes`` ran.

            :type: hawkeye.SBoxStatus
        )");

        py_hawkeye_sbox.def("get_combinational_gates", &hawkeye::SBox::get_combinational_gates, R"(
            Get the combinational gates computing the outputs of the S-box from its input flip-flops.

            Walks back from the output gates within the component and stops at the flip-flops, so the result is the logic of this S-box alone rather than that of the whole component, which several S-boxes may share.

            :returns: The combinational gates of the S-box, ordered by gate ID.
            :rtype: list[hal_py.Gate]
        )");

        py::class_<hawkeye::CipherCandidate> py_hawkeye_cipher_candidate(m, "CipherCandidate", R"(
            A candidate for a symmetric cryptographic implementation within a netlist.

            A candidate is discovered by ``detect`` in stages and is filled in as the analysis proceeds: detection only establishes the state register, ``build_round_function`` adds the combinational logic computing the next state, and ``locate_sboxes`` adds the S-boxes within that logic. Use ``has_round_function`` and ``get_sboxes`` to find out how far a candidate has been analyzed.

            All gates and nets of a candidate belong to the netlist it was detected in, so they can be inspected and grouped into modules directly.
        )");

        py_hawkeye_cipher_candidate.def(py::init<>(), R"(Default constructor for ``CipherCandidate``.)");

        py_hawkeye_cipher_candidate.def(py::init<const std::set<Gate*>&>(), py::arg("round_reg"), R"(
            Construct a round-based candidate, i.e., one whose input and output register are the same.

            :param set[hal_py.Gate] round_reg: The state register of the candidate.
        )");

        py_hawkeye_cipher_candidate.def(py::init<const std::set<Gate*>&, const std::set<Gate*>&>(), py::arg("in_reg"), py::arg("out_reg"), R"(
            Construct a candidate from an input and an output register. The candidate is round-based if both registers are equal.

            :param set[hal_py.Gate] in_reg: The input register of the candidate.
            :param set[hal_py.Gate] out_reg: The output register of the candidate.
        )");

        py_hawkeye_cipher_candidate.def_static(
            "detect",
            [](Netlist* nl, const std::vector<hawkeye::DetectionConfiguration>& configs, u32 min_state_size = 40, const std::vector<Gate*>& start_ffs = {})
                -> std::optional<std::vector<hawkeye::CipherCandidate>> {
                auto res = hawkeye::CipherCandidate::detect(nl, configs, min_state_size, start_ffs);
                if (res.is_ok())
                {
                    return std::move(res.get());
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
            Attempt to locate candidates for symmetric cryptographic SPN, Feistel, and ARX implementations within a gate-level netlist.

            Operates on an abstraction of the netlist that holds only the flip-flops as vertices and their connections through combinational logic as edges.
            Computes the k-neighborhood of every flip-flop for ``k = 1, ..., config.timeout`` and stops once the size of the neighborhood saturates, at which point a candidate is created if the neighborhood is larger than ``config.min_register_size``.
            Depending on the ``config``, further criteria narrow down the search, see ``DetectionConfiguration.Control`` and ``DetectionConfiguration.Components``.
            The candidates found are then reduced by discarding those smaller than ``min_state_size`` as well as those that fully contain a smaller candidate.

            The returned candidates only know their state register, call ``build_round_function`` on a candidate to analyze it further.

            :param hal_py.Netlist nl: The netlist to operate on.
            :param list[hawkeye.DetectionConfiguration] configs: The configurations of the detection approaches to be executed one after another on each start flip-flop.
            :param int min_state_size: The minimum size of a candidate to be considered a cryptographic state register. Defaults to ``40``.
            :param list[hal_py.Gate] start_ffs: The flip-flops to analyze. Defaults to an empty list, i.e., all flip-flops of the netlist are analyzed.
            :returns: A list of candidates on success, ``None`` otherwise.
            :rtype: list[hawkeye.CipherCandidate] or None
        )");

        py_hawkeye_cipher_candidate.def(
            "build_round_function",
            [](hawkeye::CipherCandidate& self) -> bool {
                auto res = self.build_round_function();
                if (res.is_ok())
                {
                    return true;
                }
                log_error("python_context", "cannot build the round function of the candidate:\n{}", res.get_error().get());
                return false;
            },
            R"(
            Determine the round function of the candidate, i.e., the combinational logic computing the next state.

            Determines the state logic between the input and the output register together with the state, control, and other inputs of the candidate, and builds the graph that ``locate_sboxes`` operates on.
            Discards any S-boxes located so far, as they are derived from the round function, which invalidates all S-boxes previously returned by ``locate_sboxes`` and ``get_sboxes``.

            Recomputes the round function on every call, which only makes a difference if the netlist changed in the meantime.

            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        py_hawkeye_cipher_candidate.def(
            "locate_sboxes",
            [](hawkeye::CipherCandidate& self) -> std::optional<std::vector<hawkeye::SBox*>> {
                auto res = self.locate_sboxes();
                if (res.is_ok())
                {
                    return res.get();
                }
                log_error("python_context", "cannot locate S-boxes:\n{}", res.get_error().get());
                return std::nullopt;
            },
            py::return_value_policy::reference_internal,
            R"(
            Try to locate S-boxes within the round function of the candidate.

            Computes an initial set of connected components within the round function.
            If these components are reasonably small and their input and output sizes match, they are turned into S-boxes right away.
            Otherwise, iteratively considers more combinational gates starting from the components' input gates and searches for sub-components.

            Returns the S-boxes located by an earlier call unchanged instead of locating them again. Call ``clear_sboxes`` to locate them anew.

            :returns: The S-boxes of the candidate on success, ``None`` otherwise.
            :rtype: list[hawkeye.SBox] or None
        )");

        py_hawkeye_cipher_candidate.def("clear_sboxes", &hawkeye::CipherCandidate::clear_sboxes, R"(
            Discard the S-boxes located so far.

            Invalidates all S-boxes previously returned by ``locate_sboxes`` and ``get_sboxes``.
        )");

        py_hawkeye_cipher_candidate.def(
            "identify_sboxes",
            [](hawkeye::CipherCandidate& self, const hawkeye::SBoxDatabase& db) -> std::optional<u32> {
                auto res = self.identify_sboxes(db);
                if (res.is_ok())
                {
                    return res.get();
                }
                log_error("python_context", "cannot identify the S-boxes of the candidate:\n{}", res.get_error().get());
                return std::nullopt;
            },
            py::arg("db"),
            R"(
            Try to identify all S-boxes of the candidate by matching them against a database of known S-boxes.

            Annotates every S-box with the outcome, see ``SBox.status`` and ``SBox.identified_as``. An S-box that is not contained in the database is not an error.

            Since the exact outputs of an S-box are not known in advance, ``locate_sboxes`` produces many variants of the same S-box that differ only in which of the surplus gates are taken as its outputs but all read the same input flip-flops.
            Variants are therefore identified as a group, and the group is left as soon as one of them matches, marking the remaining ones ``superseded``.

            :param hawkeye.SBoxDatabase db: The database of known S-boxes.
            :returns: The number of identified S-boxes on success, ``None`` otherwise.
            :rtype: int or None
        )");

        py_hawkeye_cipher_candidate.def(
            "identify_sbox",
            [](const hawkeye::CipherCandidate& self, const hawkeye::SBox* sbox, const hawkeye::SBoxDatabase& db) -> std::optional<std::string> {
                auto res = self.identify_sbox(sbox, db);
                if (res.is_ok())
                {
                    return res.get();
                }
                log_error("python_context", "cannot identify S-box:\n{}", res.get_error().get());
                return std::nullopt;
            },
            py::arg("sbox"),
            py::arg("db"),
            R"(
            Try to identify a single S-box of this candidate by matching it against a database of known S-boxes under affine equivalence.

            Tries every assignment of the control inputs that the S-box reads, as the round function computes the S-box for one of them and something else for the others, and the right one is not known in advance. The remaining inputs are held at ``0``.

            Does not annotate the S-box, use ``identify_sboxes`` for that.

            Note that an S-box which simply does not match anything in the database is not an error: in that case an empty string is returned. ``None`` is only returned if the S-box could not be analyzed at all.

            :param hawkeye.SBox sbox: The S-box to identify. Must be one of the S-boxes of this candidate.
            :param hawkeye.SBoxDatabase db: The database of known S-boxes.
            :returns: The name of the matching S-box, or an empty string if no S-box of the database matched. ``None`` on error.
            :rtype: str or None
        )");

        py_hawkeye_cipher_candidate.def(
            "create_modules",
            [](hawkeye::CipherCandidate& self) -> Module* {
                auto res = self.create_modules();
                if (res.is_ok())
                {
                    return res.get();
                }
                log_error("python_context", "cannot create the modules of the candidate:\n{}", res.get_error().get());
                return nullptr;
            },
            R"(
            Write the candidate back into the netlist as a module hierarchy.

            Creates one module holding the entire candidate, a submodule holding its state register, and one submodule per identified S-box holding its combinational gates.
            S-boxes that were not identified are skipped, as are identified S-boxes that overlap an S-box module already created, since a gate belongs to exactly one module. Every skipped S-box is reported to the log.

            :returns: The module holding the candidate on success, ``None`` otherwise.
            :rtype: hal_py.Module or None
        )");

        py_hawkeye_cipher_candidate.def("get_netlist", &hawkeye::CipherCandidate::get_netlist, R"(
            Get the netlist that the candidate belongs to.

            :returns: The netlist of the candidate.
            :rtype: hal_py.Netlist
        )");

        py_hawkeye_cipher_candidate.def("get_size", &hawkeye::CipherCandidate::get_size, R"(
            Get the size of the candidate, i.e., the width of its state register.

            :returns: The size of the candidate.
            :rtype: int
        )");

        py_hawkeye_cipher_candidate.def("is_round_based", &hawkeye::CipherCandidate::is_round_based, R"(
            Check whether the candidate is round-based, i.e., whether its input and output register are the same.

            :returns: ``True`` if the candidate is round-based, ``False`` if it is pipelined.
            :rtype: bool
        )");

        py_hawkeye_cipher_candidate.def("has_round_function", &hawkeye::CipherCandidate::has_round_function, R"(
            Check whether the round function of the candidate has been computed, see ``build_round_function``.

            :returns: ``True`` if the round function has been computed, ``False`` otherwise.
            :rtype: bool
        )");

        py_hawkeye_cipher_candidate.def("get_input_reg", &hawkeye::CipherCandidate::get_input_reg, R"(
            Get the input register of the candidate, ordered by gate ID.

            :returns: The input register of the candidate.
            :rtype: list[hal_py.Gate]
        )");

        py_hawkeye_cipher_candidate.def("get_output_reg", &hawkeye::CipherCandidate::get_output_reg, R"(
            Get the output register of the candidate, ordered by gate ID. Equal to the input register for a round-based candidate.

            :returns: The output register of the candidate.
            :rtype: list[hal_py.Gate]
        )");

        py_hawkeye_cipher_candidate.def("get_round_logic", &hawkeye::CipherCandidate::get_round_logic, R"(
            Get the combinational logic computing the next state, ordered by gate ID.

            :returns: The round function of the candidate, empty if it has not been computed yet.
            :rtype: list[hal_py.Gate]
        )");

        py_hawkeye_cipher_candidate.def("get_gates", &hawkeye::CipherCandidate::get_gates, R"(
            Get all gates of the candidate, i.e., its registers together with its round function, ordered by gate ID.

            :returns: The gates of the candidate.
            :rtype: list[hal_py.Gate]
        )");

        py_hawkeye_cipher_candidate.def("get_sboxes", &hawkeye::CipherCandidate::get_sboxes, py::return_value_policy::reference_internal, R"(
            Get the S-boxes located within the round function of the candidate.

            :returns: The S-boxes of the candidate, empty if they have not been located yet.
            :rtype: list[hawkeye.SBox]
        )");

        py_hawkeye_cipher_candidate.def(
            "get_graph",
            [](const hawkeye::CipherCandidate& self) -> graph_algorithm::NetlistGraph* {
                // The graph is a type of the graph_algorithm plugin, and pybind11 can only hand a type to Python once
                // the module defining it has been imported. Import it here, where the type is about to be handed over,
                // rather than at module initialization: importing a sibling extension module while this one is still
                // initializing changes the order in which the two libraries are torn down, which aborts the interpreter
                // at exit on some platforms. The plugin links against graph_algorithm, so this only fails if this
                // module is imported outside of the hal_plugins package, in which case get_graph is the only thing that
                // stops working.
                py::module_::import("hal_plugins.graph_algorithm");
                return self.get_graph();
            },
            py::return_value_policy::reference_internal,
            R"(
            Get the graph of the round function, in which the gates of the state register are represented by a primary and a shadow vertex so that the feedback of a round-based candidate does not close a cycle.

            :returns: The graph of the round function, ``None`` if the round function has not been computed yet.
            :rtype: graph_algorithm.NetlistGraph or None
        )");

        py_hawkeye_cipher_candidate.def("get_state_inputs", &hawkeye::CipherCandidate::get_state_inputs, R"(
            Get the state inputs of the round function.

            :returns: The state inputs of the candidate.
            :rtype: set[hal_py.Net]
        )");

        py_hawkeye_cipher_candidate.def("get_control_inputs", &hawkeye::CipherCandidate::get_control_inputs, R"(
            Get the control inputs of the round function.

            :returns: The control inputs of the candidate.
            :rtype: set[hal_py.Net]
        )");

        py_hawkeye_cipher_candidate.def("get_other_inputs", &hawkeye::CipherCandidate::get_other_inputs, R"(
            Get the remaining inputs of the round function.

            :returns: The other inputs of the candidate.
            :rtype: set[hal_py.Net]
        )");

        py_hawkeye_cipher_candidate.def("get_state_outputs", &hawkeye::CipherCandidate::get_state_outputs, R"(
            Get the state outputs of the round function.

            :returns: The state outputs of the candidate.
            :rtype: set[hal_py.Net]
        )");

        py_hawkeye_cipher_candidate.def("get_input_ffs_of_gate", &hawkeye::CipherCandidate::get_input_ffs_of_gate, R"(
            Get a dict from each gate of the round function to the input flip-flops it depends on.

            :returns: A dict from gates to sets of input flip-flops.
            :rtype: dict[hal_py.Gate,set[hal_py.Gate]]
        )");

        py_hawkeye_cipher_candidate.def("get_longest_distance_to_gate", &hawkeye::CipherCandidate::get_longest_distance_to_gate, R"(
            Get a dict from a distance to all gates reachable within at most that distance from any input flip-flop.

            :returns: A dict from longest distance to a set of gates.
            :rtype: dict[int,set[hal_py.Gate]]
        )");

        m.def(
            "identify_sbox",
            [](const std::vector<BooleanFunction>& output_functions, const hawkeye::SBoxDatabase& db) -> std::optional<std::string> {
                auto res = hawkeye::CipherCandidate::identify_sbox(output_functions, db);
                if (res.is_ok())
                {
                    return res.get();
                }
                log_error("python_context", "cannot identify S-box:\n{}", res.get_error().get());
                return std::nullopt;
            },
            py::arg("output_functions"),
            py::arg("db"),
            R"(
            Try to identify an S-box given as one Boolean function per output bit by matching it against a database of known S-boxes under affine equivalence.

            Evaluates the Boolean functions into a truth table and looks that up in the database, so it works on functions that do not come from a netlist at all.
            Every variable occurring in them is taken as an input bit of the S-box, so substitute anything that is not one beforehand.

            Note that an S-box which simply does not match anything in the database is not an error: in that case an empty string is returned. ``None`` is only returned if the S-box could not be analyzed at all.

            :param list[hal_py.BooleanFunction] output_functions: The Boolean functions of the S-box, one per output bit. Their order does not matter, as affine equivalence absorbs a permutation of the output bits.
            :param hawkeye.SBoxDatabase db: The database of known S-boxes.
            :returns: The name of the matching S-box, or an empty string if no S-box of the database matched. ``None`` on error.
            :rtype: str or None
        )");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
