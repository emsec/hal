#include "python_shell/plugin_python_shell.h"

#include "hal_core/utilities/program_arguments.h"
#include "hal_core/utilities/utils.h"
#include "hal_core/def.h"

#include <Python.h>
#include <cstring>

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<PluginPythonShell>();
    }

    ProgramOptions PluginPythonShell::get_cli_options() const
    {
        ProgramOptions description;

        description.add("--python",
                        "start python shell. To run a python script "
                        "use following syntax: --python <python3 "
                        "interpreter options> <file to process> "
                        "<args to pass to python script>");

        return description;
    }

    std::string PluginPythonShell::get_name() const
    {
        return std::string("python shell");
    }

    std::string PluginPythonShell::get_version() const
    {
        return std::string("0.1");
    }

    bool PluginPythonShell::exec(ProgramArguments& args)
    {
        int argc;
        const char** argv;

        args.get_original_arguments(&argc, &argv);

        /* copy command line interface options */
        int argc_new       = argc - 1;
        wchar_t** argv_new = (wchar_t**)std::malloc((argc_new) * sizeof(wchar_t*));

        /* pass all parameters to python shell */
        for (int i = 0; i < argc_new; i++)
        {
            argv_new[i] = (wchar_t*)std::malloc((std::strlen(argv[i + 1]) + 1) * sizeof(wchar_t));
            std::mbstowcs(argv_new[i], argv[i + 1], std::strlen(argv[i + 1]) + 1);
        }

        // initiliaze python shell
        Py_Initialize();
        PyRun_SimpleString("import sys");
        PyRun_SimpleString(std::string("sys.path.append(\"" + utils::get_library_directory().string() + "\")").c_str());
        PyRun_SimpleString("from hal_py import *");
        Py_Main(argc_new, argv_new);
        Py_Finalize();

        /* cleanup of copied command line interface options */
        for (int j = 0; j < argc_new; ++j)
        {
            std::free(argv_new[j]);
        }
        std::free(argv_new);
        return 0;
    }
}    // namespace hal
