//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
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

#include "hal_core/defines.h"

#include <filesystem>
#include <functional>

namespace hal
{
    /** forward declaration */
    class GateLibrary;
    class GateLibraryWriter;

    /**
     * The gate library writer manager keeps track of all gate library writers that are available within HAL. It is used to dispatch writing tasks to the respective writers.
     * 
     * @ingroup gate_library_writer
     */
    namespace gate_library_writer_manager
    {
        using WriterFactory = std::function<std::unique_ptr<GateLibraryWriter>()>;

        /**
         * Register a new gate library writer for a selection of file types.<br>
         * If writers for some of the extensions already exist, they remain unchanged and the new parser is not registered at all.
         *
         * @param[in] name - The name of the writer.
         * @param[in] writer_factory - A factory function that constructs a new writer instance.
         * @param[in] supported_file_extensions - The file extensions this writer can process.
         */
        NETLIST_API void register_writer(const std::string& name, const WriterFactory& writer_factory, const std::vector<std::string>& supported_file_extensions);

        /**
         * Unregister the specified writer.
         *
         * @param[in] name - The name of the writer.
         */
        NETLIST_API void unregister_writer(const std::string& name);

        /**
         * Write the gate library into a file at the given location.
         * 
         * @param[in] gate_lib - The gate library.
         * @param[in] file_path - The output path.
         * @returns True on success, false otherwise.
         */
        NETLIST_API bool write(GateLibrary* gate_lib, const std::filesystem::path& file_path);
    }    // namespace gate_library_writer_manager
}    // namespace hal