#include "netlist/netlist_factory.h"

#include "core/log.h"
#include "core/program_arguments.h"
#include "netlist/event_system/event_controls.h"
#include "netlist/gate_library/gate_library_manager.h"
#include "netlist/hdl_parser/hdl_parser_manager.h"
#include "netlist/netlist.h"
#include "netlist/persistent/netlist_serializer.h"

#include <fstream>
#include <iostream>
#include <unistd.h>

namespace hal
{
    namespace netlist_factory
    {
        std::shared_ptr<Netlist> create_netlist(const std::shared_ptr<GateLibrary>& gate_library)
        {
            if (gate_library == nullptr)
            {
                log_critical("netlist", "nullptr given as gate library.");
                return nullptr;
            }

            return std::make_shared<Netlist>(gate_library);
        }

        std::shared_ptr<Netlist> load_netlist(const std::filesystem::path& hdl_file, const std::filesystem::path& gate_library_file)
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

            std::shared_ptr<Netlist> nl = HDLParserManager::parse(hdl_file, lib);

            return nl;
        }

        std::shared_ptr<Netlist> load_netlist(const std::filesystem::path& hal_file)
        {
            if (access(hal_file.c_str(), F_OK | R_OK) == -1)
            {
                log_critical("netlist", "cannot access file '{}'.", hal_file.string());
                return nullptr;
            }

            std::shared_ptr<Netlist> nl = netlist_serializer::deserialize_from_file(hal_file);

            return nl;
        }

        std::shared_ptr<Netlist> load_netlist(const ProgramArguments& args)
        {
            if (!args.is_option_set("--input-file"))
            {
                log_critical("netlist", "no file to process specified.");
                return nullptr;
            }

            std::filesystem::path file_name = std::filesystem::path(args.get_parameter("--input-file"));

            if (access(file_name.c_str(), F_OK | R_OK) == -1)
            {
                log_critical("netlist", "cannot access file '{}'.", file_name.string());
                return nullptr;
            }

            auto extension = file_name.extension();

            std::shared_ptr<Netlist> nl = nullptr;

            if (extension == ".hal")
            {
                nl = netlist_serializer::deserialize_from_file(file_name);
            }
            else
            {
                nl = HDLParserManager::parse(file_name, args);
            }

            return nl;
        }
    }    // namespace netlist_factory
}    // namespace hal
