#pragma once

#include "hal_core/defines.h"

#include <filesystem>

namespace hal
{
    /* forward declaration*/
    class GateLibrary;

    /**
     * The gate library writer is the base class that needs to be inherited from all writers that shall be registered with HAL.
     * 
     * @ingroup gate_lib_writer
     */
    class NETLIST_API GateLibraryWriter
    {
    public:
        GateLibraryWriter()          = default;
        virtual ~GateLibraryWriter() = default;

        /**
         * Write the gate library to a file at the provided location.
         *
         * @param[in] gate_lib - The gate library.
         * @param[in] file_path - The output path.
         * @returns True on success.
         */
        virtual bool write(GateLibrary* gate_lib, const std::filesystem::path& file_path) = 0;
    };
}    // namespace hal