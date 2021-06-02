#include "hal_core/utilities/project_manager.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/stringbuffer.h"

#include <iostream>
#include <fstream>
#include <filesystem>

#define PRETTY_JSON_OUTPUT 0
#if PRETTY_JSON_OUTPUT == 1
#include "rapidjson/prettywriter.h"
#else
#include "rapidjson/writer.h"
#endif

namespace hal {
    ProjectManager* ProjectManager::inst = nullptr;

    ProjectManager::ProjectManager()
    {
    }

    ProjectManager* ProjectManager::instance()
    {
        if (!inst) inst = new ProjectManager;
        return inst;
    }

    bool ProjectManager::serialize() const
    {
        std::filesystem::path currentDir = std::filesystem::current_path();

            std::ofstream hal_file_stream;
            hal_file_stream.open(hal_file.string());
            if (hal_file_stream.fail())
            {
                log_error("hdl_writer", "Cannot open or create file {}. Please verify that the file and the containing directory is writable!", hal_file.string());
                return false;
            }

            rapidjson::Document document;
            document.SetObject();

            document.AddMember("serialization_format_version", SERIALIZATION_FORMAT_VERSION, document.GetAllocator());

            serialize(nl, document);

            if (!hal_file_manager::serialize(hal_file, nl, document))
            {
                log_info("netlist_persistent", "serialization failed");
                return false;
            }

            rapidjson::StringBuffer strbuf;
#if PRETTY_JSON_OUTPUT == 1
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(strbuf);
#else
            rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
#endif
            document.Accept(writer);

            hal_file_stream << strbuf.GetString();

            hal_file_stream.close();

            log_info("netlist_persistent", "serialized netlist in {:2.2f} seconds", DURATION(begin_time));

            return true;
        }

    }
}
