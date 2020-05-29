#include "core/utils.h"

#include "core/log.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/predef.h>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <sstream>
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
    bool ends_with(const std::string& s, const std::string& ending)
    {
        return ends_with_t<std::string>(s, ending);
    }

    bool starts_with(const std::string& s, const std::string& start)
    {
        return starts_with_t<std::string>(s, start);
    }

    bool equals_ignore_case(const std::string& a, const std::string& b)
    {
        return boost::algorithm::iequals(a, b);
    }

    bool is_integer(const std::string& s)
    {
        return is_integer_t<std::string>(s);
    }

    bool is_floating_point(const std::string& s)
    {
        return is_floating_point_t<std::string>(s);
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
        size_t end = line.find_last_not_of(to_remove);
        if (end != std::string::npos)
        {
            return line.substr(0, end + 1);
        }
        return "";
    }

    std::string trim(const std::string& s, const char* to_remove)
    {
        return trim_t<std::string>(s, to_remove);
    }

    std::string replace(const std::string& str, const std::string& search, const std::string& replace)
    {
        return replace_t<std::string>(str, search, replace);
    }

    std::string to_upper(const std::string& s)
    {
        return to_upper_t<std::string>(s);
    }

    std::string to_lower(const std::string& s)
    {
        return to_lower_t<std::string>(s);
    }

    u32 num_of_occurrences(const std::string& str, const std::string& substr)
    {
        u32 num_of_occurrences = 0;
        auto position          = str.find(substr, 0);
        while (position != std::string::npos)
        {
            num_of_occurrences++;
            position = str.find(substr, position + 1);
        }
        return num_of_occurrences;
    }

    bool file_exists(const std::string& filename)
    {
        std::ifstream ifile(filename.c_str());
        return (bool)ifile;
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
        hal::path p(hal::fs::canonical(buf, ec));
        return p.parent_path().make_preferred();
#elif __linux__
        ssize_t size = readlink("/proc/self/exe", buf, sizeof(buf));
        std::string path(buf, size);
        hal::error_code ec;
        hal::path p(hal::fs::canonical(path, ec));
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

    hal::path get_config_directory()
    {
        std::vector<hal::path> path_hints = {
            get_base_directory() / "share/hal/defaults",
        };
        return get_first_directory_exists(path_hints);
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
        return R"(pybind11 (https://github.com/pybind/pybind11):
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

=================================================================================

Boost (https://www.boost.org/)
Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

=================================================================================

Qt (https://www.qt.io/)
            GNU LESSER GENERAL PUBLIC LICENSE

 The Qt Toolkit is Copyright (C) 2015 The Qt Company Ltd.
 Contact: http://www.qt.io/licensing/

 You may use, distribute and copy the Qt Toolkit under the terms of
 GNU Lesser General Public License version 3, which is displayed below.
 This license makes reference to the version 3 of the GNU General
 Public License, which you can find in the LICENSE.GPLv3 file.

-------------------------------------------------------------------------

            GNU LESSER GENERAL PUBLIC LICENSE
                Version 3, 29 June 2007

 Copyright © 2007 Free Software Foundation, Inc. <http://fsf.org/>
Everyone is permitted to copy and distribute verbatim copies of this
licensedocument, but changing it is not allowed.

This version of the GNU Lesser General Public License incorporates
the terms and conditions of version 3 of the GNU General Public
License, supplemented by the additional permissions listed below.

0. Additional Definitions.

 As used herein, “this License” refers to version 3 of the GNU Lesser
General Public License, and the “GNU GPL” refers to version 3 of the
GNU General Public License.

 “The Library” refers to a covered work governed by this License,
other than an Application or a Combined Work as defined below.

 An “Application” is any work that makes use of an interface provided
by the Library, but which is not otherwise based on the Library.
Defining a subclass of a class defined by the Library is deemed a mode
of using an interface provided by the Library.

 A “Combined Work” is a work produced by combining or linking an
Application with the Library. The particular version of the Library
with which the Combined Work was made is also called the “Linked
Version”.

 The “Minimal Corresponding Source” for a Combined Work means the
Corresponding Source for the Combined Work, excluding any source code
for portions of the Combined Work that, considered in isolation, are
based on the Application, and not on the Linked Version.

 The “Corresponding Application Code” for a Combined Work means the
object code and/or source code for the Application, including any data
and utility programs needed for reproducing the Combined Work from the
Application, but excluding the System Libraries of the Combined Work.

1. Exception to Section 3 of the GNU GPL.

 You may convey a covered work under sections 3 and 4 of this License
without being bound by section 3 of the GNU GPL.

2. Conveying Modified Versions.

 If you modify a copy of the Library, and, in your modifications, a
facility refers to a function or data to be supplied by an Application
that uses the facility (other than as an argument passed when the
facility is invoked), then you may convey a copy of the modified
version:

    a) under this License, provided that you make a good faith effort
    to ensure that, in the event an Application does not supply the
    function or data, the facility still operates, and performs
    whatever part of its purpose remains meaningful, or

    b) under the GNU GPL, with none of the additional permissions of
    this License applicable to that copy.

3. Object Code Incorporating Material from Library Header Files.

 The object code form of an Application may incorporate material from
a header file that is part of the Library. You may convey such object
code under terms of your choice, provided that, if the incorporated
material is not limited to numerical parameters, data structure
layouts and accessors, or small macros, inline functions and templates
(ten or fewer lines in length), you do both of the following:

    a) Give prominent notice with each copy of the object code that
    the Library is used in it and that the Library and its use are
    covered by this License.

    b) Accompany the object code with a copy of the GNU GPL and this
    license document.

