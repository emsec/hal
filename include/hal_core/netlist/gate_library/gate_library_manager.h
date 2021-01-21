#pragma once

#include "hal_core/defines.h"

#include <filesystem>
#include <vector>

namespace hal
{
    class GateLibrary;

    /**
     * The gate library manager keeps track of all gate libraries that are used within HAL. Further, it takes care of loading and saving gate libraries on demnand.
     * 
     * @ingroup gate_lib
     */
    namespace gate_library_manager
    {
        /**
         * Load a gate library from file.
         *
         * @param[in] file_path - The input path.
         * @param[in] reload - If true, reloads the library in case it is already loaded.
         * @returns The gate library on success, nullptr otherwise.
         */
        NETLIST_API GateLibrary* load(std::filesystem::path file_path, bool reload = false);

        /**
         * Load all gate libraries available in standard gate library directories.
         *
         * @param[in] reload - If true, reloads all libraries that have already been loaded.
         */
        NETLIST_API void load_all(bool reload = false);

        /**
         * Save a gate library to file.
         * 
         * @param[in] file_path - The output path. 
         * @param[in] gate_lib - The gate library.
         * @param[in] overwrite - If true, overwrites already existing files.
         * @returns True on success, false otherwise.
         */
        // TODO test
        NETLIST_API bool save(std::filesystem::path file_path, GateLibrary* gate_lib, bool overwrite = false);

        /**
         * Get a gate library by file path. If no library with the given name is loaded, loading the gate library from file will be attempted.
         *
         * @param[in] file_path - The input path.
         * @returns The gate library on success, nullptr otherwise.
         */
        NETLIST_API GateLibrary* get_gate_library(const std::string& file_path);

        /**
         * Get a gate library by name. If no library with the given name is loaded, a nullptr will be returned.
         *
         * @param[in] lib_name - The name of the gate library.
         * @returns The gate library on success, nullptr otherwise.
         */
        NETLIST_API GateLibrary* get_gate_library_by_name(const std::string& lib_name);

        /**
         * Get all loaded gate libraries.
         *
         * @returns A vector of gate libraries.
         */
        NETLIST_API std::vector<GateLibrary*> get_gate_libraries();

    }    // namespace gate_library_manager
}    // namespace hal
