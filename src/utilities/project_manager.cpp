#include "hal_core/utilities/project_manager.h"

#include <filesystem>
#include <rapidjson/filereadstream.h>
#include <iostream>

#include "hal_core/utilities/log.h"
#include "hal_core/utilities/project_serializer.h"
#include "hal_core/utilities/project_filelist.h"
#include "hal_core/netlist/netlist_factory.h"

const int SERIALIZATION_FORMAT_VERSION = 9;

namespace hal {
    ProjectManager* ProjectManager::inst = nullptr;

    const std::string ProjectManager::s_project_file = ".project.halp";

    ProjectManager::ProjectManager()
        : m_user_provided_directory(false)
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

    void ProjectManager::set_project_directory(const std::string& path)
    {
        m_proj_dir = ProjectDirectory(path);
        m_user_provided_directory = true;
    }

    bool ProjectManager::has_user_provided_directory() const
    {
        return m_user_provided_directory;
    }

    const ProjectDirectory &ProjectManager::get_project_directory() const
    {
        return m_proj_dir;
    }

    bool ProjectManager::create_project_directory() const
    {
        return std::filesystem::create_directory(m_proj_dir);
    }

    void ProjectManager::set_netlist_file(const std::string& fname, Netlist *netlist)
    {
        m_netlist_save = netlist;
        m_netlist_file = fname;
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

    bool ProjectManager::serialize() const
    {
        if (!m_netlist_save) return false;

        std::filesystem::path projFilePath(m_proj_dir);
        projFilePath.append(s_project_file);

        JsonWriteDocument doc;
        doc["serialization_format_version"] = SERIALIZATION_FORMAT_VERSION;
        doc["netlist"] = m_netlist_file;

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
