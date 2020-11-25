#include "python_shell/plugin_python_shell.h"

#include "hal_core/defines.h"
#include "hal_core/utilities/program_arguments.h"
#include "hal_core/utilities/utils.h"

#include <Python.h>
#include <cstring>
#include <fstream>

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<PluginPythonShell>();
    }

    ProgramOptions PluginPythonShell::get_cli_options() const
    {
        ProgramOptions description;

        description.add("--python", "start python shell");
        description.add("--python-script", "run a python script in HAL. to pass args use --python-args", {ProgramOptions::A_REQUIRED_PARAMETER});
        description.add(
            {"--python-args", "--py-args"}, "supply arguments to the python invocation. to provide multiple arguments use '\"' and separate them with spaces", {ProgramOptions::A_REQUIRED_PARAMETER});

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

    bool PluginPythonShell::exec(ProgramArguments& args)
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
                    return false;
                }
            }
        }

        // initiliaze python shell
        Py_Initialize();

        PySys_SetArgv(argc, argv);

        PyRun_SimpleString("import sys");
        PyRun_SimpleString(std::string("sys.path.append(\"" + utils::get_library_directory().string() + "\")").c_str());
        PyRun_SimpleString("from hal_py import *");
        PyRun_SimpleString("import hal_py");

        // changing cwd not required
        // PyRun_SimpleString("import os");
        // PyRun_SimpleString(("os.chdir(\""+ std::filesystem::current_path().string() +"\")").c_str());

        if (args.is_option_set("--python-script"))
        {
            auto file_path = args.get_parameter("--python-script");
            if (!std::filesystem::exists(file_path) || std::filesystem::is_directory(file_path) || !utils::ends_with(file_path, std::string(".py")))
            {
                log_error(get_name(), "'{}' is not a python script file", file_path);
                return false;
            }

            std::ifstream stream(file_path);
            std::string str;
            stream.seekg(0, std::ios::end);
            str.reserve(stream.tellg());
            stream.seekg(0, std::ios::beg);
            str.assign((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

            PyRun_SimpleString(str.c_str());
        }
        else
        {
            Py_Main(argc, argv);
        }

        Py_Finalize();

        /* cleanup of copied command line interface options */
        for (int i = 0; i < argc; ++i)
        {
            PyMem_RawFree(argv[i]);
        }
        delete[] argv;
        return 0;
    }
}    // namespace hal
