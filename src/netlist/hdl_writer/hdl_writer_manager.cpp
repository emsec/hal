#include "netlist/hdl_writer/hdl_writer_manager.h"

#include "core/log.h"
#include "netlist/hdl_writer/hdl_writer.h"
#include "netlist/netlist.h"
#include "netlist/netlist_factory.h"

#include <chrono>
#include <fstream>

namespace hal
{
    namespace hdl_writer_manager
    {
        namespace
        {
            std::unordered_map<HDLWriter*, std::vector<std::string>> m_writer_to_extensions;
            std::unordered_map<std::string, HDLWriter*> m_extension_to_writer;

            HDLWriter* get_writer_for_file(const std::filesystem::path& file_name)
            {
                auto extension = core_utils::to_lower(file_name.extension().string());
                if (!extension.empty() && extension[0] != '.')
                {
                    extension = "." + extension;
                }

                HDLWriter* writer = nullptr;
                if (auto it = m_extension_to_writer.find(extension); it != m_extension_to_writer.end())
                {
                    writer = it->second;
                }
                if (writer == nullptr)
                {
                    log_error("hdl_writer", "no hdl writer registered for file type '{}'", extension);
                    return nullptr;
                }

                log_info("hdl_writer", "selected writer: {}", writer->get_name());

                return writer;
            }
        }    // namespace

        ProgramOptions get_cli_options()
        {
            ProgramOptions description;
            description.add("--write-hdl", "Write netlist to HDL file", {ProgramOptions::A_REQUIRED_PARAMETER});
            return description;
        }

        void register_writer(HDLWriter* writer, const std::vector<std::string>& supported_file_extensions)
        {
            for (auto ext : supported_file_extensions)
            {
                ext = core_utils::trim(core_utils::to_lower(ext));
                if (!ext.empty() && ext[0] != '.')
                {
                    ext = "." + ext;
                }
                if (auto it = m_extension_to_writer.find(ext); it != m_extension_to_writer.end())
                {
                    log_warning("hdl_writer", "file type '{}' already has associated writer '{}', it remains unchanged", ext, it->second->get_name());
                    continue;
                }
                m_extension_to_writer.emplace(ext, writer);
                m_writer_to_extensions[writer].push_back(ext);

                log_info("hdl_writer", "registered hdl writer '{}' for file type '{}'", writer->get_name(), ext);
            }
        }

        void unregister_writer(HDLWriter* writer)
        {
            if (auto it = m_writer_to_extensions.find(writer); it != m_writer_to_extensions.end())
            {
                for (const auto& ext : it->second)
                {
                    if (auto rm_it = m_extension_to_writer.find(ext); rm_it != m_extension_to_writer.end())
                    {
                        m_extension_to_writer.erase(rm_it);
                        log_info("hdl_writer", "unregistered hdl writer '{}' which was registered for file type '{}'", writer->get_name(), ext);
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
            auto writer = get_writer_for_file(file_name);
            if (writer == nullptr)
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

            bool write_success = false;

            auto begin_time = std::chrono::high_resolution_clock::now();

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

            return write_success;
        }
    }    // namespace hdl_writer_manager
}    // namespace hal
