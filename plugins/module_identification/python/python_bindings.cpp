#include "hal_core/python_bindings/python_bindings.h"

#include "hal_core/netlist/boolean_function.h"
#include "module_identification/api/configuration.h"
#include "module_identification/api/module_identification.h"
#include "module_identification/api/result.h"
#include "module_identification/plugin_module_identification.h"
#include "module_identification/types/architecture_types.h"
#include "module_identification/types/candidate_types.h"
#include "module_identification/types/multithreading_types.h"

namespace py = pybind11;
namespace hal
{
    namespace module_identification
    {
#ifdef PYBIND11_MODULE
        PYBIND11_MODULE(hawkeye, m)
        {
            m.doc() = "Plugin for module classification against a library of predefined types.";
#else
        PYBIND11_PLUGIN(hawkeye)
        {
            py::module m("hawkeye", "Plugin for module classification against a library of predefined types.");
#endif    // ifdef PYBIND11_MODULE

            py::class_<ModuleIdentificationPlugin, RawPtrWrapper<ModuleIdentificationPlugin>, BasePluginInterface> py_module_identification_plugin(
                m, "ModuleIdentificationPlugin", R"(This class provides an interface to integrate the module identification tool as a plugin within the HAL framework.)");

            py_module_identification_plugin.def_property_readonly("name", &ModuleIdentificationPlugin::get_name, R"(
                The name of the plugin.

                :type: str
            )");

            py_module_identification_plugin.def("get_name", &ModuleIdentificationPlugin::get_name, R"(
                Get the name of the plugin.

                :returns: The name of the plugin.
                :rtype: str
            )");

            py_module_identification_plugin.def_property_readonly("version", &ModuleIdentificationPlugin::get_version, R"(
                The version of the plugin.

                :type: str
            )");

            py_module_identification_plugin.def("get_version", &ModuleIdentificationPlugin::get_version, R"(
                Get the version of the plugin.

                :returns: The version of the plugin.
                :rtype: str
            )");

            py_module_identification_plugin.def_property_readonly("description", &ModuleIdentificationPlugin::get_description, R"(
                The description of the plugin.

                :type: str
            )");

            py_module_identification_plugin.def("get_description", &ModuleIdentificationPlugin::get_description, R"(
                Get the description of the plugin.

                :returns: The description of the plugin.
                :rtype: str
            )");

            py_module_identification_plugin.def_property_readonly("dependencies", &ModuleIdentificationPlugin::get_dependencies, R"(
                A set of plugin names that this plugin depends on.

                :type: set[str]
            )");

            py_module_identification_plugin.def("get_dependencies", &ModuleIdentificationPlugin::get_dependencies, R"(
                Get a set of plugin names that this plugin depends on.

                :returns: A set of plugin names that this plugin depends on.
                :rtype: set[str]
            )");

            // Define Configuration class
            py::class_<Configuration> py_configuration(m, "Configuration", R"(
                Configuration for the module identification analysis.
                This struct holds important parameters that configure the module identification analysis, including netlist to analyze, known registers, candidate types to check, threading options, etc.
            )");

            py_configuration.def(py::init<hal::Netlist*>(), py::arg("nl"), R"(
                Constructs a new ModuleIdentification analysis configuration for the given netlist.
                
                :param hal_py.Netlist nl: The netlist to be analyzed.
            )");

            py_configuration.def(py::init<>(), R"(
                Constructs an empty configuration.
            )");

            py_configuration.def_readwrite("netlist", &Configuration::m_netlist, R"(
                The netlist to be analyzed.
                
                :type: hal_py.Netlist
            )");

            py_configuration.def_readwrite("known_registers", &Configuration::m_known_registers, R"(
                A vector handling possibly known registers.
                
                :type: list[list[hal_py.Gate]]
            )");

            py_configuration.def_readwrite("types_to_check", &Configuration::m_types_to_check, R"(
                CandidateTypes that shall be checked. Defaults to all checkable candidate types.
                
                :type: list[module_identification.CandidateType]
            )");

            py_configuration.def_readwrite("max_thread_count", &Configuration::m_max_thread_count, R"(
                Maximum number of concurrent threads created during execution. Defaults to 1.
                
                :type: int
            )");

            py_configuration.def_readwrite("max_control_signals", &Configuration::m_max_control_signals, R"(
                Maximum number of control signals to be tested. Defaults to 3.
                
                :type: int
            )");

            py_configuration.def_readwrite("already_classified_candidates", &Configuration::m_already_classified_candidates, R"(
                Gates to ignore during processing.
                
                :type: list[list[hal_py.Gate]]
            )");

            py_configuration.def_readwrite("blocked_base_candidates", &Configuration::m_blocked_base_candidates, R"(
                Base candidates to block during analysis.
                
                :type: list[set[hal_py.Gate]]
            )");

            py_configuration.def_readwrite("multithreading_priority", &Configuration::m_multithreading_priority, R"(
                Choose which `MultithreadingPriority` to use for the analysis. Defaults to memory priority
                
                :type: module_identification.MultithreadingPriority
            )");

            py_configuration.def("with_known_registers", &Configuration::with_known_registers, py::arg("registers"), R"(
                Set the known registers for prioritization.

                :param list[list[hal_py.Gate]] registers: The groups provided by a dana run.
                :returns: The updated module identification configuration.
                :rtype: module_identification.Configuration
            )");

            py_configuration.def("with_max_thread_count", &Configuration::with_max_thread_count, py::arg("max_thread_count"), R"(
                Set the maximum number of threads.

                :param int max_thread_count: The number of threads to be started at max.
                :returns: The updated module identification configuration.
                :rtype: module_identification.Configuration
            )");

            py_configuration.def("with_max_control_signals", &Configuration::with_max_control_signals, py::arg("max_control_signals"), R"(
                Set the maximum number of control signals to be tested.

                :param int max_control_signals: The number of control signals checked.
                :returns: The updated module identification configuration.
                :rtype: module_identification.Configuration
            )");

            py_configuration.def("with_multithreading_priority", &Configuration::with_multithreading_priority, py::arg("priority"), R"(
                Set the multithreading priority type.

                :param module_identification.MultithreadingPriority priority: The type of multithreading used during execution.
                :returns: The updated module identification configuration.
                :rtype: module_identification.Configuration
            )");

            py_configuration.def("with_types_to_check", &Configuration::with_types_to_check, py::arg("types_to_check"), R"(
                Set the candidate types to be checked.

                :param list[module_identification.CandidateType] types_to_check: A list of candidate types to be checked for.
                :returns: The updated module identification configuration.
                :rtype: module_identification.Configuration
            )");

            py_configuration.def("with_already_classified_candidates", &Configuration::with_already_classified_candidates, py::arg("already_classified_candidates"), R"(
                Add gates to be ignored during processing.

                All candidates that are build during the module identification run that contain any gates that overlap with any already classified candidate are discarded to avoid conflicts.

                :param list[list[hal_py.Gate]] already_classified_candidates: Candidates to be ignored.
                :returns: The updated module identification configuration.
                :rtype: module_identification.Configuration
            )");

            py_configuration.def("with_blocked_base_candidates", &Configuration::with_blocked_base_candidates, py::arg("blocked_base_candidates"), R"(
                Add base candidates to be blocked during analysis.

                :param list[set[hal_py.Gate]] blocked_base_candidates: Base candidates to be ignored.
                :returns: The updated module identification configuration.
                :rtype: module_identification.Configuration
            )");

            // Define Result class
            py::class_<Result> py_result(m, "Result", R"(
                The result of a module identification run containing the candidates.
            )");

            py_result.def(py::init<hal::Netlist*, const std::vector<std::pair<BaseCandidate, VerifiedCandidate>>&, const std::string&>(),
                          py::arg("nl"),
                          py::arg("result"),
                          py::arg("timing_stats_json") = "",
                          R"(
                Constructor for `Result`.

                :param hal_py.Netlist nl: The netlist on which module identification has been performed.
                :param list[tuple(module_identification.BaseCandidate, module_identification.VerifiedCandidate)] result: A vector of pairs containing base candidates and their verified candidates.
                :param str timing_stats_json: A JSON string containing timing statistics. Defaults to an empty string.
            )");

            py_result.def("get_netlist", &Result::get_netlist, R"(
                Get the netlist on which module identification has been performed.

                :returns: The netlist.
                :rtype: hal_py.Netlist
            )");

            py_result.def("get_verified_candidate_gates", &Result::get_verified_candidate_gates, R"(
                Get a map of the candidate IDs to the gates contained inside the verified candidates.

                This map only contains verified candidates that are fully verified.
                The ID is only unique for this result.

                :returns: A map of candidate IDs to a vector of gates.
                :rtype: dict[int, list[hal_py.Gate]]
            )");

            py_result.def("get_verified_candidates", &Result::get_verified_candidates, R"(
                Get a map of the candidate IDs to the verified candidates.

                This map only contains verified candidates that are fully verified.
                The ID is only unique for this result.

                :returns: A map of candidate IDs to verified candidates.
                :rtype: dict[int, module_identification.VerifiedCandidate]
            )");

            py_result.def("get_candidate_gates", &Result::get_candidate_gates, R"(
                Get a map of the candidate IDs to the gates contained inside the candidate.

                This map contains all checked candidates, even the ones not verified.
                The ID is only unique for this result.

                :returns: A map of candidate IDs to a vector of gates.
                :rtype: dict[int, list[hal_py.Gate]]
            )");

            py_result.def("get_candidates", &Result::get_candidates, R"(
                Get a map of the candidate IDs to the candidates.

                This map contains all checked candidates, even the ones not verified.
                The ID is only unique for this result.

                :returns: A map of candidate IDs to candidates.
                :rtype: dict[int, module_identification.VerifiedCandidate]
            )");

            py_result.def("get_candidate_gates_by_id", &Result::get_candidate_gates_by_id, py::arg("id"), R"(
                Get the gates of the candidate with the corresponding ID.

                :param int id: The ID of the requested candidate.
                :returns: The gates of the candidate on success.
                :rtype: list[hal_py.Gate]
            )");

            py_result.def("get_candidate_by_id", &Result::get_candidate_by_id, py::arg("id"), R"(
                Get the candidate with the corresponding ID.

                :param int id: The ID of the requested candidate.
                :returns: The verified candidate on success.
                :rtype: module_identification.VerifiedCandidate
            )");

            py_result.def("get_all_gates", &Result::get_all_gates, R"(
                Get all gates contained in any of the candidates.

                :returns: A set of gates.
                :rtype: set[hal_py.Gate]
            )");

            py_result.def("get_all_verified_gates", &Result::get_all_verified_gates, R"(
                Get all gates contained in any of the verified candidates.

                :returns: A set of gates.
                :rtype: set[hal_py.Gate]
            )");

            py_result.def("create_modules_in_netlist", &Result::create_modules_in_netlist, R"(
                Creates a HAL module for each candidate of the result.

                :returns: ``True`` on success, ``False`` otherwise.
                :rtype: bool
            )");

            py_result.def("get_timing_stats", &Result::get_timing_stats, R"(
                Get the collected timing information formatted as a JSON string.

                :returns: A JSON formatted string.
                :rtype: str
            )");

            py_result.def("merge", &Result::merge, py::arg("other"), py::arg("dana_cache"), R"(
                Merges two results by combining the found verified candidates.

                When both results contain a verified candidate for the same base candidate, the better one is chosen via the same post-processing used in the orignal module identificaion process.
                This requires that the base candidates are identical and that all gates of all candidates still exist in the netlist!

                :param module_identification.Result other: Another module identification result that is merged with this one.
                :param list[list[hal_py.Gate]] dana_cache: A list of previously identified register groupings that is used in the post-processing.
                :returns: The merged module identification result on success.
                :rtype: module_identification.Result
            )");

            py_result.def_static("assign_base_candidates_to_iterations", &Result::assign_base_candidates_to_iterations, py::arg("iteration_results"), py::arg("create_block_lists") = false, R"(
                For different runs of the plugin figure out in which iteration the plugin found the highest quality result for each candidate.

                This is used to compare the results of different runs of the plugin and afterwards get a list of base candidates for each execution for which this execution gave the best results.

                :param list[module_identification.Result] iteration_results: A vector of all the execution results.
                :param bool create_block_lists: A parameter to determine whether to create allow or block lists.
                :returns: A vector of allow or block lists for each plugin execution iteration.
                :rtype: list[list[set[hal_py.Gate]]]
            )");

            // Define execute function
            m.def(
                "execute",
                [](const Configuration& config) -> std::optional<Result> {
                    auto res = module_identification::execute(config);
                    if (res.is_ok())
                    {
                        return res.get();
                    }
                    else
                    {
                        log_error("python_context", "error encountered while executing module identification:\n{}", res.get_error().get());
                        return std::nullopt;
                    }
                },
                py::arg("config"),
                R"(
                Perform a full run of the module identification process on the given netlist with the provided configuration.

                This function executes the complete module identification process on the specified netlist. It uses the provided configuration to guide the identification process and returns a result object containing information about all analyzed candidates.

                :param module_identification.Configuration config: The configuration to guide the identification process.
                :returns: The result of the run containing all computed results and options for further processing on success, ``None`` otherwise.
                :rtype: module_identification.Result or None
            )");

            m.def(
                "execute_on_gates",
                [](const std::vector<hal::Gate*>& gates, const Configuration& config) -> std::optional<Result> {
                    auto res = module_identification::execute_on_gates(gates, config);
                    if (res.is_ok())
                    {
                        return res.get();
                    }
                    else
                    {
                        log_error("python_context", "error encountered while executing module identification on gates:\n{}", res.get_error().get());
                        return std::nullopt;
                    }
                },
                py::arg("gates"),
                py::arg("config"),
                R"(
                Perform a module identification run on the specified gates with the provided configuration.

                This function executes the module identification process on a specific set of gates. It uses the provided configuration to guide the identification process and returns a result object containing information about all analyzed candidates.

                :param list[hal_py.Gate] gates: The gates to be analyzed.
                :param module_identification.Configuration config: The configuration to guide the identification process.
                :returns: The result of the run containing all computed results and options for further processing on success, ``None`` otherwise.
                :rtype: module_identification.Result or None
            )");

            // Define Architecture enum
            py::enum_<Architecture> py_architecture(m, "Architecture", R"(
                Defines supported FPGA architectures.
                This enum specifies the FPGA architectures that are supported by the module identification plugin.
            )");

            py_architecture.value("lattice_ice40", module_identification::Architecture::lattice_ice40, R"(Lattice iCE40 FPGA architecture.)")
                .value("xilinx_unisim", module_identification::Architecture::xilinx_unisim, R"(Xilinx Unisim FPGA architecture.)")
                .export_values();

            // Define CandidateType enum
            py::enum_<CandidateType> py_candidate_type(m, "CandidateType", R"(
                Enumeration of the different candidate types for module identification.
                This enum specifies the types of operations that the module identification process can recognize and verify, such as arithmetic operations and comparisons.
            )");

            py_candidate_type.value("adder", CandidateType::adder, R"(Addition operation.)")
                .value("subtraction", CandidateType::subtraction, R"(Subtraction operation.)")
                .value("counter", CandidateType::counter, R"(Counter operation.)")
                .value("negation", CandidateType::negation, R"(Negation operation.)")
                .value("absolute", CandidateType::absolute, R"(Absolute value operation.)")
                .value("constant_multiplication", CandidateType::constant_multiplication, R"(Constant multiplication operation.)")
                .value("equal", CandidateType::equal, R"(Equality comparison.)")
                .value("less_than", CandidateType::less_than, R"(Less-than comparison.)")
                .value("less_equal", CandidateType::less_equal, R"(Less-than-or-equal comparison.)")
                .value("signed_less_than", CandidateType::signed_less_than, R"(Signed less-than comparison.)")
                .value("signed_less_equal", CandidateType::signed_less_equal, R"(Signed less-than-or-equal comparison.)")
                .value("value_check", CandidateType::value_check, R"(Value check against a constant operation.)")
                .value("none", CandidateType::none, R"(No operation.)")
                .value("mixed", CandidateType::mixed, R"(Mixed operation, for merged `VerifiedCandidates` that contain multiple candidate types.)")
                .export_values();

            // Define MultithreadingPriority enum
            py::enum_<MultithreadingPriority> py_multithreading_priority(m, "MultithreadingPriority", R"(
                Specifies the strategy for multithreading in the module identification process.
                This enum class defines the strategies for managing multithreading in the module identification plugin.
                The strategies determine how resources are allocated and prioritized when performing multithreaded operations.
            )");

            py_multithreading_priority
                .value("time_priority", MultithreadingPriority::time_priority, R"(
                    Prioritize time efficiency in multithreading.
                    This option specifies that multithreading should be handled with a priority on time efficiency,
                    aiming to complete tasks as quickly as possible.
                )")
                .value("memory_priority", MultithreadingPriority::memory_priority, R"(
                    Prioritize memory efficiency in multithreading.
                    This option specifies that multithreading should be handled with a priority on memory efficiency,
                    aiming to minimize memory usage even if it results in longer execution times.
                )")
                .export_values();

            // Define WordLevelOperation struct
            py::class_<WordLevelOperation> py_word_level_operation(m, "WordLevelOperation", R"(
                Represents a word-level operation with its operands, control signals, and the operation implemented as a HAL Boolean function.
                This struct is used to store the information related to a word-level operation, which includes the operands, control signals, and the Boolean function representing the operation.
            )");

            py_word_level_operation.def(py::init<>());

            py_word_level_operation.def_readwrite("operands", &WordLevelOperation::operands, R"(
                A map of operand names to their corresponding Boolean functions.
                
                :type: dict[str, hal_py.BooleanFunction]
            )");

            py_word_level_operation.def_readwrite("ctrl_signals", &WordLevelOperation::ctrl_signals, R"(
                A vector of control signals as Boolean functions.
                
                :type: list[hal_py.BooleanFunction]
            )");

            py_word_level_operation.def_readwrite("operation", &WordLevelOperation::operation, R"(
                The Boolean function representing the word-level operation.
                
                :type: hal_py.BooleanFunction
            )");

            // Define VerifiedCandidate class
            py::class_<VerifiedCandidate> py_verified_candidate(m, "VerifiedCandidate", R"(
               This class is used to represent a verified candidate within the module identification process, providing methods for their creation, manipulation, and verification.
            )");

            py_verified_candidate.def_static("merge", &VerifiedCandidate::merge, py::arg("candidates"), R"(
                Merge multiple verified candidates into a single candidate.
                
                This function merges a vector of verified candidates into a single candidate.
                
                :param list[module_identification.VerifiedCandidate] candidates: A vector of verified candidates to merge.
                :returns: The merged verified candidate on success.
                :rtype: module_identification.VerifiedCandidate
            )");

            py_verified_candidate.def("get_candidate_info", &VerifiedCandidate::get_candidate_info, R"(
                Get the candidate information as a string.
                
                :returns: A string containing the candidate information.
                :rtype: str
            )");

            py_verified_candidate.def("is_verified", &VerifiedCandidate::is_verified, R"(
                Check if the candidate is verified.
                
                :returns: `true` if the candidate is verified, `false` otherwise.
                :rtype: bool
            )");

            py_verified_candidate.def("get_name", &VerifiedCandidate::get_name, R"(
                Get the name of the candidate that represents the functionality of the candidate.
                
                :returns: A string containing the name of the candidate.
                :rtype: str
            )");

            py_verified_candidate.def("get_merged_word_level_operation", &VerifiedCandidate::get_merged_word_level_operation, R"(
                Get the merged word-level operation for the candidate.
                
                The mergred word-level operation includes all word-level oprations that we were able to verify for different control mappings of the candidate.
                
                :returns: The merged word-level operation as a Boolean function.
                :rtype: hal_py.BooleanFunction
            )");

            py_verified_candidate.def_readwrite("operands", &VerifiedCandidate::m_operands, R"(
                A vector of operands.
                
                :type: list[list[hal_py.Net]]
            )");

            py_verified_candidate.def_readwrite("output_nets", &VerifiedCandidate::m_output_nets, R"(
                A vector of output nets.
                
                :type: list[hal_py.Net]
            )");

            py_verified_candidate.def_readwrite("control_signals", &VerifiedCandidate::m_control_signals, R"(
                A vector of control signal nets.
                
                :type: list[hal_py.Net]
            )");

            py_verified_candidate.def_readwrite("control_signal_mappings", &VerifiedCandidate::m_control_signal_mappings, R"(
                A vector of all control mappings covered by this candidate.
                
                :type: list[dict[hal_py.Net, hal_py.BooleanFunction.Value]]
            )");

            py_verified_candidate.def_readwrite("word_level_operations", &VerifiedCandidate::m_word_level_operations, R"(
                A map of control signal mappings to their respective word-level operations.
                
                :type: dict[dict[hal_py.Net, hal_py.BooleanFunction.Value], hal_py.BooleanFunction]
            )");

            py_verified_candidate.def_readwrite("gates", &VerifiedCandidate::m_gates, R"(
                A vector of gates associated with the candidate.
                
                :type: list[hal_py.Gate]
            )");

            py_verified_candidate.def_readwrite("base_gates", &VerifiedCandidate::m_base_gates, R"(
                A vector of base gates associated with the candidate.
                
                :type: list[hal_py.Gate]
            )");

            py_verified_candidate.def_readwrite("total_input_nets", &VerifiedCandidate::m_total_input_nets, R"(
                A vector of all input nets to the gate subgraph, regardless of whether they appear in a word-level operation or not.
                
                :type: list[hal_py.Net]
            )");

            py_verified_candidate.def_readwrite("total_output_nets", &VerifiedCandidate::m_total_output_nets, R"(
                A vector of all output nets of the subgraph, regardless of whether they appear in a word-level operation or not.
                
                :type: list[hal_py.Net]
            )");

            py_verified_candidate.def_readwrite("types", &VerifiedCandidate::m_types, R"(
                The set of contained candidate types.
                
                :type: set[module_identification.CandidateType]
            )");

            py_verified_candidate.def_readwrite("verified", &VerifiedCandidate::m_verified, R"(
                Indicates whether the candidate is verified.
                
                :type: bool
            )");
        }
    }    // namespace module_identification
}    // namespace hal
