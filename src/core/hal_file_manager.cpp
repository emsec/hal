#include "core/hal_file_manager.h"
#include "core/log.h"

#include <fstream>
#include <sstream>

#include "rapidjson/filereadstream.h"
#include "rapidjson/stringbuffer.h"

#define PRETTY_JSON_OUTPUT 0
#if PRETTY_JSON_OUTPUT == 1
#include "rapidjson/prettywriter.h"
#else
#include "rapidjson/writer.h"
#endif

namespace hal_file_manager
{
    namespace
    {
        callback_hook<bool(const hal::path&, std::shared_ptr<netlist> netlist, rapidjson::Document&)> m_on_serialize_hook;
        callback_hook<bool(const hal::path&, std::shared_ptr<netlist> netlist, rapidjson::Document&)> m_on_deserialize_hook;
    }    // namespace

    bool serialize(const hal::path& file, std::shared_ptr<netlist> netlist, rapidjson::Document& document)
    {
        for (const auto& id : m_on_serialize_hook.get_ids())
        {
            if (!m_on_serialize_hook.call(id, file, netlist, document))
            {
                return false;
            }
        }
        return true;
    }

    bool deserialize(const hal::path& file, std::shared_ptr<netlist> netlist, rapidjson::Document& document)
    {
        for (const auto& id : m_on_deserialize_hook.get_ids())
        {
            if (!m_on_deserialize_hook.call(id, file, netlist, document))
            {
                return false;
            }
        }
        return true;
    }

    void register_on_serialize_callback(std::function<bool(const hal::path&, std::shared_ptr<netlist> netlist, rapidjson::Document&)> callback)
    {
        m_on_serialize_hook.add_callback(callback);
    }

    void register_on_deserialize_callback(std::function<bool(const hal::path&, std::shared_ptr<netlist> netlist, rapidjson::Document&)> callback)
    {
        m_on_deserialize_hook.add_callback(callback);
    }
}    // namespace hal_file_manager
