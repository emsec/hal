#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void program_options_init(py::module& m)
    {
        py::class_<ProgramArguments> py_program_arguments(m, "ProgramArguments", R"(
            Holds the parsed command line arguments of the program.
        )");

        py_program_arguments.def(py::init<>(), R"(
            Construct an empty set of program arguments.
        )");

        // get_original_arguments is deliberately not exposed: ProgramArguments stores the argv pointer
        // without taking ownership of the strings, which no longer exist once parse() has returned to Python.

        py_program_arguments.def("get_set_options", &ProgramArguments::get_set_options, R"(
            Get all options that are set.

            :returns: A list of the flags of all options that are set.
            :rtype: list[str]
        )");

        py_program_arguments.def("is_option_set", &ProgramArguments::is_option_set, py::arg("flag"), R"(
            Check whether an option is set.

            :param str flag: A flag of the option.
            :returns: ``True`` if the option is set, ``False`` otherwise.
            :rtype: bool
        )");

        py_program_arguments.def("set_option",
                                 py::overload_cast<const std::string&, const std::string&>(&ProgramArguments::set_option),
                                 py::arg("flag"),
                                 py::arg("parameter"),
                                 R"(
            Set an option with a single parameter.

            :param str flag: The flag of the option.
            :param str parameter: The parameter of the option.
        )");

        py_program_arguments.def("set_option",
                                 py::overload_cast<const std::string&, const std::vector<std::string>&>(&ProgramArguments::set_option),
                                 py::arg("flag"),
                                 py::arg("parameters"),
                                 R"(
            Set an option with multiple parameters.

            :param str flag: The flag of the option.
            :param list[str] parameters: The parameters of the option.
        )");

        py_program_arguments.def("set_option",
                                 py::overload_cast<const std::string&, const std::set<std::string>&, const std::vector<std::string>&>(&ProgramArguments::set_option),
                                 py::arg("flag"),
                                 py::arg("equivalent_flags"),
                                 py::arg("parameters"),
                                 R"(
            Set an option that can be addressed through any of several equivalent flags.

            :param str flag: The flag of the option.
            :param set[str] equivalent_flags: All flags that address the same option.
            :param list[str] parameters: The parameters of the option.
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        py_program_arguments.def("get_parameter", &ProgramArguments::get_parameter, py::arg("flag"), R"(
            Get the first parameter of an option.

            :param str flag: A flag of the option.
            :returns: The parameter, or an empty string if the option is not set.
            :rtype: str
        )");

        py_program_arguments.def("get_parameters", &ProgramArguments::get_parameters, py::arg("flag"), R"(
            Get all parameters of an option.

            :param str flag: A flag of the option.
            :returns: A list of parameters, or an empty list if the option is not set.
            :rtype: list[str]
        )");

        py::class_<ProgramOptions> py_program_options(m, "ProgramOptions", R"(
            Holds the command line options that can be configured from the command line or at runtime using ProgramArguments.
        )");

        py_program_options.def_readonly_static("A_REQUIRED_PARAMETER", &ProgramOptions::A_REQUIRED_PARAMETER, R"(
            Constant to specify that a parameter is required and does not have a default value.

            :type: str
        )");

        py_program_options.def(py::init<const std::string&>(), py::arg("name") = std::string(), R"(
            Construct a set of program options.

            :param str name: The name of this group of program options, used for grouping in ``get_options_string``. Defaults to an empty string.
        )");

        py_program_options.def(
            "parse",
            [](ProgramOptions& self, const std::vector<std::string>& args) -> ProgramArguments {
                std::vector<const char*> argv;
                argv.reserve(args.size());
                for (const auto& arg : args)
                {
                    argv.push_back(arg.c_str());
                }
                return self.parse((int)argv.size(), argv.data());
            },
            py::arg("args"),
            R"(
            Parse the given command line arguments into the internal structure.
            As on the command line, the first entry is expected to be the name of the program.

            :param list[str] args: The arguments.
            :returns: The parsed arguments. The original arguments are not retained.
            :rtype: hal_py.ProgramArguments
        )");

        py_program_options.def("get_unknown_arguments", &ProgramOptions::get_unknown_arguments, R"(
            Get the command line arguments that could not be parsed.
            Only valid after ``parse`` was called.

            :returns: A list of all arguments that could not be parsed.
            :rtype: list[str]
        )");

        py_program_options.def("is_registered", &ProgramOptions::is_registered, py::arg("flag"), R"(
            Check whether a flag is already registered for an option.
            No flag can be registered twice.

            :param str flag: The flag to check.
            :returns: ``True`` if the flag is already registered, ``False`` otherwise.
            :rtype: bool
        )");

        py_program_options.def("add",
                               [](ProgramOptions& self, const std::string& flag, const std::string& description, const std::vector<std::string>& parameters) { return self.add_flags({flag}, description, parameters); },
                               py::arg("flag"),
                               py::arg("description"),
                               py::arg("parameters") = std::vector<std::string>(),
                               R"(
            Add a new option with a single flag.
            The length of ``parameters`` is the number of parameters this option takes, its entries are the default values of these parameters.
            Use ``hal_py.ProgramOptions.A_REQUIRED_PARAMETER`` to mark a parameter as required.

            :param str flag: The flag activating the option.
            :param str description: A description of the option.
            :param list[str] parameters: A list of default values for all parameters. Defaults to an empty list.
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        py_program_options.def("add",
                               &ProgramOptions::add_flags,
                               py::arg("flags"),
                               py::arg("description"),
                               py::arg("parameters") = std::vector<std::string>(),
                               R"(
            Add a new option with multiple flags.
            The length of ``parameters`` is the number of parameters this option takes, its entries are the default values of these parameters.
            Use ``hal_py.ProgramOptions.A_REQUIRED_PARAMETER`` to mark a parameter as required.

            :param list[str] flags: The flags activating the option.
            :param str description: A description of the option.
            :param list[str] parameters: A list of default values for all parameters. Defaults to an empty list.
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        py_program_options.def("add", py::overload_cast<const ProgramOptions&, const std::string&>(&ProgramOptions::add), py::arg("other_options"), py::arg("category") = std::string(), R"(
            Add another set of options.

            :param hal_py.ProgramOptions other_options: The set of options to add.
            :param str category: A category for the added options, used for grouping in ``get_options_string``. Defaults to an empty string.
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        py_program_options.def("remove", &ProgramOptions::remove, py::arg("flag"), R"(
            Remove a single flag.
            If multiple flags for an option exist, the others will still remain available.

            :param str flag: The flag activating the option.
            :returns: ``True`` if the option was found, ``False`` otherwise.
            :rtype: bool
        )");

        py_program_options.def("get_options_string", &ProgramOptions::get_options_string, R"(
            Get a formatted string of all options and their description, including the categories of added options.
            Useful for usage messages.

            :returns: The formatted string.
            :rtype: str
        )");

        py_program_options.def("get_options", &ProgramOptions::get_options, R"(
            Get the flags and the description of all options.

            :returns: A list of tuples comprising the set of all flags of the option and the description of the option.
            :rtype: list[tuple(set[str],str)]
        )");
    }
}    // namespace hal
