#include "netlist/persistent/netlist_serializer.h"

#include "netlist/gate.h"
#include "netlist/module.h"
#include "netlist/net.h"
#include "netlist/netlist.h"

#include "netlist/event_system/gate_event_handler.h"
#include "netlist/event_system/module_event_handler.h"
#include "netlist/event_system/net_event_handler.h"
#include "netlist/event_system/netlist_event_handler.h"

#include "netlist/gate_library/gate_library_manager.h"

#include "core/log.h"

#define RAPIDJSON_HAS_STDSTRING 1
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/stringbuffer.h"

#define PRETTY_JSON_OUTPUT 0
#if PRETTY_JSON_OUTPUT == 1
#include "rapidjson/prettywriter.h"
#else
#include "rapidjson/writer.h"
#endif

#include <chrono>
#include <fstream>
#include <sstream>

#ifndef DURATION
#define DURATION(begin_time) (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - begin_time).count() / 1000
#endif

namespace netlist_serializer
{
    // serializing functions
    namespace
    {
        const int SERIALIZON_FORMAT_VERSION = 1;

#define JSON_STR_HELPER(x) rapidjson::Value{}.SetString(x.c_str(), x.length(), allocator)
        rapidjson::Value serialize(std::map<std::tuple<std::string, std::string>, std::tuple<std::string, std::string>> data, rapidjson::Document::AllocatorType& allocator)
        {
            rapidjson::Value val(rapidjson::kArrayType);
            for (const auto& it : data)
            {
                rapidjson::Value entry(rapidjson::kArrayType);
                entry.PushBack(JSON_STR_HELPER(std::get<0>(it.first)), allocator);
                entry.PushBack(JSON_STR_HELPER(std::get<1>(it.first)), allocator);
                entry.PushBack(JSON_STR_HELPER(std::get<0>(it.second)), allocator);
                entry.PushBack(JSON_STR_HELPER(std::get<1>(it.second)), allocator);
                val.PushBack(entry, allocator);
            }
            return val;
        }

        rapidjson::Value serialize(endpoint ep, rapidjson::Document::AllocatorType& allocator)
        {
            rapidjson::Value val(rapidjson::kObjectType);
            val.AddMember("gate_id", ep.gate->get_id(), allocator);
            val.AddMember("pin_type", ep.pin_type, allocator);
            return val;
        }

        rapidjson::Value serialize(std::shared_ptr<gate> g, rapidjson::Document::AllocatorType& allocator)
        {
            rapidjson::Value val(rapidjson::kObjectType);
            val.AddMember("id", g->get_id(), allocator);
            val.AddMember("name", g->get_name(), allocator);
            val.AddMember("type", g->get_type(), allocator);
            val.AddMember("data", serialize(g->get_data(), allocator), allocator);
            return val;
        }

        rapidjson::Value serialize(std::shared_ptr<net> n, rapidjson::Document::AllocatorType& allocator)
        {
            rapidjson::Value val(rapidjson::kObjectType);
            val.AddMember("id", n->get_id(), allocator);
            val.AddMember("name", n->get_name(), allocator);

            if (n->get_src().gate != nullptr)
            {
                val.AddMember("src", serialize(n->get_src(), allocator), allocator);
            }

            {
                rapidjson::Value dsts(rapidjson::kArrayType);
                auto sorted = n->get_dsts();
                std::sort(sorted.begin(), sorted.end(), [](const endpoint& lhs, const endpoint& rhs) { return lhs.gate->get_id() < rhs.gate->get_id(); });
                for (const auto& dst : sorted)
                {
                    dsts.PushBack(serialize(dst, allocator), allocator);
                }
                val.AddMember("dsts", dsts, allocator);
            }

            val.AddMember("data", serialize(n->get_data(), allocator), allocator);
            return val;
        }

