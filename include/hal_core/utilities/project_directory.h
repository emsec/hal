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

    class ProjectDirectory : public std::filesystem::path
    {
        friend class ProjectManager;

    public:
        /**
         * ProjectDirectory constructor.
         * 
         * @param path - Path to hal project. If path has an extension (like .v or .hal) the extension will be removed.
         */
        ProjectDirectory(const std::string& path = std::string());

        /**
         * Returns the default file name which is a file with the same as the name as
         * the project directory plus an extension.
         * 
         * @param[in] extension - Extension of default file name. If empty, '.hal' is assumed.
         * @return Absolute path to file
         */
        std::filesystem::path get_default_filename(const std::string& extension = std::string()) const;

        /**
         * Returns absolute path to file within project directory
         * 
         * @param[in] relative_filename - Relative file name in project directory.
         * @return Absolute path of file
         */
        std::filesystem::path get_filename(const std::string& relative_filename) const;

        /**
         * Similar to get_default_filename() but will return file name which is located
         * in the autosave (aka shadow) directory.
         * 
         * @param[in] extension - Extension of shadow file name. If empty '.hal' is assumed
         * @return Absolute path to file
         */
        std::filesystem::path get_shadow_filename(const std::string& extension = std::string()) const;

        /**
         * Get autosave directory path
         * 
         * @return Absolute path to autosave directory
         */
        std::filesystem::path get_shadow_dir() const;

        /**
         * Project directories canonical path. Other than std::filesystem::canonical
         * this method will return an empty path if no project path given.
         * 
         * @return Absolute canonical path to project directory
         */
        std::filesystem::path get_canonical_path() const;

        /**
         * If filename is within project directory the relative file name will
         * be returned. Otherwise this method will return parameter filename
         * without modifications.
         * 
         * @param[in] filename - Absolute path to file
         * @return Relative file name if file in project directory
         */
        std::filesystem::path get_relative_file_path(const std::string& filename) const;

        /**
         * Generate a directory name in current working directory which contains a
         * four digit random number
         * 
         * @return Absolute path to directory
         */
        static ProjectDirectory generate_random();

        /**
         * Name of autosave subdirectory
         */
        static const std::string s_shadow_dir;
    };
}    // namespace hal
