#include "hal_core/netlist/project_manager.h"

#include <filesystem>
#include <rapidjson/filereadstream.h>
#include <rapidjson/stringbuffer.h>
#include <iostream>
#include <fstream>

#include "hal_core/utilities/log.h"
#include "hal_core/netlist/project_serializer.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/netlist/persistent/netlist_serializer.h"
#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/netlist.h"

#define PRETTY_JSON_OUTPUT 0
#if PRETTY_JSON_OUTPUT == 1
#include "rapidjson/prettywriter.h"
#else
#include "rapidjson/writer.h"
#endif

const int SERIALIZATION_FORMAT_VERSION = 10;

namespace hal {
    ProjectManager* ProjectManager::inst = nullptr;

    const std::string ProjectManager::s_project_file = ".project.json";

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

    void ProjectManager::unregister_serializer(const std::string &tagname)
    {
        auto it = m_serializer.find(tagname);
        if (it != m_serializer.end())
            m_serializer.erase(it);
    }

    std::string ProjectManager::get_filename(const std::string& tagname)
    {
        auto it = m_filename.find(tagname);
        if (it==m_filename.end()) return std::string();
        return it->second;
    }

    bool ProjectManager::open_project(const std::string& path)
    {
        if (!path.empty()) m_proj_dir = ProjectDirectory(path);
        if (!std::filesystem::exists(m_proj_dir)) return false;
        if (deserialize())
        {
            m_project_status = Opened;
            return true;
        }
        return false;
    }

    void ProjectManager::set_project_directory(const std::string& path)
    {
        m_proj_dir = ProjectDirectory(path);
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

        std::filesystem::create_directory(m_proj_dir.get_filename("py"));
        std::filesystem::create_directory(m_proj_dir.get_filename(ProjectDirectory::s_shadow_dir));
        return serialize_to_projectfile(false);
    }

    bool ProjectManager::remove_project_directory()
    {
        if (m_proj_dir.empty()) return false;
        return std::filesystem::remove_all(m_proj_dir);
    }

    void ProjectManager::set_project_status(ProjectStatus stat)
    {
        m_project_status = stat;
    }

    ProjectManager::ProjectStatus ProjectManager::get_project_status() const
    {
        return m_project_status;
    }

    void ProjectManager::set_gatelib_path(const std::string &glpath)
    {
        m_gatelib_path = glpath;
    }

    bool ProjectManager::serialize_project(Netlist* netlist, bool shadow)
    {
        if (!netlist) return false;

        m_netlist_save = netlist;
        const GateLibrary* gl = m_netlist_save->get_gate_library();
        if (gl) m_gatelib_path = gl->get_path().string();
        if (shadow)
            m_netlist_file = m_proj_dir.get_shadow_filename(".hal");
        else
            m_netlist_file = m_proj_dir.get_default_filename(".hal");

        if (!netlist_serializer::serialize_to_file(m_netlist_save, m_netlist_file)) return false;

        if (!serialize_external(shadow)) return false;

        return serialize_to_projectfile(shadow);
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

    void ProjectManager::restore_project_file_from_autosave()
    {
        std::filesystem::path projFilePath(m_proj_dir);
        projFilePath.append(s_project_file);

        FILE* fp = fopen(projFilePath.string().c_str(), "r");
        if (fp == NULL) return;

        char buffer[65536];
        rapidjson::FileReadStream frs(fp, buffer, sizeof(buffer));
        rapidjson::Document doc;
        doc.ParseStream<0, rapidjson::UTF8<>, rapidjson::FileReadStream>(frs);
        fclose(fp);

        if (doc.HasMember("netlist"))
        {
            rapidjson::Value::MemberIterator netlistMember = doc.FindMember("netlist");
            std::string netlistFilename = netlistMember->value.GetString();
            int n = ProjectDirectory::s_shadow_dir.size() + 1;
            if (netlistFilename.substr(0,n) == ProjectDirectory::s_shadow_dir + '/')
            {
                netlistFilename.erase(0,n);
                netlistMember->value.SetString(netlistFilename.c_str(), doc.GetAllocator());

                std::ofstream of(projFilePath);
                if (!of.good()) return;

                rapidjson::StringBuffer strbuf;
#if PRETTY_JSON_OUTPUT == 1
                rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(strbuf);
#else
                rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
#endif
                doc.Accept(writer);
                of << strbuf.GetString();
                of.close();
            }
        }
    }

    bool ProjectManager::deserialize()
    {
        std::filesystem::path projFilePath(m_proj_dir);
        projFilePath.append(s_project_file);

        FILE* fp = fopen(projFilePath.string().c_str(), "r");
        if (fp == NULL)
        {
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
            m_netlist_load = netlist_factory::load_netlist(netlistPath);
            if (!m_netlist_load)
            {
                log_error("project_manager", "cannot load netlist {}.", netlistPath.string());
                return false;
            }
        }
        else
        {
            log_error("project_manager", "no 'netlist' token found in project file {}.", projFilePath.string());
            return false;
        }

        if (doc.HasMember("serializer"))
        {
            for(auto it = doc["serializer"].MemberBegin(); it!= doc["serializer"].MemberEnd(); ++it)
            {
                m_filename[it->name.GetString()] = it->value.GetString();
            }
        }

        for (auto it = m_serializer.begin(); it != m_serializer.end(); ++it)
        {
            it->second->deserialize(m_netlist_load.get(), m_proj_dir);
        }
        return true;
    }

    bool ProjectManager::serialize_external(bool shadow)
    {
        if (!m_netlist_save) return false;

        m_filename.clear();

        for (auto it = m_serializer.begin(); it != m_serializer.end(); ++it)
        {
            std::string relfile = it->second->serialize(m_netlist_save, shadow
                                                         ? m_proj_dir.get_filename(ProjectDirectory::s_shadow_dir)
                                                         : m_proj_dir, shadow);
            if (relfile.empty()) continue;
            m_filename[it->first] = relfile;
        }
        return true;
    }


    bool ProjectManager::serialize_to_projectfile(bool shadow) const
    {
        std::filesystem::path projFilePath = m_proj_dir.get_filename(s_project_file);
        if (shadow)
        {
            std::filesystem::path shadowPath = m_proj_dir.get_filename(ProjectDirectory::s_shadow_dir);
            std::filesystem::create_directory(shadowPath);
            projFilePath = shadowPath;
            projFilePath.append(s_project_file);
        }

        JsonWriteDocument doc;
        doc["serialization_format_version"] = SERIALIZATION_FORMAT_VERSION;
        doc["netlist"]      = m_proj_dir.get_relative_file_path(m_netlist_file).string();
        doc["gate_library"] = m_proj_dir.get_relative_file_path(m_gatelib_path).string();

        if (!m_filename.empty())
        {
             JsonWriteObject& serial = doc.add_object("serializer");
             for (auto it = m_filename.begin(); it != m_filename.end(); ++it)
             {
                 if (it->second.empty()) continue;
                 serial[it->first] = it->second;
             }
             serial.close();
        }
        return doc.serialize(projFilePath.string());
    }

    void ProjectManager::dump() const
    {
        for (auto it = m_filename.begin(); it != m_filename.end(); ++it)
        {
            std::cout << "serializer: <" << it->first << "> <" << it->second << ">" << std::endl;
        }
    }
}
