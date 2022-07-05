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
        std::unordered_map<std::string,std::string> m_filename;

        bool serialize_external(bool shadow);

        /**
         * Internal method to deserialize hal project, called by open_project()
         *
         * @return true if OK, false if any error
         */
        bool deserialize();

        /**
         * Serialize project file summary to .project.json
         * @param[in] shadow true if called from autosave procedure
         * @return true if project file was written successfully, false otherwise
         */
        bool serialize_to_projectfile(bool shadow) const;

    public:
        /**
         * Returns the singleton instance which gets constructed upon first call
         * @return the singleton instance
         */
        static ProjectManager* instance();

        /**
         * Registers an external serializer identified by unique tagname
         *
         * @param[in] tagname unique tagname of serializer
         * @param[in] serializer serializer instance which must be derived from ProjectSerializer
         */
        void register_serializer(const std::string& tagname, ProjectSerializer* serializer);

        /**
         * Unregisters external serializer identified by tagname
         * @param[in] tagname unique tagname of serializer
         */
        void unregister_serializer(const std::string& tagname);

        /**
         * Returns the current project status (None, Opened, Saved)
         *
         * @return project status
         */
        ProjectStatus get_project_status() const;

        /**
         * Set current project status to new value (None, Opened, Saved).
         * Must be called when project got closed.
         *
         * @param[in] stat New project status value.
         */
        void set_project_status(ProjectStatus stat);

        /**
         * Returns name of file to be parsed by external serializer
         *
         * @param[in] tagname unique tagname of serializer
         * @return relative file name
         */
        std::string get_filename(const std::string& tagname);

        /**
         * Returns parsed netlist which is (temporarily) owned by project manager
         *
         * @return unique pointer to netlist
         */
        std::unique_ptr<Netlist>& get_netlist();

        /**
         * Dump for debugging purpose
         */
        void dump() const;

        /**
         * Set gate library path name
         *
         * @param[in] glpath path to gate library
         */
        void set_gatelib_path(const std::string& glpath);

        /**
         * Serialize netlist and dependend data to project directory
         *
         * @param[in] netlist Netlist to save
         * @param[in] shadow true if called from autosave procedure
         * @return true if serialization of core netlist was successful, false otherwise
         */
        bool serialize_project(Netlist* netlist, bool shadow = false);

        /**
         * Open hal project in directory <path>
         *
         * @param[in] path to project directory, might be empty if previously set by set_project_directory()
         * @return true on success, false on error
         */
        bool open_project(const std::string& path = std::string());

        /**
         * Returns project directory
         *
         * @return project directory
         */
        const ProjectDirectory& get_project_directory() const;

        /**
         * Set project directory
         *
         * @param[in] path to project directory
         */
        void set_project_directory(const std::string& path);

        /**
         * When copying files from autosave in project directory netlist filename needs to be adjusted.
         */
        void restore_project_file_from_autosave();

        /**
         * Create project directory. Project directory must not exist
         *
         * @param[in] path to project directory
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
         * File name of project info file (mandatory for all hal projects)
         */
        static const std::string s_project_file;
    };
}    // namespace hal
