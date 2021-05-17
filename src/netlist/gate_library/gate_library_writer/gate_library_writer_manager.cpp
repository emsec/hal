#include "hal_core/netlist/gate_library/gate_library_writer/gate_library_writer_manager.h"

#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/gate_library/gate_library_writer/gate_library_writer.h"
#include "hal_core/utilities/log.h"

#include <chrono>
#include <fstream>
#include <unordered_map>

namespace hal
{
    namespace gate_library_writer_manager
    {
        namespace
        {
            std::unordered_map<std::string, std::vector<std::string>> m_writer_to_extensions;
            std::unordered_map<std::string, std::pair<std::string, WriterFactory>> m_extension_to_writer;

            WriterFactory get_writer_factory_for_file(const std::filesystem::path& file_name)
            {
                std::string extension = utils::to_lower(file_name.extension().string());
                if (!extension.empty() && extension[0] != '.')
                {
                    extension = "." + extension;
                }

                if (auto it = m_extension_to_writer.find(extension); it != m_extension_to_writer.end())
                {
                    log_info("gate_library_writer", "selected gate library writer '{}'.", it->second.first);
                    return it->second.second;
                }

                log_error("gate_library_writer", "no gate library writer registered for file extension '{}'.", extension);
                return WriterFactory();
            }
        }    // namespace

        void register_writer(const std::string& name, const WriterFactory& writer_factory, const std::vector<std::string>& supported_file_extensions)
        {
            for (std::string ext : supported_file_extensions)
            {
                ext = utils::trim(utils::to_lower(ext));
                if (!ext.empty() && ext[0] != '.')
                {
                    ext = "." + ext;
                }
                if (auto it = m_extension_to_writer.find(ext); it != m_extension_to_writer.end())
                {
                    log_warning("gate_library_writer", "writer '{}' cannot be registered as file extension '{}' is already associated with writer '{}'.", name, ext, it->second.first);
                    continue;
                }
                m_extension_to_writer.emplace(ext, std::make_pair(name, writer_factory));
                m_writer_to_extensions[name].push_back(ext);

                log_info("gate_library_writer", "registered gate library writer '{}' for file extension '{}'.", name, ext);
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
                        log_info("gate_library_writer", "unregistered gate library writer '{}' for file extension '{}'.", name, ext);
                    }
                }
                m_writer_to_extensions.erase(it);
            }
        }

        bool write(GateLibrary* gate_lib, const std::filesystem::path& file_path)
        {
            auto factory = get_writer_factory_for_file(file_path);
            if (!factory)
            {
                return false;
            }

            std::unique_ptr<GateLibraryWriter> writer = factory();

            log_info("gate_library_writer", "writing gate library '{}' to file '{}'...", gate_lib->get_name(), file_path.string());

            auto begin_time = std::chrono::high_resolution_clock::now();
            if (!writer->write(gate_lib, file_path))
            {
                log_error("gate_library_writer", "failed to write gate library '{}' to file '{}'.", gate_lib->get_name(), file_path.string());
                return false;
            }

            log_info("gate_library_writer",
                     "wrote gate library '{}' to file '{}' in {:2.2f} seconds.",
                     gate_lib->get_name(),
                     file_path.string(),
                     (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - begin_time).count() / 1000);

            return true;
        }
    }    // namespace gate_library_writer_manager
}    // namespace hal