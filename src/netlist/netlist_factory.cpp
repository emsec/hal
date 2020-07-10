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

        std::shared_ptr<Netlist> load_netlist(const std::filesystem::path& hdl_file, const std::filesystem::path& gate_library_file, const std::string& parser_name)
        {
            if (access(hdl_file.c_str(), F_OK | R_OK) == -1)
            {
                log_critical("netlist", "could not access file '{}'.", hdl_file.string());
                return nullptr;
            }

            auto lib = gate_library_manager::load_file(gate_library_file);
            if (!lib)
            {
                log_critical("netlist", "could not load specified gate library.");
                return nullptr;
            }

            return HDLParserDispatcher::parse(hdl_file, parser_name, lib);
        }

        std::vector<std::shared_ptr<Netlist>> load_netlists(const std::filesystem::path& hdl_file, const std::string& parser_name)
        {
            if (access(hdl_file.c_str(), F_OK | R_OK) == -1)
            {
                log_critical("netlist", "could not access file '{}'.", hdl_file.string());
                return {};
            }

            return HDLParserDispatcher::parse_all(hdl_file, parser_name);
        }

        std::shared_ptr<Netlist> load_netlist(const std::filesystem::path& hal_file)
        {
            if (access(hal_file.c_str(), F_OK | R_OK) == -1)
            {
                log_critical("netlist", "could not access file '{}'.", hal_file.string());
                return nullptr;
            }

            return netlist_serializer::deserialize_from_file(hal_file);
        }

        std::shared_ptr<Netlist> load_netlist(const ProgramArguments& args)
        {
            if (!args.is_option_set("--input-file"))
            {
                log_critical("netlist", "no file to process specified.");
                return nullptr;
            }

            std::filesystem::path hdl_file = std::filesystem::path(args.get_parameter("--input-file"));

            if (access(hdl_file.c_str(), F_OK | R_OK) == -1)
            {
                log_critical("netlist", "could not access file '{}'.", hdl_file.string());
                return nullptr;
            }

            auto extension = hdl_file.extension();

            std::shared_ptr<Netlist> nl = nullptr;

            if (extension == ".hal")
            {
                return netlist_serializer::deserialize_from_file(hdl_file);
            }
            else
            {
                return HDLParserDispatcher::parse(hdl_file, args);
            }
        }
    }    // namespace netlist_factory
}    // namespace hal
