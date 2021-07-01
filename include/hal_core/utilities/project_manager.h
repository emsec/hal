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
    public:
        enum ProjectStatus {None, Opened, Saved};
    private:
        ProjectManager();

        static ProjectManager* inst;

        ProjectStatus m_project_status;
        Netlist* m_netlist_save;
        std::unique_ptr<Netlist> m_netlist_load;
        ProjectDirectory m_proj_dir;
        std::string m_proj_file;
        std::string m_netlist_file;
        std::string m_gatelib_path;
        std::unordered_map<std::string,ProjectSerializer*> m_serializer;
        std::unordered_map<std::string,ProjectFilelist*> m_filelist;

        void parse_filelist(const std::string& tagname, rapidjson::Value& farray);
    public:
        /**
         * Returns the singleton instance which gets constructed upon first call
         * @return the singleton instance
         */
        static ProjectManager* instance();

        /**
         * Registers an external serializer identified by unique tagname
         *
         * @param tagname unique tagname of serializer
         * @param serializer serializer instance which must be derived from ProjectSerializer
         */
        void register_serializer(const std::string& tagname, ProjectSerializer* serializer);

        /**
         * Returns the current project status (None, Opened, Saved)
         *
         * @return project status
         */
        ProjectStatus get_project_status() const;

        /**
         * Returns list of files to be parsed by external serializer
         *
         * @param tagname unique tagname of serializer
         * @return list of files
         */
        ProjectFilelist* get_filelist(const std::string& tagname);

        /**
         * Returns parsed netlist which is (temporarily) owned by project manager
         *
         * @return unique pointer to netlist
         */
        std::unique_ptr<Netlist>& get_netlist();

        /**
         * Serialize hal project
         *
         * @return true if OK, false if any error
         */
        bool yserialize() const;

        /**
         * Deserialize hal project
         *
         * @return true if OK, false if any error
         */
        bool deserialize();

        /**
         * Dump for debugging purpose
         */
        void dump() const;

        /**
         * Set gate library path name
         *
         * @param fname name of gate library path
         */
        void set_gatelib_path(const std::string& glpath);

        /**
         * Serialize netlist to project directory
         *
         * @param netlist Netlist to save
         * @param shadow true if called from autosave procedure
         * @param fname user provided filename. Use default filename if empty
         */
        bool serialize_netlist(Netlist* netlist, bool shadow = false, const std::string& fname = std::string());

        /**
         * Open project in directory path
         *
         * @param path to project directory
         * @return true on success, false on error
         */
        bool open_project_directory(const std::string& path);

        /**
         * Create project directory. Project directory must not exist
         *
         * @param path to project directory
         * @return true on success, false on error
         */
        bool create_project_directory(const std::string& path);

        /**
         * Getter for netlist filename
         *
         * @return the netlist filename
         */
        std::string get_netlist_filename() const;

        /**
         * Returns project directory
         *
         * @return project directory
         */
        const ProjectDirectory& get_project_directory() const;

        /**
         * The name of the file with project info (without path)
         */
        static const std::string s_project_file;
    };
}    // namespace hal
