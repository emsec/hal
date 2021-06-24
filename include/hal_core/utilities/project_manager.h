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
#include <unordered_map>
#include <filesystem>

#include "hal_core/utilities/project_directory.h"
#include "hal_core/utilities/json_write_document.h"
#include "hal_core/netlist/netlist.h"

namespace hal
{
    class Netlist;
    class ProjectSerializer;
    class ProjectFilelist;

    class ProjectManager
    {
    private:
        ProjectManager();

        static ProjectManager* inst;

        Netlist* m_netlist_save;
        std::unique_ptr<Netlist> m_netlist_load;
        ProjectDirectory m_proj_dir;
        bool m_user_provided_directory;
        std::string m_proj_file;
        std::string m_netlist_file;
        std::unordered_map<std::string,ProjectSerializer*> m_serializer;
        std::unordered_map<std::string,ProjectFilelist*> m_filelist;

        void parse_filelist(const std::string& tagname, rapidjson::Value& farray);
    public:
        static ProjectManager* instance();
        void register_serializer(const std::string& tagname, ProjectSerializer* serializer);
        ProjectFilelist* get_filelist(const std::string& tagname);
        std::unique_ptr<Netlist>& get_netlist();
        bool serialize() const;
        bool deserialize();
        void dump() const;
        void set_netlist_file(const std::string& fname, Netlist* netlist);
        void set_project_directory(const std::string& path);
        bool create_project_directory() const;
        bool has_user_provided_directory() const;
        std::string get_netlist_filename() const;
        const ProjectDirectory& get_project_directory() const;

        /**
         * The name of the file with project info (without path)
         */
        static const std::string s_project_file;
    };
}    // namespace hal