        rapidjson::Value serialize(std::shared_ptr<module> m, rapidjson::Document::AllocatorType& allocator)
        {
            rapidjson::Value val(rapidjson::kObjectType);
            val.AddMember("id", m->get_id(), allocator);
            val.AddMember("name", m->get_name(), allocator);
            if (m->get_parent_module() == nullptr)
            {
                val.AddMember("parent", 0, allocator);
            }
            else
            {
                val.AddMember("parent", m->get_parent_module()->get_id(), allocator);
            }
            {
                rapidjson::Value gates(rapidjson::kArrayType);
                auto to_sort = m->get_gates(DONT_CARE, DONT_CARE, false);
                std::vector<std::shared_ptr<gate>> sorted(to_sort.begin(), to_sort.end());
                std::sort(sorted.begin(), sorted.end(), [](const std::shared_ptr<gate>& lhs, const std::shared_ptr<gate>& rhs) { return lhs->get_id() < rhs->get_id(); });
                for (const auto& g : sorted)
                {
                    gates.PushBack(g->get_id(), allocator);
                }
                val.AddMember("gates", gates, allocator);
            }

            {
                rapidjson::Value nets(rapidjson::kArrayType);
                auto to_sort = m->get_nets(DONT_CARE, false);
                std::vector<std::shared_ptr<net>> sorted(to_sort.begin(), to_sort.end());
                std::sort(sorted.begin(), sorted.end(), [](const std::shared_ptr<net>& lhs, const std::shared_ptr<net>& rhs) { return lhs->get_id() < rhs->get_id(); });
                for (const auto& n : sorted)
                {
                    nets.PushBack(n->get_id(), allocator);
                }
                val.AddMember("nets", nets, allocator);
            }

            val.AddMember("data", serialize(m->get_data(), allocator), allocator);
            return val;
        }

        void serialize(std::shared_ptr<netlist> nl, rapidjson::Document& document)
        {
            rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

            document.AddMember("serialization_format_version", SERIALIZON_FORMAT_VERSION, allocator);

            document.AddMember("gate_library", nl->get_gate_library()->get_name(), allocator);
            document.AddMember("id", nl->get_id(), allocator);
            document.AddMember("input_file", nl->get_input_filename().string(), allocator);
            document.AddMember("design_name", nl->get_design_name(), allocator);
            document.AddMember("device_name", nl->get_device_name(), allocator);

            {
                rapidjson::Value gates(rapidjson::kArrayType);
                rapidjson::Value global_vccs(rapidjson::kArrayType);
                rapidjson::Value global_gnds(rapidjson::kArrayType);
                auto to_sort = nl->get_gates();
                std::vector<std::shared_ptr<gate>> sorted(to_sort.begin(), to_sort.end());
                std::sort(sorted.begin(), sorted.end(), [](const std::shared_ptr<gate>& lhs, const std::shared_ptr<gate>& rhs) { return lhs->get_id() < rhs->get_id(); });
                for (const auto& gate : sorted)
                {
                    gates.PushBack(serialize(gate, allocator), allocator);
                    if (nl->is_global_gnd_gate(gate))
                    {
                        global_gnds.PushBack(gate->get_id(), allocator);
                    }
                    if (nl->is_global_vcc_gate(gate))
                    {
                        global_vccs.PushBack(gate->get_id(), allocator);
                    }
                }
                document.AddMember("gates", gates, allocator);
                document.AddMember("global_vcc", global_vccs, allocator);
                document.AddMember("global_gnd", global_gnds, allocator);
            }
            {
                rapidjson::Value nets(rapidjson::kArrayType);
                rapidjson::Value global_in(rapidjson::kArrayType);
                rapidjson::Value global_out(rapidjson::kArrayType);
                rapidjson::Value global_inout(rapidjson::kArrayType);
                auto to_sort = nl->get_nets();
                std::vector<std::shared_ptr<net>> sorted(to_sort.begin(), to_sort.end());
                std::sort(sorted.begin(), sorted.end(), [](const std::shared_ptr<net>& lhs, const std::shared_ptr<net>& rhs) { return lhs->get_id() < rhs->get_id(); });
                for (const auto& net : sorted)
                {
                    nets.PushBack(serialize(net, allocator), allocator);
                    if (nl->is_global_input_net(net))
                    {
                        global_in.PushBack(net->get_id(), allocator);
                    }
                    if (nl->is_global_output_net(net))
                    {
                        global_out.PushBack(net->get_id(), allocator);
                    }
                    if (nl->is_global_inout_net(net))
                    {
                        global_inout.PushBack(net->get_id(), allocator);
                    }
                }
                document.AddMember("nets", nets, allocator);
                document.AddMember("global_in", global_in, allocator);
                document.AddMember("global_out", global_out, allocator);
                document.AddMember("global_inout", global_inout, allocator);
            }
            {
                rapidjson::Value modules(rapidjson::kArrayType);
                std::vector<std::shared_ptr<module>> sorted;
                for (const auto& module : nl->get_modules())
                {
                    sorted.push_back(module);
                }
                std::sort(sorted.begin(), sorted.end(), [](const std::shared_ptr<module>& lhs, const std::shared_ptr<module>& rhs) { return lhs->get_id() < rhs->get_id(); });
                for (const auto& module : sorted)
                {
                    modules.PushBack(serialize(module, allocator), allocator);
                }
                document.AddMember("modules", modules, allocator);
            }
        }
    }    // namespace

