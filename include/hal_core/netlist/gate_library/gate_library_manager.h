#pragma once

#include "hal_core/defines.h"

#include <filesystem>
#include <vector>

namespace hal
{
    class GateLibrary;

    /**
     * TODO python documentation
     * @ingroup gate_lib
     */
    namespace gate_library_manager
    {
        /**
         * Load a gate library from file.
         *
         * @param[in] file_path - The input path.
         * @param[in] reload_if_existing - If true, reloads the library in case it is already loaded.
         * @returns Pointer to the gate library or a nullptr on failure.
         */
        NETLIST_API GateLibrary* load(std::filesystem::path file_path, bool reload_if_existing = false);

        /**
         * Load all available gate libraries.
         *
         * @param[in] reload_if_existing - If true, reloads all libraries that have already been loaded.
         */
        NETLIST_API void load_all(bool reload_if_existing = false);

        /**
         * Get a gate library by file path.<br>
         * If no library with the given name is loaded, loading the gate library from file will be attempted.
         *
         * @param[in] file_path - The input path.
         * @returns Pointer to the gate library or a nullptr on failure.
         */
        NETLIST_API GateLibrary* get_gate_library(const std::string& file_path);

        /**
         * Get a gate library by name.<br>
         * If no library with the given name is loaded, a nullptr will be returned.
         *
         * @param[in] lib_name - The name of the gate library.
         * @returns Pointer to the gate library or a nullptr on failure.
         */
        NETLIST_API GateLibrary* get_gate_library_by_name(const std::string& lib_name);

        /**
         * Get all loaded gate libraries.
         *
         * @returns A vector of pointers to the gate libraries.
         */
        NETLIST_API std::vector<GateLibrary*> get_gate_libraries();

    }    // namespace gate_library_manager
}    // namespace hal
