// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "hal_core/defines.h"
#include "hal_core/utilities/result.h"
#include "rapidjson/document.h"

#include <filesystem>
#include <string>
#include <vector>

namespace hal
{
    /**
     * Upgrade of gate library files in HGL format from an older format version to version 6.
     *
     * What changed in version 6:
     *  1. The flat LUT configuration `{data_identifier, data_category, bit_order}` is replaced by an `output_pins` array,
     *     where each entry specifies the output pin, its `data_identifier`, `bit_offset`, `bit_count`, `is_ascending`
     *     and `input_pins`.
     *  2. Each cell that carries init data gets a `parameters` section, so that INIT identifiers can be resolved by name
     *     and size: one `bit_vector` parameter of size 2^n per LUT identifier, of size 1 per flip-flop or latch
     *     identifier, and of size `bit_size / count` per RAM identifier.
     *  3. Legacy gate type properties without the `c_` prefix (`buffer`, `carry`, `mux`, `lut`) are renamed to their
     *     canonical forms.
     *
     * @ingroup netlist
     */
    namespace hgl_upgrade
    {
        /**
         * The format version gate libraries are upgraded to.
         */
        const u32 TARGET_VERSION = 6;

        /**
         * Upgrade a parsed HGL document in place.
         *
         * A document that is already at the target version is left unchanged. Cells that cannot be migrated
         * automatically are left unchanged as well and reported by a warning.
         *
         * @param[in,out] document - The parsed HGL document.
         * @returns A list of warnings, one per cell that requires manual migration, or an error if the document cannot
         * be upgraded at all.
         */
        NETLIST_API Result<std::vector<std::string>> upgrade_document(rapidjson::Document& document);

        /**
         * Get the file name the original gate library is kept under after its upgrade.
         *
         * The `.hgl` extension is replaced by `.json`, so that the original file does not get selected as a gate
         * library by accident.
         *
         * @param[in] file_path - The path of the gate library file.
         * @returns The path of the backup file.
         */
        NETLIST_API std::filesystem::path get_backup_path(const std::filesystem::path& file_path);

        /**
         * Replace a gate library file by an upgraded document.
         *
         * The original file is renamed as given by `get_backup_path` and the document is written under the original
         * name, so that projects referring to the gate library need no changes. If writing fails, the original file
         * is restored.
         *
         * @param[in] document - The upgraded HGL document.
         * @param[in] file_path - The path of the gate library file to replace.
         * @returns The path of the backup file on success, an error otherwise.
         */
        NETLIST_API Result<std::filesystem::path> replace_file(const rapidjson::Document& document, const std::filesystem::path& file_path);
    }    // namespace hgl_upgrade
}    // namespace hal
