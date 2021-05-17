#include "hal_core/defines.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/netlist/netlist_parser/netlist_parser_manager.h"
#include "hal_core/netlist/netlist_writer/netlist_writer_manager.h"
#include "hal_core/netlist/persistent/netlist_serializer.h"
#include "hal_core/plugin_system/plugin_interface_base.h"
#include "hal_core/plugin_system/plugin_interface_cli.h"
#include "hal_core/plugin_system/plugin_interface_ui.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/program_arguments.h"
#include "hal_core/utilities/program_options.h"
#include "hal_core/utilities/utils.h"
#include "hal_version.h"

#include <fstream>
#include <iostream>
#include <string>

#define SUCCESS 0
#define ERROR 1

using namespace hal;

int cleanup(int return_code = SUCCESS)
{
    if (!plugin_manager::unload_all_plugins())
    {
        return ERROR;
    }
    return return_code;
}

void initialize_cli_options(ProgramOptions& cli_options)
{
    ProgramOptions generic_options("generic options");
    /* initialize generic options */
    generic_options.add({"-h", "--help"}, "print help messages");
    generic_options.add({"-v", "--version"}, "displays the current version");
    generic_options.add({"-L", "--show-log-options"}, "show all logging options");
    generic_options.add({"-l", "--logfile"}, "specify log file name", {""});
    generic_options.add({"--log-time"}, "includes time information into the log");
    generic_options.add({"--licenses"}, "Shows the licenses of projects used by HAL");

    generic_options.add({"-i", "--input-file"}, "input file", {ProgramOptions::A_REQUIRED_PARAMETER});
    generic_options.add({"-gl", "--gate-library"}, "used gate-library of the netlist", {ProgramOptions::A_REQUIRED_PARAMETER});
    generic_options.add({"-e", "--empty-netlist"}, "create a new empty netlist, requires a gate library to be specified");
    generic_options.add("--volatile-mode", "[cli only] prevents hal from creating a .hal progress file (e.g. cluster use)");
    generic_options.add("--no-log", "prevents hal from creating a .log file");

    /* initialize netlist parser options */
    generic_options.add(netlist_parser_manager::get_cli_options());

    /* initialize netlist writer options */
    generic_options.add(netlist_writer_manager::get_cli_options());
    cli_options.add(generic_options);
}

