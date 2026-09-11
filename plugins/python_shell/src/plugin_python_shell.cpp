#include "python_shell/plugin_python_shell.h"

#include "hal_core/defines.h"
#include "hal_core/utilities/program_arguments.h"
#include "hal_core/utilities/utils.h"

#include <Python.h>
#include <cstring>
#include <fstream>
#include <iostream>

#include "pybind11/embed.h"
#include "pybind11/eval.h"

namespace py = pybind11;

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<PluginPythonShell>();
    }

    ProgramOptions CliExtensionPythonShell::get_cli_options() const
    {
        ProgramOptions description;

        description.add("--python", "start python shell");

        return description;
    }

    std::string PluginPythonShell::get_name() const
    {
        return std::string("HAL Python");
    }

    std::string PluginPythonShell::get_version() const
    {
        return std::string("0.1");
    }

    namespace
    {
        /// Python code that binds `netlist` to the project or netlist named on the command line, or to None.
        std::string netlist_loader(const ProgramArguments& args)
        {
            const auto quote = [](const std::string& text) { return "r'''" + text + "'''"; };

            if (args.is_option_set("--project-dir"))
            {
                return "netlist = hal_py.NetlistFactory.load_hal_project(" + quote(args.get_parameter("--project-dir")) + ")\n";
            }
            if (args.is_option_set("--import-netlist"))
            {
                std::string code = "netlist = hal_py.NetlistFactory.load_netlist(" + quote(args.get_parameter("--import-netlist"));
                if (args.is_option_set("--gate-library"))
                {
                    code += ", " + quote(args.get_parameter("--gate-library"));
                }
                return code + ")\n";
            }
            return "netlist = None\n";
        }

        /// The exit code a script asked for through SystemExit: None is 0, an int is itself, anything else is printed and is 1.
        int exit_code_of(py::error_already_set& e)
        {
            py::object code = e.value().attr("code");
            if (code.is_none())
            {
                return 0;
            }
            if (py::isinstance<py::int_>(code))
            {
                return code.cast<int>();
            }
            std::cerr << py::str(code).cast<std::string>() << std::endl;
            return 1;
        }
    }    // namespace

    int PluginPythonShell::exec(ProgramArguments& args)
    {
        int argc       = 0;
        wchar_t** argv = nullptr;

        // python needs arguments as argc/argv, so we convert them here
        if (args.is_option_set("--python-args"))
        {
            std::vector<std::string> py_args;
            auto py_arg_str = args.get_parameter("--python-args");

            if (py_arg_str.find(' ') != std::string::npos)
            {
                py_args = utils::split(py_arg_str, ' ');
            }
            else
            {
                py_args.push_back(py_arg_str);
            }

            /* copy command line interface options */
            argc = py_args.size();
            argv = new wchar_t*[argc];

            /* pass all parameters to python shell */
            for (int i = 0; i < argc; i++)
            {
                argv[i] = Py_DecodeLocale(py_args[i].c_str(), nullptr);
                if (argv[i] == nullptr)
                {
                    log_error(get_name(), "unable to convert argument '{}' for Python", py_args[i]);
                    return 1;
                }
            }
        }

        std::string script_path;
        if (args.is_option_set("--python-script"))
        {
            script_path = args.get_parameter("--python-script");
            if (!std::filesystem::exists(script_path) || std::filesystem::is_directory(script_path) || !utils::ends_with(script_path, std::string(".py")))
            {
                log_error(get_name(), "'{}' is not a python script file", script_path);
                return 1;
            }
        }

        // initiliaze python shell
        Py_Initialize();

        PySys_SetArgv(argc, argv);

        int exit_code = 0;
        {
            // the scope ends before Py_Finalize, so that no Python object outlives the interpreter
            py::dict globals = py::module_::import("__main__").attr("__dict__");
            try
            {
                py::exec("import sys\n"
                         "sys.path.append(r'''" + utils::get_library_directory().string() + "''')\n"
                         "from hal_py import *\n"
                         "import hal_py\n"
                         + netlist_loader(args),
                         globals);

                if (script_path.empty())
                {
                    // interactive shell; Py_Main takes over stdin until the user leaves it
                    Py_Main(argc, argv);
                }
                else if (globals["netlist"].is_none() && (args.is_option_set("--project-dir") || args.is_option_set("--import-netlist")))
                {
                    log_error(get_name(), "could not load the netlist, the script is not run");
                    exit_code = 1;
                }
                else
                {
                    py::eval_file(script_path, globals);
                }
            }
            catch (py::error_already_set& e)
            {
                if (e.matches(PyExc_SystemExit))
                {
                    exit_code = exit_code_of(e);
                }
                else
                {
                    e.restore();
                    PyErr_Print();
                    exit_code = 1;
                }
            }
        }

        Py_Finalize();

        /* cleanup of copied command line interface options */
        for (int i = 0; i < argc; ++i)
        {
            PyMem_RawFree(argv[i]);
        }
        delete[] argv;
        return exit_code;
    }
}    // namespace hal
