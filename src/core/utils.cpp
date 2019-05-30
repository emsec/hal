#include "core/utils.h"

#include "core/log.h"

#include <sstream>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/predef.h>

#include <boost/algorithm/string/predicate.hpp>

#include <dirent.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#include <tchar.h>
#include <windows.h>
#elif __APPLE__ && __MACH__

#include <mach-o/dyld.h>
#include <unistd.h>

#elif __linux__
#include <stdlib.h>
#include <unistd.h>
#endif

namespace core_utils
{
    bool ends_with(const std::string& full_string, const std::string& ending)
    {
        return boost::algorithm::ends_with(full_string, ending);
    }

    bool starts_with(const std::string& full_string, const std::string& start)
    {
        return boost::algorithm::starts_with(full_string, start);
    }

    bool is_integer(const std::string& s)
    {
        if (s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+')))
        {
            return false;
        }

        char* p;
        strtol(s.c_str(), &p, 10);

        return (*p == 0);
    }

    bool is_floating_point(const std::string& s)
    {
        std::stringstream ss(s);
        float f;
        ss >> f;
        return (ss.eof() && !ss.fail());
    }

    std::vector<std::string> split(const std::string& s, const char delim, bool obey_brackets)
    {
        std::vector<std::string> result;
        std::string item = "";
        if (obey_brackets)
        {
            int bracket_level = 0;
            for (size_t i = 0; i < s.length(); ++i)
            {
                char c = s.at(i);
                switch (c)
                {
                    case '(':
                    case '{':
                    case '[':
                        ++bracket_level;
                        break;
                    case ')':
                    case '}':
                    case ']':
                        --bracket_level;
                        break;
                    default:
                        break;
                }
                if (bracket_level < 0)
                {
                    bracket_level = 0;
                }
                if (c == delim)
                {
                    // No constant expression, therefore not usable in switch case
                    if (bracket_level == 0)
                    {
                        result.push_back(item);
                        item = "";
                    }
                    else
                    {
                        item.push_back(c);
                    }
                }
                else
                {
                    item.push_back(c);
                }
            }
            if (!item.empty())
            {
                result.push_back(item);
            }
        }
        else
        {
            std::stringstream ss(s);
            while (std::getline(ss, item, delim))
            {
                result.push_back(item);
            }
        }
        if (s.back() == delim)
        {
            result.push_back("");
        }
        return result;
    }

    std::string ltrim(const std::string& line, const char* to_remove)
    {
        size_t start = line.find_first_not_of(to_remove);

        if (start != std::string::npos)
        {
            return line.substr(start, line.size() - start);
        }
        return "";
    }

    std::string rtrim(const std::string& line, const char* to_remove)
    {
        size_t end            = line.find_last_not_of(to_remove);
        if (end != std::string::npos)
        {
            return line.substr(0, end + 1);
        }
        return "";
    }

    std::string trim(const std::string& line, const char* to_remove)
    {
        size_t start = line.find_first_not_of(to_remove);
        size_t end   = line.find_last_not_of(to_remove);

        if (start != std::string::npos)
        {
            return line.substr(start, end - start + 1);
        }
        else
        {
            return "";
        }
    }

    std::string replace(const std::string& str, const std::string& search, const std::string& replace)
    {
        auto s     = str;
        size_t pos = 0;
        if (search.empty())
        {
            return str;
        }    // Just return the original string as we cannot determine what we want to replace.
        while ((pos = s.find(search, pos)) != std::string::npos)
        {
            s.replace(pos, search.length(), replace);
            pos += replace.length();
        }
        return s;
    }

    std::string join(const std::string& joiner, const std::vector<std::string>& items)
    {
        if (items.empty())
        {
            return "";
        }
        std::string s = items[0];
        if (items.size() == 1)
        {
            return s;
        }
        for (size_t i = 1; i < items.size(); ++i)
        {
            s += joiner + items[i];
        }
        return s;
    }

    std::string join(const std::string& joiner, const std::set<std::string>& items)
    {
        return join(joiner, std::vector<std::string>(items.begin(), items.end()));
    }

    std::string to_upper(const std::string& s)
    {
        std::string result = "";
        for (size_t i = 0; i < s.size(); i++)
        {
            result += toupper(s[i]);
        }
        return result;
    }

    std::string to_lower(const std::string& s)
    {
        std::string result = "";
        for (size_t i = 0; i < s.size(); i++)
        {
            result += tolower(s[i]);
        }
        return result;
    }

    u32 num_of_occurrences(const std::string& str, const std::string& substr)
    {
        u32 num_of_occurrences = 0;
        auto position = str.find(substr, 0);
        while (position != std::string::npos) {
            num_of_occurrences++;
            position = str.find(substr, position + 1);
        }
        return num_of_occurrences;
    }


    bool folder_exists_and_is_accessible(const hal::path& folder)
    {
#ifdef _WIN32
        DWORD ftyp = GetFileAttributesA(folder.c_str());
        if (ftyp == INVALID_FILE_ATTRIBUTES)
            return false;    //something is wrong with your path!

        if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
            if (0 == access(folder.c_str(), R_OK))
                return true;

        return false;    // this is not a directory!
#else

        struct stat sb;

        if (stat(folder.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))
        {
            if (0 == access(folder.c_str(), R_OK))
            {
                return true;
            }
        }
        return false;
#endif
    }

    hal::path get_binary_directory()
    {
        char buf[1024] = {0};
#ifdef _WIN32
        DWORD ret = GetModuleFileNameA(NULL, buf, sizeof(buf));
        if (ret == 0 || ret == sizeof(buf))
            return std::string();
        return hal::path(buf);
#elif __APPLE__ && __MACH__
        uint32_t size = sizeof(buf);
        int ret       = _NSGetExecutablePath(buf, &size);
        if (0 != ret)
        {
            return std::string();
        }

        hal::error_code ec;
        hal::path p(hal::fs::canonical(buf, hal::fs::current_path(), ec));
        return p.parent_path().make_preferred();
#elif __linux__
        ssize_t size = readlink("/proc/self/exe", buf, sizeof(buf));
        std::string path(buf, size);
        hal::error_code ec;
        hal::path p(hal::fs::canonical(path, hal::fs::current_path(), ec));
        return p.make_preferred().parent_path();
#endif
    }

    hal::path get_base_directory()
    {
        auto environ_path = std::getenv("HAL_BASE_PATH");
        if (environ_path)
        {
            return hal::path(environ_path).make_preferred();
        }
        hal::error_code ec;
        auto bin_dir = get_binary_directory();
        auto p       = bin_dir / "hal";
        if (hal::fs::exists(p, ec))
        {
            return bin_dir.parent_path();
        }
        hal::path which_result = which("hal");
        if (!which_result.empty())
        {
            return which_result.parent_path().parent_path();
        }
        die("core", "Cannot determine base path of hal installation. Please set the environment variable HAL_BASE_PATH. Giving up!");
    }

    hal::path get_library_directory()
    {
        std::vector<hal::path> path_hints = {
            get_base_directory() / "lib64/",
            get_base_directory() / "lib/x86_64-linux-gnu",
            get_base_directory() / "lib/",
        };
        return get_first_directory_exists(path_hints);
    }

    hal::path get_share_directory()
    {
        std::vector<hal::path> path_hints = {
            get_base_directory() / "share/hal",
        };
        return get_first_directory_exists(path_hints);
    }

    hal::path get_user_share_directory()
    {
        hal::path dir = hal::path(getenv("HOME")) / ".local/share/hal";
        hal::fs::create_directories(dir);
        return dir;
    }

    hal::path get_user_config_directory()
    {
        hal::path dir = hal::path(getenv("HOME")) / ".config/hal";
        hal::fs::create_directories(dir);
        return dir;
    }

    hal::path get_default_log_directory(hal::path source_file)
    {
        hal::path dir = (source_file.empty()) ? get_user_share_directory() / "log" : source_file.parent_path();
        hal::fs::create_directories(dir);
        return dir;
    }

    std::vector<hal::path> get_gate_library_directories()
    {
        std::vector<hal::path> path_hints = {
            get_share_directory() / "gate_libraries",
            get_user_share_directory() / "gate_libraries",
        };
        return path_hints;
    }

    std::vector<hal::path> get_plugin_directories()
    {
        std::vector<hal::path> path_hints = {

            get_library_directory() / "hal_plugins/",
            get_library_directory() / "plugins/",
            get_user_share_directory() / "plugins/",
            hal::path(getenv("HOME")) / "plugins/",
        };
        return path_hints;
    }

    hal::path get_first_directory_exists(std::vector<hal::path> path_hints)
    {
        for (const auto& path : path_hints)
        {
            hal::error_code ec;
            if (hal::fs::exists(path, ec))
            {
                return path;
            }
        }
        return hal::path();
    }

    hal::path get_file(std::string file_name, std::vector<hal::path> path_hints)
    {
        if (file_name.empty())
        {
            return hal::path();
        }

        for (const auto& path : path_hints)
        {
            hal::error_code ec1;
            if (hal::fs::exists(hal::path(path), ec1))
            {
                hal::path file_path = hal::path(path).string() + "/" + hal::path(file_name).string();
                hal::error_code ec2;
                if (hal::fs::exists(file_path, ec2))
                {
                    return file_path;
                }
            }
        }
        return hal::path();
    }

    hal::path which(const std::string& name, const std::string& path)
    {
        if (name.empty())
        {
            return hal::path();
        }
        std::string internal_path = path;
        if (internal_path.empty())
        {
            internal_path = std::getenv("PATH");
        }
#ifdef _WIN32
        const char which_delimiter = ';';
        return hal::path();    // Curerently no support for windows. Sorry ...
#else
        const char which_delimiter = ':';
#endif
        auto folders = split(internal_path, which_delimiter, false);
        for (const auto& folder : folders)
        {
            hal::error_code ec;
            UNUSED(ec);
            hal::path p = hal::path(folder) / name;
            struct stat sb;

            if (stat(p.c_str(), &sb) == 0 && sb.st_mode & S_IXUSR)
            {
                return p;
            }
        }
        return hal::path();
    }

    std::string get_open_source_licenses()
    {
        return R"(buddy 2.4 (https://github.com/jgcoded/BuDDy):
               Copyright (C) 1996-2002 by Jorn Lind-Nielsen
                            All rights reserved

    Permission is hereby granted, without written agreement and without
    license or royalty fees, to use, reproduce, prepare derivative
    works, distribute, and display this software and its documentation
    for any purpose, provided that (1) the above copyright notice and
    the following two paragraphs appear in all copies of the source code
    and (2) redistributions, including without limitation binaries,
    reproduce these notices in the supporting documentation. Substantial
    modifications to this software may be copyrighted by their authors
    and need not follow the licensing terms described here, provided
    that the new terms are clearly indicated in all files where they apply.

    IN NO EVENT SHALL JORN LIND-NIELSEN, OR DISTRIBUTORS OF THIS
    SOFTWARE BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL,
    INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS
    SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE AUTHORS OR ANY OF THE
    ABOVE PARTIES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    JORN LIND-NIELSEN SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
    BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
    FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS
    ON AN "AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO
    OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
    MODIFICATIONS.

=================================================================================

libvcdparse (https://github.com/kmurray/libvcdparse): The MIT License (MIT)

Copyright (c) 2016 Kevin E. Murray

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

=================================================================================

pybind11 (https://github.com/pybind/pybind11):
Copyright (c) 2016 Wenzel Jakob <wenzel.jakob@epfl.ch>, All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Please also refer to the file CONTRIBUTING.md, which clarifies licensing of
external contributions to this project including patches, pull requests, etc.

=================================================================================

rapidjson (https://github.com/Tencent/rapidjson):

Tencent is pleased to support the open source community by making RapidJSON
available.

Copyright (C) 2015 THL A29 Limited, a Tencent company, and Milo Yip.  All
rights reserved.

If you have downloaded a copy of the RapidJSON binary from Tencent, please
note that the RapidJSON binary is licensed under the MIT License. If you have
downloaded a copy of the RapidJSON source code from Tencent, please note that
RapidJSON source code is licensed under the MIT License, except for the
third-party components listed below which are subject to different license
terms.  Your integration of RapidJSON into your own projects may require
compliance with the MIT License, as well as the other licenses applicable to
the third-party components included within RapidJSON. To avoid the problematic
JSON license in your own projects, it's sufficient to exclude the
bin/jsonchecker/ directory, as it's the only code under the JSON license. A
copy of the MIT License is included in this file.

Other dependencies and licenses:

Open Source Software Licensed Under the BSD License:
--------------------------------------------------------------------

The msinttypes r29 Copyright (c) 2006-2013 Alexander Chemeris All rights
reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.
* Neither the name of  copyright holder nor the names of its contributors may
  be used to endorse or promote products derived from this software without
  specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Open Source Software Licensed Under the JSON License:
--------------------------------------------------------------------

json.org Copyright (c) 2002 JSON.org All Rights Reserved.

JSON_checker Copyright (c) 2002 JSON.org All Rights Reserved.


Terms of the JSON License:
---------------------------------------------------

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

The Software shall be used for Good, not Evil.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


Terms of the MIT License:
--------------------------------------------------------------------

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

=================================================================================

sanitizers-cmake (https://github.com/arsenm/sanitizers-cmake):
The MIT License (MIT)

Copyright (c)
    2013 Matthew Arsenault
    2015-2016 RWTH Aachen University, Federal Republic of Germany

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

=================================================================================

spdlog (https://github.com/gabime/spdlog):
The MIT License (MIT)

Copyright (c) 2016 Gabi Melman.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
)";
    }
}    // namespace core_utils
