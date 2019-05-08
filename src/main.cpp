#include "def.h"

#include "core/log.h"
#include "core/plugin_manager.h"
#include "core/utils.h"
#include "hal_version.h"

#include "core/interface_cli.h"
#include "core/program_arguments.h"
#include "core/program_options.h"

#include "core/interface_interactive_ui.h"

#include "hdl_parser/hdl_parser_dispatcher.h"
#include "hdl_writer/hdl_writer_dispatcher.h"
#include "netlist/event_system/event_log.h"
#include "netlist/gate_library/gate_library_manager.h"
#include "netlist/netlist.h"
#include "netlist/netlist_factory.h"
#include "netlist/persistent/netlist_serializer.h"

#include <fstream>
#include <iostream>
#include <string>

#define SUCCESS 0
#define ERROR 1

void initialize_cli_options(program_options& cli_options)
{
    program_options generic_options("generic options");
    /* initialize generic options */
    generic_options.add({"-h", "--help"}, "print help messages");
    generic_options.add({"-v", "--version"}, "displays the current version");
    generic_options.add({"-L", "--show-log-options"}, "show all logging options");
    generic_options.add({"-l", "--logfile"}, "specify log file name", {""});
    generic_options.add({"--log-time"}, "includes time information into the log");
    generic_options.add({"--licenses"}, "Shows the licenses of projects used by HAL");

    generic_options.add({"-i", "--input-file"}, "input file", {program_options::REQUIRED_PARAM});
    generic_options.add({"-g", "--gui"}, "start graphical user interface");
    generic_options.add("--python",
                        "start python shell. To run a python script "
                        "use following syntax: --python <python3 "
                        "interpreter options> <file to process> "
                        "<args to pass to python script>");
    generic_options.add("--volatile-mode", "prevents hal from creating a .hal progress file (e.g. cluster use)");
    generic_options.add("--no-log", "prevents hal from creating a .log file");

    /* initialize hdl parser options */
    generic_options.add(hdl_parser_dispatcher::get_cli_options());

    /* initialize hdl writer options */
    generic_options.add(hdl_writer_dispatcher::get_cli_options());
    cli_options.add(generic_options);
}

int redirect_control_to_interactive_ui(const std::string& name, program_arguments& args)
{
    /* add timestamp to log output */
    log_manager::get_instance().set_format_pattern("[%d.%m.%Y %H:%M:%S] [%n] [%l] %v");
    event_log::initialize();

    auto file_name = core_utils::get_file(std::string("lib" + name + ".") + std::string(LIBRARY_FILE_EXTENSION), {core_utils::get_library_directory()});
    if (!plugin_manager::load(name, file_name))
    {
        return ERROR;
    }

    auto factory = plugin_manager::get_plugin_factory(name);
    if (factory == nullptr)
    {
        return ERROR;
    }
    log_info("core", "Starting {}.", name);
    auto ui = std::dynamic_pointer_cast<i_interactive_ui>(factory->query_interface(interface_type::interactive_ui));
    ui->initialize_logging();
    auto ret = ui->exec(args);
    log_info("core", "Closing {}.", name);
    return (ret) ? SUCCESS : ERROR;
}

int cleanup(std::shared_ptr<netlist> const g = nullptr)
{
    if (!plugin_manager::unload_all_plugins())
    {
        return ERROR;
    }

    if (g != nullptr)
    {
        log_info("core", "closed '{}'", g->get_input_filename().string());
    }
    return SUCCESS;
}

