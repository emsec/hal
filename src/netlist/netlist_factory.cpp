#include "netlist/netlist_factory.h"

#include "core/log.h"
#include "core/program_arguments.h"
#include "netlist/event_system/event_controls.h"
#include "netlist/gate_library/gate_library_manager.h"
#include "netlist/hdl_parser/hdl_parser_dispatcher.h"
#include "netlist/netlist.h"
#include "netlist/persistent/netlist_serializer.h"

#include <fstream>
#include <iostream>
#include <unistd.h>

namespace netlist_factory
{
    std::shared_ptr<netlist> create_netlist(const std::shared_ptr<gate_library>& gate_library)
    {
        return std::make_shared<netlist>(gate_library);
    }

    std::shared_ptr<netlist> load_netlist(const hal::path& hdl_file, const std::string& language, const hal::path& gate_library_file)
    {
        if (access(hdl_file.c_str(), F_OK | R_OK) == -1)
        {
            log_critical("netlist", "cannot access file '{}'.", hdl_file.string());
            return nullptr;
        }

        auto lib = gate_library_manager::load_file(gate_library_file);

        if (!lib)
        {
            log_critical("netlist", "cannot read netlist without gate library.");
            return nullptr;
        }

        std::shared_ptr<netlist> nl = hdl_parser_dispatcher::parse(lib, language, hdl_file);

        return nl;
    }

    std::shared_ptr<netlist> load_netlist(const hal::path& hal_file)
    {
        if (access(hal_file.c_str(), F_OK | R_OK) == -1)
        {
            log_critical("netlist", "cannot access file '{}'.", hal_file.string());
            return nullptr;
        }

        std::shared_ptr<netlist> nl = netlist_serializer::deserialize_from_file(hal_file);

        return nl;
    }

    std::shared_ptr<netlist> load_netlist(const program_arguments& args)
    {
        if (!args.is_option_set("--input-file"))
        {
            log_critical("netlist", "no file to process specified.");
            return nullptr;
        }

        hal::path file_name = hal::path(args.get_parameter("--input-file"));

        if (access(file_name.c_str(), F_OK | R_OK) == -1)
        {
            log_critical("netlist", "cannot access file '{}'.", file_name.string());
            return nullptr;
        }

        auto extension = file_name.extension();

        std::shared_ptr<netlist> nl = nullptr;

        if (extension == ".hal")
        {
            nl = netlist_serializer::deserialize_from_file(file_name);
        }
        else
        {
            nl = hdl_parser_dispatcher::parse(file_name, args);
        }

        return nl;
    }
}    // namespace netlist_factory
