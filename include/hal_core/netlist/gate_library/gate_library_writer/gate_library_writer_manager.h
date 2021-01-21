#pragma once

#include "hal_core/defines.h"

#include <filesystem>
#include <functional>

namespace hal
{
    /** forward declaration */
    class GateLibrary;
    class GateLibraryWriter;

    /**
     * The gate library writer manager keeps track of all gate library writers that are available within HAL. Further, it is used to dispatch writing tasks to the respective writers.
     * 
     * @ingroup gate_library_writer
     */
    namespace gate_library_writer_manager
    {
        using WriterFactory = std::function<std::unique_ptr<GateLibraryWriter>()>;

        /**
         * Registers a new gate library writer for a selection of file types.<br>
         * If writers for some of the extensions already exist, they remain changed and only new ones are registered.
         *
         * @param[in] name - The name of the writer.
         * @param[in] writer_factory - A factory function that constructs a new writer instance.
         * @param[in] supported_file_extensions - The file extensions this writer can process.
         */
        NETLIST_API void register_writer(const std::string& name, const WriterFactory& writer_factory, const std::vector<std::string>& supported_file_extensions);

        /**
         * Unregisters a specific writer.
         *
         * @param[in] name - The name of the writer.
         */
        NETLIST_API void unregister_writer(const std::string& name);

        /**
         * Writes the gate library into a file depending on its file extension.
         * 
         * @param[in] gate_lib - The gate library.
         * @param[in] file_path - The output path.
         * @returns True on success, false otherwise.
         */
        NETLIST_API bool write(GateLibrary* gate_lib, const std::filesystem::path& file_path);
    }    // namespace gate_library_writer_manager
}    // namespace hal