int main(int argc, const char* argv[])
{
    /* initialize and parse generic cli options */
    ProgramOptions cli_options("cli options");
    initialize_cli_options(cli_options);
    ProgramOptions all_options("all options");
    all_options.add(cli_options);
    all_options.add(LogManager::get_instance().get_option_descriptions());
    ProgramArguments args = all_options.parse(argc, argv);

    /* initialize logging */
    LogManager& lm = LogManager::get_instance();

    lm.add_channel("core", {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
    lm.add_channel("gate_library_parser", {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
    lm.add_channel("gate_library_writer", {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
    lm.add_channel("gate_library_manager", {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
    lm.add_channel("gate_library", {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
    lm.add_channel("netlist", {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
    lm.add_channel("netlist_utils", {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
    lm.add_channel("netlist_internal", {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
    lm.add_channel("netlist_persistent", {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
    lm.add_channel("gate", {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
    lm.add_channel("net", {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
    lm.add_channel("module", {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
    lm.add_channel("grouping", {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");

    lm.add_channel("netlist_parser", {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
    lm.add_channel("hdl_writer", {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
    lm.add_channel("python_context", {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
    lm.add_channel("event", {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");

    if (args.is_option_set("--logfile"))
    {
        lm.set_file_name(std::filesystem::path(args.get_parameter("--logfile")));
    }
    lm.handle_options(args);

    if (args.is_option_set("--log-time"))
    {
        lm.set_format_pattern("[%d.%m.%Y %H:%M:%S] [%n] [%l] %v");
    }

    /* initialize plugin manager */
    plugin_manager::add_existing_options_description(cli_options);

    // suppress output
    if (args.is_option_set("--help") || args.is_option_set("--licenses") || args.is_option_set("--version") || argc == 1)
    {
        lm.deactivate_all_channels();
    }

    if (!plugin_manager::load_all_plugins())
    {
        return cleanup(ERROR);
    }

    /* add plugin cli options */
    auto options = plugin_manager::get_cli_plugin_options();
    if (!options.get_options().empty())
    {
        cli_options.add(plugin_manager::get_cli_plugin_options());
        all_options.add(plugin_manager::get_cli_plugin_options());
    }

    /* parse program options */
    bool unknown_option_exists = false;
    args                       = all_options.parse(argc, argv);
    // Check for unknown options include log manager options --> use all_options
    for (const auto& opt : all_options.get_unknown_arguments())
    {
        unknown_option_exists = true;
        log_error("core", "unkown command line argument '{}'", opt);
    }

    /* process help output */
    if (args.is_option_set("--help") || args.get_set_options().size() == 0 || unknown_option_exists)
    {
        std::cout << cli_options.get_options_string() << std::endl;
        return cleanup(unknown_option_exists ? ERROR : SUCCESS);
    }

    if (args.is_option_set("--version"))
    {
        std::cout << hal_version::version << std::endl;
        return cleanup();
    }

    if (args.is_option_set("--licenses"))
    {
        std::cout << utils::get_open_source_licenses() << std::endl;
        return cleanup();
    }

    /* redirect control to ui plugin if enabled */
    {
        std::vector<std::string> plugins_to_execute;
        auto ui_plugin_flags = plugin_manager::get_ui_plugin_flags();
        for (const auto& option : args.get_set_options())
        {
            auto it = ui_plugin_flags.find(option);
            if (it != ui_plugin_flags.end())
            {
                if (std::find(plugins_to_execute.begin(), plugins_to_execute.end(), it->second) == plugins_to_execute.end())
                {
                    plugins_to_execute.push_back(it->second);
                }
            }
        }

        if (plugins_to_execute.size() > 1)
        {
            log_error("core", "passed options for multiple ui plugins: {}", utils::join(", ", plugins_to_execute));
            return cleanup(ERROR);
        }
        else if (plugins_to_execute.size() == 1)
        {
            auto plugin_name = plugins_to_execute[0];
            auto plugin      = plugin_manager::get_plugin_instance<UIPluginInterface>(plugin_name);
            if (plugin == nullptr)
            {
                return cleanup(ERROR);
            }

            ProgramArguments plugin_args;

            for (const auto& option : plugin->get_cli_options().get_options())
            {
                auto flags      = std::get<0>(option);
                auto first_flag = *flags.begin();
                if (args.is_option_set(first_flag))
                {
                    plugin_args.set_option(first_flag, flags, args.get_parameters(first_flag));
                }
            }

            log_info("core", "executing '{}' with", plugin_name);
            for (const auto& option : plugin_args.get_set_options())
            {
                log_info("core", "  '{}': {}", option, utils::join(",", plugin_args.get_parameters(option)));
            }

            /* add timestamp to log output */
            LogManager::get_instance().set_format_pattern("[%d.%m.%Y %H:%M:%S] [%n] [%l] %v");

            auto ret = plugin->exec(args);

            return cleanup(ret ? ERROR : SUCCESS);
        }
    }

    /**
     * control for command line interface
     */

    if (args.is_option_set("--show-log-options"))
    {
        std::cout << lm.get_option_descriptions().get_options_string() << std::endl;
        return cleanup();
    }

    /* handle input file */
    if (args.is_option_set("--empty-netlist") && args.is_option_set("--input-file"))
    {
        log_error("core", "Found --empty-netlist and --input-file!");
        return cleanup();
    }

    if (args.is_option_set("--empty-netlist") && !args.is_option_set("--gate-library"))
    {
        log_error("core", "Found --empty-netlist but --gate-library is missing!");
        return cleanup();
    }

    std::filesystem::path file_name;

    if (args.is_option_set("--empty-netlist"))
    {
        file_name = std::filesystem::path("./empty_netlist.hal");
    }
    else
    {
        file_name = std::filesystem::path(args.get_parameter("--input-file"));
    }

    if (args.is_option_set("--no-log"))
    {
        log_warning("core",
                    "the console output will not be written to a local log "
                    "file (--no-log).");
    }
    else if (!args.is_option_set("--logfile"))
    {
        auto log_path = file_name;
        lm.set_file_name(log_path.replace_extension(".log"));
    }

    std::unique_ptr<Netlist> netlist;

    if (args.is_option_set("--empty-netlist"))
    {
        auto lib = gate_library_manager::load(args.get_parameter("--gate-library"));
        netlist  = netlist_factory::create_netlist(lib);
    }
    else
    {
        netlist = netlist_factory::load_netlist(args);
    }

    if (netlist == nullptr)
    {
        return cleanup(ERROR);
    }

    bool volatile_mode = false;
    if (args.is_option_set("--volatile-mode"))
    {
        volatile_mode = true;
        log_warning("core", "your modifications will not be written to a .hal file (--volatile-mode).");
    }

    /* cli plugins */
    std::vector<std::string> plugins_to_execute;
    auto option_to_plugin_name = plugin_manager::get_cli_plugin_flags();
    for (const auto& option : args.get_set_options())
    {
        auto it = option_to_plugin_name.find(option);
        if (it != option_to_plugin_name.end())
        {
            if (std::find(plugins_to_execute.begin(), plugins_to_execute.end(), it->second) == plugins_to_execute.end())
            {
                plugins_to_execute.push_back(it->second);
            }
        }
    }

    bool plugins_successful = true;
    for (const auto& plugin_name : plugins_to_execute)
    {
        auto plugin = plugin_manager::get_plugin_instance<CLIPluginInterface>(plugin_name);
        if (plugin == nullptr)
        {
            return cleanup(ERROR);
        }

        ProgramArguments plugin_args;

        for (const auto& option : plugin->get_cli_options().get_options())
        {
            auto flags      = std::get<0>(option);
            auto first_flag = *flags.begin();
            if (args.is_option_set(first_flag))
            {
                plugin_args.set_option(first_flag, flags, args.get_parameters(first_flag));
            }
        }

        log_info("core", "executing '{}' with", plugin_name);
        for (const auto& option : plugin_args.get_set_options())
        {
            log_info("core", "  '{}': {}", option, utils::join(",", plugin_args.get_parameters(option)));
        }

        if (!plugin->handle_cli_call(netlist.get(), plugin_args))
        {
            plugins_successful = false;
            break;
        }
    }

    if (!plugins_successful)
    {
        return cleanup(ERROR);
    }

    if (!volatile_mode)
    {
        auto path = file_name;
        path.replace_extension(".hal");
        netlist_serializer::serialize_to_file(netlist.get(), path);
    }

    /* handle file writer */
    if (!netlist_writer_manager::write(netlist.get(), args))
    {
        return cleanup();
    }

    /* cleanup */
    return cleanup();
}