    // deserializing functions
    namespace
    {
        endpoint deserialize_endpoint(std::shared_ptr<netlist> nl, const rapidjson::Value& val)
        {
            return {nl->get_gate_by_id(val["gate_id"].GetUint()), val["pin_type"].GetString()};
        }

        void deserialize_data(std::shared_ptr<data_container> c, const rapidjson::Value& val)
        {
            for (const auto& entry : val.GetArray())
            {
                c->set_data(entry[0].GetString(), entry[1].GetString(), entry[2].GetString(), entry[3].GetString());
            }
        }

        bool deserialize_gate(std::shared_ptr<netlist> nl, const rapidjson::Value& val)
        {
            auto g = nl->create_gate(val["id"].GetUint(), val["type"].GetString(), val["name"].GetString());
            if (g == nullptr)
            {
                return false;
            }

            deserialize_data(g, val["data"]);
            return true;
        }

        bool deserialize_net(std::shared_ptr<netlist> nl, const rapidjson::Value& val)
        {
            auto n = nl->create_net(val["id"].GetUint(), val["name"].GetString());
            if (n == nullptr)
            {
                return false;
            }

            if (val.HasMember("src"))
            {
                n->set_src(deserialize_endpoint(nl, val["src"]));
            }

            for (const auto& dst_node : val["dsts"].GetArray())
            {
                n->add_dst(deserialize_endpoint(nl, dst_node));
            }

            deserialize_data(n, val["data"]);

            return true;
        }

        bool deserialize_module(std::shared_ptr<netlist> nl, const rapidjson::Value& val)
        {
            auto parent_id             = val["parent"].GetUint();
            std::shared_ptr<module> sm = nl->get_top_module();
            if (parent_id != 0)
            {
                sm = nl->create_module(val["id"].GetUint(), val["name"].GetString(), nl->get_module_by_id(parent_id));
                if (sm == nullptr)
                {
                    return false;
                }
            }

            for (const auto& gate_node : val["gates"].GetArray())
            {
                sm->assign_gate(nl->get_gate_by_id(gate_node.GetUint()));
            }

            for (const auto& net_node : val["nets"].GetArray())
            {
                sm->assign_net(nl->get_net_by_id(net_node.GetUint()));
            }

            deserialize_data(sm, val["data"]);
            return true;
        }

        std::shared_ptr<netlist> deserialize(const rapidjson::Document& document)
        {
            auto lib = gate_library_manager::get_gate_library(document["gate_library"].GetString());
            if (lib == nullptr)
            {
                log_critical("netlist.persistent", "error loading gate library '{}'.", document["gate_library"].GetString());
                return nullptr;
            }

            u32 encoded_version = document["serialization_format_version"].GetUint();
            if (encoded_version < SERIALIZON_FORMAT_VERSION)
            {
                log_warning("netlist.persistent", "the netlist was serialized with an older version of the serializer, deserialization may contain errors.");
            }
            else if (encoded_version > SERIALIZON_FORMAT_VERSION)
            {
                log_warning("netlist.persistent", "the netlist was serialized with a newer version of the serializer, deserialization may contain errors.");
            }

            std::shared_ptr<netlist> nl = std::make_shared<netlist>(lib);

            nl->set_id(document["id"].GetUint());
            nl->set_input_filename(document["input_file"].GetString());
            nl->set_design_name(document["design_name"].GetString());
            nl->set_device_name(document["device_name"].GetString());

            for (const auto& gate_node : document["gates"].GetArray())
            {
                if (!deserialize_gate(nl, gate_node))
                {
                    return nullptr;
                }
            }
            for (const auto& gate_node : document["global_vcc"].GetArray())
            {
                nl->mark_global_vcc_gate(nl->get_gate_by_id(gate_node.GetUint()));
            }
            for (const auto& gate_node : document["global_gnd"].GetArray())
            {
                nl->mark_global_gnd_gate(nl->get_gate_by_id(gate_node.GetUint()));
            }

            for (const auto& net_node : document["nets"].GetArray())
            {
                if (!deserialize_net(nl, net_node))
                {
                    return nullptr;
                }
            }
            for (const auto& net_node : document["global_in"].GetArray())
            {
                nl->mark_global_input_net(nl->get_net_by_id(net_node.GetUint()));
            }
            for (const auto& net_node : document["global_out"].GetArray())
            {
                nl->mark_global_output_net(nl->get_net_by_id(net_node.GetUint()));
            }
            for (const auto& net_node : document["global_inout"].GetArray())
            {
                nl->mark_global_inout_net(nl->get_net_by_id(net_node.GetUint()));
            }

            for (const auto& module_node : document["modules"].GetArray())
            {
                if (!deserialize_module(nl, module_node))
                {
                    return nullptr;
                }
            }

            return nl;
        }
    }    // namespace