4. Combined Works.

 You may convey a Combined Work under terms of your choice that, taken
together, effectively do not restrict modification of the portions of
the Library contained in the Combined Work and reverse engineering for
debugging such modifications, if you also do each of the following:

    a) Give prominent notice with each copy of the Combined Work that
    the Library is used in it and that the Library and its use are
    covered by this License.

    b) Accompany the Combined Work with a copy of the GNU GPL and this
    license document.

    c) For a Combined Work that displays copyright notices during
    execution, include the copyright notice for the Library among
    these notices, as well as a reference directing the user to the
    copies of the GNU GPL and this license document.

    d) Do one of the following:

        0) Convey the Minimal Corresponding Source under the terms of
        this License, and the Corresponding Application Code in a form
        suitable for, and under terms that permit, the user to
        recombine or relink the Application with a modified version of
        the Linked Version to produce a modified Combined Work, in the
        manner specified by section 6 of the GNU GPL for conveying
        Corresponding Source.

        1) Use a suitable shared library mechanism for linking with
        the Library. A suitable mechanism is one that (a) uses at run
        time a copy of the Library already present on the user's
        computer system, and (b) will operate properly with a modified
        version of the Library that is interface-compatible with the
        Linked Version.

    e) Provide Installation Information, but only if you would
    otherwise be required to provide such information under section 6
    of the GNU GPL, and only to the extent that such information is
    necessary to install and execute a modified version of the
    Combined Work produced by recombining or relinking the Application
    with a modified version of the Linked Version. (If you use option
    4d0, the Installation Information must accompany the Minimal
    Corresponding Source and Corresponding Application Code. If you
    use option 4d1, you must provide the Installation Information in
    the manner specified by section 6 of the GNU GPL for conveying
    Corresponding Source.)

5. Combined Libraries.

 You may place library facilities that are a work based on the Library
side by side in a single library together with other library
facilities that are not Applications and are not covered by this
License, and convey such a combined library under terms of your
choice, if you do both of the following:

    a) Accompany the combined library with a copy of the same work
    based on the Library, uncombined with any other library
    facilities, conveyed under the terms of this License.

    b) Give prominent notice with the combined library that part of
    it is a work based on the Library, and explaining where to find
    the accompanying uncombined form of the same work.

6. Revised Versions of the GNU Lesser General Public License.

 The Free Software Foundation may publish revised and/or new versions
of the GNU Lesser General Public License from time to time. Such new
versions will be similar in spirit to the present version, but may
differ in detail to address new problems or concerns.

Each version is given a distinguishing version number. If the Library
as you received it specifies that a certain numbered version of the
GNU Lesser General Public License “or any later version” applies to
it, you have the option of following the terms and conditions either
of that published version or of any later version published by the
Free Software Foundation. If the Library as you received it does not
specify a version number of the GNU Lesser General Public License,
you may choose any version of the GNU Lesser General Public License
ever published by the Free Software Foundation.

If the Library as you received it specifies that a proxy can decide
whether future versions of the GNU Lesser General Public License shall
apply, that proxy's public statement of acceptance of any version is
permanent authorization for you to choose that version for the Library.
)";
    }
}    // namespace core_utils
