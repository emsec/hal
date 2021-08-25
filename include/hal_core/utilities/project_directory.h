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


#include <string>
#include <vector>
#include <filesystem>

namespace hal {

    class ProjectDirectory : public std::filesystem::path
    {
        friend class ProjectManager;

    public:
        ProjectDirectory(const std::string& dirname = std::string());

        std::filesystem::path get_default_filename(const std::string& extension = std::string()) const;

        std::filesystem::path get_filename(const std::string& relative_filename) const;

        std::filesystem::path get_shadow_filename(const std::string& extension = std::string()) const;

        std::filesystem::path get_shadow_dir() const;

        std::filesystem::path get_canonical_path() const;

        std::filesystem::path get_relative_file_path(const std::string& filename) const;

        static ProjectDirectory generateRandom();
        static const std::string s_shadow_dir;
    };
}
