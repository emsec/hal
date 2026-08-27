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

#include <filesystem>
#include <string>
#include <vector>

namespace hal
{

    /**
     * Represents a project directory.
     *
     * @ingroup utilities
     */
    class ProjectDirectory : public std::filesystem::path
    {
        friend class ProjectManager;

    public:
        /**
         * Constructs a ProjectDirectory object.
         *
         * @param[in] path - Path to the project directory. If the path has an extension (like `.v` or `.hal`) the extension will be removed.
         */
        ProjectDirectory(const std::string& path = std::string());

        /**
         * Returns the default file name for the project directory, which is a file with the same name as the project directory plus an extension.
         *
         * @param[in] extension - Extension of the default file name. If empty, `.hal` is assumed.
         * @returns The absolute path to the default file.
         */
        std::filesystem::path get_default_filename(const std::string& extension = std::string()) const;

        /**
         * Returns the absolute path to a file within the project directory.
         *
         * @param[in] relative_filename - The relative file name within the project directory.
         * @returns The absolute path to the file.
         */
        std::filesystem::path get_filename(const std::string& relative_filename) const;

        /**
         * Returns the file name within the autosave (shadow) directory.
         *
         * @param[in] extension - Extension of the shadow file name. If empty, `.hal` is assumed.
         * @returns The absolute path to the shadow file.
         */
        std::filesystem::path get_shadow_filename(const std::string& extension = std::string()) const;

        /**
         * Returns the path to the autosave (shadow) directory.
         *
         * @returns The absolute path to the autosave directory.
         */
        std::filesystem::path get_shadow_dir() const;

        /**
         * Returns the canonical path to the project directory.
         *
         * @returns The absolute canonical path to the project directory. If no project path is given, an empty path is returned.
         */
        std::filesystem::path get_canonical_path() const;

        /**
         * Returns the relative file path if the file is within the project directory.
         *
         * @param[in] filename - The absolute path to the file.
         * @returns The relative file path if the file is within the project directory; otherwise, the original filename.
         */
        std::filesystem::path get_relative_file_path(const std::string& filename) const;

        /**
         * Generates a random directory name in the current working directory.
         *
         * @returns The absolute path to the generated directory.
         */
        static ProjectDirectory generate_random();

        /**
         * Name of autosave subdirectory
         */
        static const std::string s_shadow_dir;
    };
}    // namespace hal
