#include "hal_core/netlist/netlist_factory.h"

#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_parser/netlist_parser_manager.h"
#include "hal_core/netlist/persistent/netlist_serializer.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/program_arguments.h"

#include <fstream>
#include <unistd.h>

namespace hal
{
    namespace netlist_factory
    {
        std::unique_ptr<Netlist> create_netlist(const GateLibrary* gate_library)
        {
            if (gate_library == nullptr)
            {
                log_critical("netlist", "nullptr given as gate library.");
                return nullptr;
            }

            return std::make_unique<Netlist>(gate_library);
        }

        std::unique_ptr<Netlist> load_netlist(const std::filesystem::path& netlist_file, const std::filesystem::path& gate_library_file)
        {
            if (access(netlist_file.c_str(), F_OK | R_OK) == -1)
            {
                log_critical("netlist", "could not access file '{}'.", netlist_file.string());
                return nullptr;
            }

            GateLibrary* lib = gate_library_manager::load(gate_library_file);
            if (!lib)
            {
                log_critical("netlist", "could not read netlist without gate library.");
                return nullptr;
            }

            return netlist_parser_manager::parse(netlist_file, lib);
        }

        std::unique_ptr<Netlist> load_netlist(const std::filesystem::path& netlist_file)
        {
            if (access(netlist_file.c_str(), F_OK | R_OK) == -1)
            {
                log_critical("netlist", "could not access file '{}'.", netlist_file.string());
                return nullptr;
            }

            return netlist_serializer::deserialize_from_file(netlist_file);
        }

        std::unique_ptr<Netlist> load_netlist(const ProgramArguments& args)
        {
            if (!args.is_option_set("--input-file"))
            {
                log_critical("netlist", "no file to process specified.");
                return nullptr;
            }

            std::filesystem::path netlist_file = std::filesystem::path(args.get_parameter("--input-file"));

            if (access(netlist_file.c_str(), F_OK | R_OK) == -1)
            {
                log_critical("netlist", "cannot access file '{}'.", netlist_file.string());
                return nullptr;
            }

            auto extension = netlist_file.extension();

            if (extension == ".hal")
            {
                return netlist_serializer::deserialize_from_file(netlist_file);
            }

            return netlist_parser_manager::parse(netlist_file, args);
        }

        std::vector<std::unique_ptr<Netlist>> load_netlists(const std::filesystem::path& netlist_file)
        {
            if (access(netlist_file.c_str(), F_OK | R_OK) == -1)
            {
                log_critical("netlist", "could not access file '{}'.", netlist_file.string());
                return {};
            }

            return netlist_parser_manager::parse_all(netlist_file);
        }
    }    // namespace netlist_factory
}    // namespace hal