    std::string serialize_to_string(std::shared_ptr<netlist> nl)
    {
        rapidjson::Document document;
        document.SetObject();

        serialize(nl, document);

        rapidjson::StringBuffer strbuf;
#if PRETTY_JSON_OUTPUT == 1
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(strbuf);
#else
        rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
#endif
        document.Accept(writer);

        return strbuf.GetString();
    }

    bool serialize_to_file(std::shared_ptr<netlist> nl, const hal::path& hal_file)
    {
        auto begin_time = std::chrono::high_resolution_clock::now();

        std::ofstream hal_file_stream;
        hal_file_stream.open(hal_file.string());
        if (hal_file_stream.fail())
        {
            log_error("hdl_writer", "Cannot open or create file {}. Please verify that the file and the containing directory is writable!", hal_file.string());
            return false;
        }

        hal_file_stream << serialize_to_string(nl);

        hal_file_stream.close();

        log_info("netlist.persistent", "serialized netlist in {:2.2f} seconds", DURATION(begin_time));

        return true;
    }

    std::shared_ptr<netlist> deserialize_from_string(const std::string& data)
    {
        netlist_event_handler::enable(false);
        gate_event_handler::enable(false);
        net_event_handler::enable(false);
        module_event_handler::enable(false);

        rapidjson::Document document;

        document.Parse(data);

        if (document.HasParseError())
        {
            log_error("netlist.persistent", "invalid json string for deserialization");
            return nullptr;
        }

        std::shared_ptr<netlist> netlist = deserialize(document);

        netlist_event_handler::enable(true);
        gate_event_handler::enable(true);
        net_event_handler::enable(true);
        module_event_handler::enable(true);
        return netlist;
    }

    std::shared_ptr<netlist> deserialize_from_file(const hal::path& hal_file)
    {
        auto begin_time = std::chrono::high_resolution_clock::now();

        netlist_event_handler::enable(false);
        gate_event_handler::enable(false);
        net_event_handler::enable(false);
        module_event_handler::enable(false);

        FILE* pFile = fopen(hal_file.string().c_str(), "rb");
        if (pFile == NULL)
        {
            log_error("netlist.persistent", "unable to open '{}'.", hal_file.string());
            return nullptr;
        }

        char buffer[65536];
        rapidjson::FileReadStream is(pFile, buffer, sizeof(buffer));
        rapidjson::Document document;
        document.ParseStream<0, rapidjson::UTF8<>, rapidjson::FileReadStream>(is);
        if (document.HasParseError())
        {
            log_error("netlist.persistent", "invalid json string for deserialization");
            return nullptr;
        }

        std::shared_ptr<netlist> netlist = deserialize(document);

        fclose(pFile);

        netlist_event_handler::enable(true);
        gate_event_handler::enable(true);
        net_event_handler::enable(true);
        module_event_handler::enable(true);

        log_info("netlist.persistent", "deserialized '{}' in {:2.2f} seconds", hal_file.string(), DURATION(begin_time));
        return netlist;
    }
}    // namespace netlist_serializer

#undef DURATION
