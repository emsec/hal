#include "hal_core/netlist/netlist_factory.h"

#include "hal_core/utilities/log.h"
#include "hal_core/utilities/program_arguments.h"
#include "hal_core/netlist/event_system/event_controls.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/hdl_parser/hdl_parser_manager.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/persistent/netlist_serializer.h"

#include <fstream>
#include <iostream>
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

        std::unique_ptr<Netlist> load_netlist(const std::filesystem::path& hdl_file, const std::filesystem::path& gate_library_file)
        {
            if (access(hdl_file.c_str(), F_OK | R_OK) == -1)
            {
                log_critical("netlist", "could not access file '{}'.", hdl_file.string());
                return nullptr;
            }

            GateLibrary* lib = gate_library_manager::load_file(gate_library_file);
            if (!lib)
            {
                log_critical("netlist", "could not read netlist without gate library.");
                return nullptr;
            }

            return hdl_parser_manager::parse(hdl_file, lib);
        }

        std::unique_ptr<Netlist> load_netlist(const std::filesystem::path& hal_file)
        {
            if (access(hal_file.c_str(), F_OK | R_OK) == -1)
            {
                log_critical("netlist", "could not access file '{}'.", hal_file.string());
                return nullptr;
            }

            return netlist_serializer::deserialize_from_file(hal_file);
        }

        std::unique_ptr<Netlist> load_netlist(const ProgramArguments& args)
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

            if (extension == ".hal")
            {
                return netlist_serializer::deserialize_from_file(file_name);
            }

            return hdl_parser_manager::parse(file_name, args);
        }

        std::vector<std::unique_ptr<Netlist>> load_netlists(const std::filesystem::path& hdl_file)
        {
            if (access(hdl_file.c_str(), F_OK | R_OK) == -1)
            {
                log_critical("netlist", "could not access file '{}'.", hdl_file.string());
                return {};
            }

            return hdl_parser_manager::parse_all(hdl_file);
        }
    }    // namespace netlist_factory
}    // namespace hal
