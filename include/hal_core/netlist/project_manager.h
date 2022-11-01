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
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/json_write_document.h"
#include "hal_core/utilities/project_directory.h"

#include <filesystem>
#include <string>
#include <unordered_map>

namespace hal
{
    class Netlist;
    class ProjectSerializer;

    /**
     * Project manager class that handles opening, closing, and saving of projects.
     * 
     * @ingroup persistent 
     */
    class ProjectManager
    {
    public:
        /**
         * Represents the current status of the project.
         */
        enum ProjectStatus
        {
            NONE,   /**< Represents the default state. */
            OPENED, /**< Represents an open project state. */
            SAVED   /**< Represents a saved project state. */
        };

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
        std::unordered_map<std::string, ProjectSerializer*> m_serializer;
        std::unordered_map<std::string, std::string> m_filename;

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
         * Returns the singleton instance which gets constructed upon first call.
         * 
         * @return the singleton instance.
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
         * Returns the current project status.
         *
         * @return The project status value.
         */
        ProjectStatus get_project_status() const;

        /**
         * Set the current project status to a new value.
         * Must be called when a project is closed.
         *
         * @param[in] status - The new project status value.
         */
        void set_project_status(ProjectStatus status);

        /**
         * Returns the relative path of the file to be parsed by an external serializer.
         *
         * @param[in] serializer_name - The unique name of the serializer.
         * @return The relative file path.
         */
        std::string get_filename(const std::string& serializer_name);

        /**
         * Returns parsed netlist which is (temporarily) owned by project manager.
         *
         * @return unique pointer to netlist
         */
        std::unique_ptr<Netlist>& get_netlist();

        /**
         * Dump for debugging purpose
         */
        void dump() const;

        /**
         * Set the path to the gate library file.
         *
         * @param[in] gl_path - The path to the gate library file.
         */
        void set_gate_library_path(const std::string& gl_path);

        /**
         * Serialize the netlist and all dependent data to the project directory.
         *
         * @param[in] netlist - The netlist.
         * @param[in] shadow - Set to `true` if function is called from autosave procedure, `false` otherwise. Defaults to `false`.
         * @return `true` if serialization of the netlist was successful, `false` otherwise.
         */
        bool serialize_project(Netlist* netlist, bool shadow = false);

        /**
         * Open the project specified by the provided directory path.
         *
         * @param[in] path - The path to the project directory. Can be omitted if the path was previously set using `ProjectManager::set_project_directory`.
         * @return `true` on success, `false` otherwise.
         */
        bool open_project(const std::string& path = "");

        /**
         * Returns project directory.
         *
         * @return project directory
         */
        const ProjectDirectory& get_project_directory() const;

        /**
         * Set path to the project directory.
         *
         * @param[in] path - The path to the project directory.
         */
        void set_project_directory(const std::string& path);

        /**
         * When copying files from autosave in project directory netlist filename needs to be adjusted.
         */
        void restore_project_file_from_autosave();

        /**
         * Create an empty project directory at the specified location. 
         * The project directory must not exist.
         *
         * @param[in] path - The path to the new project directory.
         * @return `true` on success, `false` otherwise.
         */
        bool create_project_directory(const std::string& path);

        /**
         * Remove the existing project directory and clear the path member variable.
         * 
         * @return `true` on success, `false` otherwise.
         */
        bool remove_project_directory();

        /**
         * Returns the path to the netlist file.
         *
         * @return The netlist file path.
         */
        std::string get_netlist_filename() const;

        /**
         * File name of project info file (mandatory for all hal projects)
         */
        static const std::string s_project_file;
    };
}    // namespace hal