int main(int argc, const char* argv[])
{
    /* initialize and parse generic cli options */
    program_options cli_options("cli options");
    initialize_cli_options(cli_options);
    program_options all_options("all options");
    all_options.add(cli_options);
    all_options.add(log_manager::get_instance().get_option_descriptions());
    auto args = all_options.parse(argc, argv);

    /* initialize logging */
    log_manager& lm = log_manager::get_instance();
    if (args.is_option_set("--logfile"))
    {
        lm.set_file_name(hal::path(args.get_parameter("--logfile")));
    }
    lm.handle_options(args);

    if (args.is_option_set("--log-time"))
    {
        lm.set_format_pattern("[%d.%m.%Y %H:%M:%S] [%n] [%l] %v");
    }

    /* redirect control to gui or python if enabled */
#ifdef WITH_GUI
    if (args.is_option_set("--gui"))
    {
        return redirect_control_to_interactive_ui("hal_gui", args);
    }
#endif
    if (args.is_option_set("--python"))
    {
        return redirect_control_to_interactive_ui("hal_python", args);
    }
    UNUSED(redirect_control_to_interactive_ui);    // in case neither gui nor python is defined

    /* initialize plugin manager */
    plugin_manager::add_existing_options_description(cli_options);

    if (!plugin_manager::load_all_plugins())
    {
        return ERROR;
    }

    /* add plugin cli options */
    auto program_options = plugin_manager::get_cli_plugin_options();
    if (!program_options.get_options().empty())
    {
        cli_options.add(plugin_manager::get_cli_plugin_options());
    }

    /* process help output */
    if (args.is_option_set("--help") || args.get_set_options().size() == 0)
    {
        std::cout << cli_options.get_options_string() << std::endl;
        return SUCCESS;
    }

    if (args.is_option_set("--version"))
    {
        std::cout << hal_version::version << std::endl;
        return SUCCESS;
    }

    if (args.is_option_set("--licenses"))
    {
        std::cout << core_utils::get_open_source_licenses() << std::endl;
        return SUCCESS;
    }

    /**
     * control for command line interface
     */

    if (args.is_option_set("--show-log-options"))
    {
        std::cout << lm.get_option_descriptions().get_options_string() << std::endl;
        return SUCCESS;
    }

    /* parse program options */
    bool unknown_option_exists = false;
    args                       = cli_options.parse(argc, argv);
    for (const auto& opt : cli_options.get_unknown_arguments())
    {
        unknown_option_exists = true;
        log_error("core", "unkown command line argument '{}'", opt);
    }
    if (unknown_option_exists)
    {
        return cleanup();
    }

    /* handle input file */
    gate_library_manager::load_all();
    auto netlist = netlist_factory::load_netlist(args);
    if (netlist == nullptr)
    {
        cleanup();
        return -1;
    }
    auto file_name = hal::path(args.get_parameter("--input-file"));

    /* handle database configuration */
    hal::path log_path = file_name;
    if (args.is_option_set("--no-log"))
    {
        log_warning("core",
                    "the console output will not be written to a local log "
                    "file (--no-log).");
    }
    else if (!args.is_option_set("--logfile"))
    {
        lm.set_file_name(log_path.replace_extension(".log"));
    }

    bool volatile_mode = false;
    if (args.is_option_set("--volatile_mode"))
    {
        volatile_mode = true;
        log_warning("core", "your modifications will not be written to a .hal file (--volatile-mode).");
    }

    /* parse plugin options */
    std::vector<std::string> plugins_to_execute;
    auto option_to_plugin_name = plugin_manager::get_flag_to_plugin_mapping();
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
        auto factory = plugin_manager::get_plugin_factory(plugin_name);
        if (factory == nullptr)
        {
            return cleanup(netlist);
        }
        auto plugin = std::dynamic_pointer_cast<i_cli>(factory->query_interface(interface_type::cli));

        program_arguments plugin_args;

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
            log_info("core", "  '{}': {}", option, core_utils::join(",", plugin_args.get_parameters(option)));
        }

        if (!plugin->handle_cli_call(netlist, plugin_args))
        {
            plugins_successful = false;
            break;
        }
    }

    if (!plugins_successful)
    {
        return cleanup(netlist);
    }

    if (!volatile_mode)
    {
        hal::path path = file_name;
        path.replace_extension(".hal");
        netlist_serializer::serialize_to_file(netlist, path);
    }

    /* handle file writer */
    if (!hdl_writer_dispatcher::write(netlist, args))
    {
        return cleanup(netlist);
    }

    /* cleanup */
    return cleanup(netlist);
}