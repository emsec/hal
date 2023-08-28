#include "hal_core/netlist/netlist_factory.h"

#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_parser/netlist_parser_manager.h"
#include "hal_core/netlist/persistent/netlist_serializer.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/program_arguments.h"
#include "hal_core/netlist/project_manager.h"
#include "hal_core/utilities/project_directory.h"

#include <fstream>
#include <unistd.h>

namespace hal
{
    namespace netlist_factory
    {
        std::unique_ptr<Netlist> create_netlist(const GateLibrary* gate_library)
        {
            netlist_serializer::Error::instance()->reset();
            if (gate_library == nullptr)
            {
                log_critical("netlist", "nullptr given as gate library.");
                netlist_serializer::Error::instance()->setError(netlist_serializer::Error::GatelibNotProvieded);
                return nullptr;
            }

            return std::make_unique<Netlist>(gate_library);
        }

        std::unique_ptr<Netlist> load_netlist(const std::filesystem::path& netlist_file, const std::filesystem::path& gate_library_file)
        {
            netlist_serializer::Error::instance()->reset();
            if (access(netlist_file.c_str(), F_OK | R_OK) == -1)
            {
                log_critical("netlist", "could not access file '{}'.", netlist_file.string());
                netlist_serializer::Error::instance()->setError(netlist_serializer::Error::NetlistNotAccessible);
                return nullptr;
            }

            GateLibrary* lib = nullptr;

            if (!gate_library_file.empty())
            {
                lib = gate_library_manager::load(gate_library_file);
                if (!lib)
                {
                    log_critical("netlist", "could not parse gate library '{}', will not read netlist.", gate_library_file.string());
                    return nullptr;
                }
            }

            if (netlist_file.extension() == ".hal")
            {
                return netlist_serializer::deserialize_from_file(netlist_file, lib);
            }
            else
            {
                if (!lib)
                {
                    log_critical("netlist", "could not read netlist '{}' without gate library.", netlist_file.string());
                    return nullptr;
                }
                return netlist_parser_manager::parse(netlist_file, lib);
            }
        }

        std::unique_ptr<Netlist> load_hal_project(const std::filesystem::path& project_dir)
        {
            netlist_serializer::Error::instance()->reset();
            if (!std::filesystem::is_directory(project_dir))
            {
                log_critical("netlist", "could not access hal project '{}'.", project_dir.string());
                netlist_serializer::Error::instance()->setError(netlist_serializer::Error::ProjectNotAccessible);
                return nullptr;
            }

            ProjectManager* pm = ProjectManager::instance();
            if (!pm->open_project(project_dir.string()))
            {
                log_critical("netlist", "could not open hal project '{}'.", project_dir.string());
                netlist_serializer::Error::instance()->setError(netlist_serializer::Error::ProjectNotAccessible);
                return nullptr;
            }

            std::unique_ptr<Netlist> retval = std::move(pm->get_netlist());
            return retval;
        }

        std::unique_ptr<Netlist> load_netlist(const ProjectDirectory& pdir, const ProgramArguments& args)
        {
            netlist_serializer::Error::instance()->reset();
            std::filesystem::path netlist_file = args.is_option_set("--import-netlist")
                    ? std::filesystem::path(args.get_parameter("--import-netlist"))
                    : pdir.get_default_filename();

            if (access(netlist_file.c_str(), F_OK | R_OK) == -1)
            {
                log_critical("netlist", "cannot access file '{}'.", netlist_file.string());
                netlist_serializer::Error::instance()->setError(netlist_serializer::Error::NetlistNotAccessible);
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
            netlist_serializer::Error::instance()->reset();
            if (access(netlist_file.c_str(), F_OK | R_OK) == -1)
            {
                log_critical("netlist", "could not access file '{}'.", netlist_file.string());
                return {};
            }

            return netlist_parser_manager::parse_all(netlist_file);
        }
    }    // namespace netlist_factory
}    // namespace hal
