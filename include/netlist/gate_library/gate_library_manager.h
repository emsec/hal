//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include "def.h"
#include <map>
#include <memory>
#include <vector>

class gate_library;

/**
 * @ingroup netlist
 */
namespace gate_library_manager
{
    /**
     * Loads all gate libraries which are available.
     *
     * @param[in] path - the file to load.
     * @param[in] reload_if_existing - If true, reloads all libraries that are already loaded.
     */
    NETLIST_API std::shared_ptr<gate_library> load_file(const hal::path& path, bool reload_if_existing = false);

    /**
     * Loads all gate libraries which are available.
     *
     * @param[in] reload_if_existing - If true, reloads all libraries that are already loaded.
     */
    NETLIST_API void load_all(bool reload_if_existing = false);

    /**
     * Get a gate library object by name.
     *
     * @param[in] name - Name of the gate library.
     * @returns Pointer to the gate library object.
     */
    NETLIST_API std::shared_ptr<gate_library> get_gate_library(const std::string& name);

    /**
     * Get all gate libraries together with the associated name.
     *
     * @returns A map from library name to pointer to the gate library object.
     */
    NETLIST_API std::vector<std::shared_ptr<gate_library>> get_gate_libraries();
}    // namespace gate_library_manager
