#include "hal_core/utilities/project_manager.h"

#include <filesystem>
#include <rapidjson/filereadstream.h>
#include <iostream>
#include <fstream>

#include "hal_core/utilities/log.h"
#include "hal_core/utilities/project_serializer.h"
#include "hal_core/utilities/project_filelist.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/netlist/persistent/netlist_serializer.h"

const int SERIALIZATION_FORMAT_VERSION = 9;

namespace hal {
    ProjectManager* ProjectManager::inst = nullptr;

    const std::string ProjectManager::s_project_file = ".project.halp";

    ProjectManager::ProjectManager()
        : m_project_status(None)
    {;}

    ProjectManager* ProjectManager::instance()
    {
        if (!inst) inst = new ProjectManager;
        return inst;
    }

    void ProjectManager::register_serializer(const std::string& tagname, ProjectSerializer* serializer)
    {
        if (m_serializer.find(tagname) == m_serializer.end())
            m_serializer[tagname] = serializer;
        else
        {
            log_warning("project_manager", "serializer '{}' already registered.", tagname);
        }
    }

    ProjectFilelist* ProjectManager::get_filelist(const std::string& tagname)
    {
        auto it = m_filelist.find(tagname);
        if (it==m_filelist.end()) return nullptr;
        return it->second;
    }

    bool ProjectManager::open_project_directory(const std::string& path)
    {
        m_proj_dir = ProjectDirectory(path);
        if (!std::filesystem::exists(m_proj_dir)) return false;
        if (deserialize())
        {
            m_project_status = Opened;
            return true;
        }
        return false;
    }

    const ProjectDirectory &ProjectManager::get_project_directory() const
    {
        return m_proj_dir;
    }

    bool ProjectManager::create_project_directory(const std::string& path)
    {
        m_proj_dir = ProjectDirectory(path);
        if (std::filesystem::exists(m_proj_dir)) return false;
        if (!std::filesystem::create_directory(m_proj_dir)) return false;
        m_netlist_file = m_proj_dir.get_default_filename(".hal");
        return yserialize();
    }

    ProjectManager::ProjectStatus ProjectManager::get_project_status() const
    {
        return m_project_status;
    }

    void ProjectManager::set_gatelib_path(const std::string &glpath)
    {
        m_gatelib_path = glpath;
    }

    bool ProjectManager::serialize_netlist(Netlist* netlist, bool shadow, const std::string& fname)
    {
        if (!netlist) return false;

        m_netlist_save = netlist;
        if (fname.empty())
        {
            if (shadow)
                m_netlist_file = m_proj_dir.get_shadow_filename(".hal");
            else
                m_netlist_file = m_proj_dir.get_default_filename(".hal");
        }
        else
        {
            m_netlist_file = fname;
        }

        if (!netlist_serializer::xserialize_to_file(m_netlist_save, m_netlist_file)) return false;

        return yserialize();
    }

    std::string ProjectManager::get_netlist_filename() const
    {
        std::filesystem::path filename(m_proj_dir);
        filename.append(m_netlist_file);
        return filename.string();
    }

    std::unique_ptr<Netlist>& ProjectManager::get_netlist()
    {
        return m_netlist_load;
    }

    bool ProjectManager::deserialize()
    {
        std::filesystem::path projFilePath(m_proj_dir);
        projFilePath.append(s_project_file);

        FILE* fp = fopen(projFilePath.string().c_str(), "r");
        if (fp == NULL)
        {
//            log_error("hgl_parser", "unable to open '{}' for reading.", file_path.string());
            log_error("project_manager", "cannot open project file '{}'.", projFilePath.string());
            return false;
        }

        char buffer[65536];
        rapidjson::FileReadStream frs(fp, buffer, sizeof(buffer));
        rapidjson::Document doc;
        doc.ParseStream<0, rapidjson::UTF8<>, rapidjson::FileReadStream>(frs);
        fclose(fp);

        if (doc.HasMember("netlist"))
        {
            m_netlist_file = doc["netlist"].GetString();
            std::filesystem::path netlistPath(m_proj_dir);
            netlistPath.append(m_netlist_file);
            m_netlist_load = netlist_factory::load_netlist(netlistPath.string());
        }
        else return false;

        if (doc.HasMember("serializer"))
        {
            for(auto it = doc["serializer"].MemberBegin(); it!= doc["serializer"].MemberEnd(); ++it)
            {
                parse_filelist(it->name.GetString(),it->value);
            }
        }

        for (auto it = m_serializer.begin(); it != m_serializer.end(); ++it)
        {
            it->second->deserialize(m_netlist_load.get(), m_proj_dir);
        }
        return true;
    }

    void ProjectManager::parse_filelist(const std::string& tagname, rapidjson::Value& farray)
    {
        ProjectFilelist* flist = new ProjectFilelist(tagname);
        for (rapidjson::Document::ConstValueIterator it = farray.Begin(); it != farray.End(); ++it)
        {
            flist->push_back(it->GetString());
        }
        m_filelist[tagname] = flist;
    }

    bool ProjectManager::yserialize() const
    {
        if (!m_netlist_save) return false;

        std::filesystem::path projFilePath(m_proj_dir);
        projFilePath.append(s_project_file);

        JsonWriteDocument doc;
        doc["serialization_format_version"] = SERIALIZATION_FORMAT_VERSION;
        doc["netlist"] = m_netlist_file;
        doc["gate_library"] = m_gatelib_path;

        JsonWriteObject& serial = doc.add_object("serializer");
        for (auto it = m_serializer.begin(); it != m_serializer.end(); ++it)
        {
            ProjectFilelist* pfl = it->second->serialize(m_netlist_save, m_proj_dir);
            if (!pfl) continue;
            JsonWriteArray& jsarray = serial.add_array(it->first);
            for (const std::string& fname : *pfl)
                jsarray << fname;
            jsarray.close();
        }
        serial.close();
        doc.serialize(projFilePath.string());

        return true;
    }

    void ProjectManager::dump() const
    {
        for (auto it = m_filelist.begin(); it != m_filelist.end(); ++it)
        {
            std::cout << "serializer: <" << it->first << ">" << std::endl;
            for (const std::string& fname : *(it->second))
            {
                std::cout << "   "  << fname << std::endl;
            }
            std::cout << "=========" << std::endl;
        }
    }
}
