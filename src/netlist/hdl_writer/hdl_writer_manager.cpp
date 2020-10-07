#include "hal_core/netlist/hdl_writer/hdl_writer_manager.h"

#include "hal_core/utilities/log.h"
#include "hal_core/netlist/hdl_writer/hdl_writer.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"

#include <chrono>
#include <fstream>

namespace hal
{
    namespace hdl_writer_manager
    {
        namespace
        {
            std::unordered_map<std::string, std::vector<std::string>> m_writer_to_extensions;
            std::unordered_map<std::string, std::pair<std::string, WriterFactory>> m_extension_to_writer;

            WriterFactory get_writer_factory_for_file(const std::filesystem::path& file_name)
            {
                auto extension = utils::to_lower(file_name.extension().string());
                if (!extension.empty() && extension[0] != '.')
                {
                    extension = "." + extension;
                }

                if (auto it = m_extension_to_writer.find(extension); it != m_extension_to_writer.end())
                {
                    log_info("hdl_writer", "selected writer: {}", it->second.first);
                    return it->second.second;
                }

                log_error("hdl_writer", "no hdl writer registered for file type '{}'", extension);
                return WriterFactory();
            }
        }    // namespace

        ProgramOptions get_cli_options()
        {
            ProgramOptions description;
            description.add("--write-hdl", "Write netlist to HDL file", {ProgramOptions::A_REQUIRED_PARAMETER});
            return description;
        }

        void register_writer(const std::string& name, const WriterFactory& writer_factory, const std::vector<std::string>& supported_file_extensions)
        {
            for (auto ext : supported_file_extensions)
            {
                ext = utils::trim(utils::to_lower(ext));
                if (!ext.empty() && ext[0] != '.')
                {
                    ext = "." + ext;
                }
                if (auto it = m_extension_to_writer.find(ext); it != m_extension_to_writer.end())
                {
                    log_warning("gate_library_manager", "file type '{}' already has associated writer '{}', it remains unchanged", ext, it->second.first);
                    continue;
                }
                m_extension_to_writer.emplace(ext, std::make_pair(name, writer_factory));
                m_writer_to_extensions[name].push_back(ext);

                log_info("gate_library_manager", "registered gate library writer '{}' for file type '{}'", name, ext);
            }
        }

        void unregister_writer(const std::string& name)
        {
            if (auto it = m_writer_to_extensions.find(name); it != m_writer_to_extensions.end())
            {
                for (const auto& ext : it->second)
                {
                    if (auto rm_it = m_extension_to_writer.find(ext); rm_it != m_extension_to_writer.end())
                    {
                        m_extension_to_writer.erase(rm_it);
                        log_info("gate_library_manager", "unregistered gate library writer '{}' which was registered for file type '{}'", name, ext);
                    }
                }
                m_writer_to_extensions.erase(it);
            }
        }

        bool write(Netlist* netlist, const ProgramArguments& args)
        {
            if (args.is_option_set("--write-hdl"))
            {
                auto output_file = args.get_parameter("--write-hdl");

                return write(netlist, output_file);
            }
            return true;
        }

        bool write(Netlist* netlist, const std::filesystem::path& file_name)
        {
            auto factory = get_writer_factory_for_file(file_name);
            if (!factory)
            {
                return false;
            }

            std::stringstream stream;

            std::ofstream file;
            file.open(file_name.string());
            if (file.fail())
            {
                log_error("hdl_writer", "Cannot open or create file {}. Please verify that the file and the containing directory is writable!", file_name.string());
                return false;
            }

            auto begin_time = std::chrono::high_resolution_clock::now();

            auto writer = factory();
            if (!writer->write(netlist, stream))
            {
                return false;
            }

            // done
            file << stream.str();
            file.close();

            log_info("hdl_writer",
                     "wrote '{}' to '{}' in {:2.2f} seconds.",
                     netlist->get_design_name(),
                     file_name.string(),
                     (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - begin_time).count() / 1000);

            return true;
        }

        bool write(Netlist* netlist, const std::string& type_extension, std::stringstream& stream)
        {
            auto factory = get_writer_factory_for_file("fake_file." + type_extension);
            if (!factory)
            {
                return false;
            }
            auto writer = factory();
            return writer->write(netlist, stream);
        }
    }    // namespace hdl_writer_manager
}    // namespace hal
