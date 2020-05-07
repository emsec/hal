#include "netlist/persistent/netlist_serializer.h"

#include "netlist/boolean_function.h"
#include "netlist/gate.h"
#include "netlist/module.h"
#include "netlist/net.h"
#include "netlist/netlist.h"

#include "netlist/event_system/event_controls.h"

#include "netlist/gate_library/gate_library_manager.h"

#include "core/hal_file_manager.h"
#include "core/log.h"

#include "rapidjson/filereadstream.h"
#include "rapidjson/stringbuffer.h"

#define PRETTY_JSON_OUTPUT false
#if PRETTY_JSON_OUTPUT
#include "rapidjson/prettywriter.h"
#else
#include "rapidjson/writer.h"
#endif

#include <chrono>
#include <fstream>
#include <sstream>

#ifndef DURATION
#define DURATION(begin_time) ((double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - begin_time).count() / 1000)
#endif

namespace netlist_serializer
{
    // serializing functions
    namespace
    {
        const int SERIALIZON_FORMAT_VERSION = 6;

#define JSON_STR_HELPER(x) rapidjson::Value{}.SetString(x.c_str(), x.length(), allocator)
        rapidjson::Value serialize(const std::map<std::tuple<std::string, std::string>, std::tuple<std::string, std::string>>& data, rapidjson::Document::AllocatorType& allocator)
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

        rapidjson::Value serialize(const endpoint& ep, rapidjson::Document::AllocatorType& allocator)
        {
            rapidjson::Value val(rapidjson::kObjectType);
            val.AddMember("gate_id", ep.get_gate()->get_id(), allocator);
            val.AddMember("pin_type", ep.get_pin(), allocator);
            val.AddMember("is_destination", ep.is_destination_pin(), allocator);
            return val;
        }

        rapidjson::Value serialize(const std::shared_ptr<gate>& g, rapidjson::Document::AllocatorType& allocator)
        {
            rapidjson::Value val(rapidjson::kObjectType);
            val.AddMember("id", g->get_id(), allocator);
            val.AddMember("name", g->get_name(), allocator);
            val.AddMember("type", g->get_type()->get_name(), allocator);
            auto data_val = serialize(g->get_data(), allocator);
            if (!data_val.Empty())
            {
                val.AddMember("data", data_val, allocator);
            }
            {
                rapidjson::Value functions(rapidjson::kObjectType);
                for (const auto& it : g->get_boolean_functions(true))
                {
                    auto s = it.second.to_string();
                    functions.AddMember(JSON_STR_HELPER(it.first), JSON_STR_HELPER(s), allocator);
                }
                if (functions.MemberCount() > 0)
                {
                    val.AddMember("custom_functions", functions, allocator);
                }
            }
            return val;
        }

        rapidjson::Value serialize(const std::shared_ptr<net>& n, rapidjson::Document::AllocatorType& allocator)
        {
            rapidjson::Value val(rapidjson::kObjectType);
            val.AddMember("id", n->get_id(), allocator);
            val.AddMember("name", n->get_name(), allocator);

            {
                rapidjson::Value srcs(rapidjson::kArrayType);
                auto sorted = n->get_sources();
                std::sort(sorted.begin(), sorted.end(), [](const endpoint& lhs, const endpoint& rhs) { return lhs.get_gate()->get_id() < rhs.get_gate()->get_id(); });
                for (const auto& src : sorted)
                {
                    srcs.PushBack(serialize(src, allocator), allocator);
                }
                if (!srcs.Empty())
                {
                    val.AddMember("srcs", srcs, allocator);
                }
            }

            {
                rapidjson::Value dsts(rapidjson::kArrayType);
                auto sorted = n->get_destinations();
                std::sort(sorted.begin(), sorted.end(), [](const endpoint& lhs, const endpoint& rhs) { return lhs.get_gate()->get_id() < rhs.get_gate()->get_id(); });
                for (const auto& dst : sorted)
                {
                    dsts.PushBack(serialize(dst, allocator), allocator);
                }
                if (!dsts.Empty())
                {
                    val.AddMember("dsts", dsts, allocator);
                }
            }

            auto data_val = serialize(n->get_data(), allocator);
            if (!data_val.Empty())
            {
                val.AddMember("data", data_val, allocator);
            }
            return val;
        }

        rapidjson::Value serialize(const std::shared_ptr<module>& m, rapidjson::Document::AllocatorType& allocator)
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
                auto to_sort = m->get_gates(nullptr, false);
                std::vector<std::shared_ptr<gate>> sorted(to_sort.begin(), to_sort.end());
                std::sort(sorted.begin(), sorted.end(), [](const std::shared_ptr<gate>& lhs, const std::shared_ptr<gate>& rhs) { return lhs->get_id() < rhs->get_id(); });
                for (const auto& g : sorted)
                {
                    gates.PushBack(g->get_id(), allocator);
                }
                if (!gates.Empty())
                {
                    val.AddMember("gates", gates, allocator);
                }
            }

            auto data_val = serialize(m->get_data(), allocator);
            if (!data_val.Empty())
            {
                val.AddMember("data", data_val, allocator);
            }
            return val;
        }

        void serialize(const std::shared_ptr<netlist>& nl, rapidjson::Document& document)
        {
            rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
            rapidjson::Value root(rapidjson::kObjectType);

            root.AddMember("gate_library", nl->get_gate_library()->get_name(), allocator);
            root.AddMember("id", nl->get_id(), allocator);
            root.AddMember("input_file", nl->get_input_filename().string(), allocator);
            root.AddMember("design_name", nl->get_design_name(), allocator);
            root.AddMember("device_name", nl->get_device_name(), allocator);

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
                    if (nl->is_gnd_gate(gate))
                    {
                        global_gnds.PushBack(gate->get_id(), allocator);
                    }
                    if (nl->is_vcc_gate(gate))
                    {
                        global_vccs.PushBack(gate->get_id(), allocator);
                    }
                }
                root.AddMember("gates", gates, allocator);
                root.AddMember("global_vcc", global_vccs, allocator);
                root.AddMember("global_gnd", global_gnds, allocator);
            }
            {
                rapidjson::Value nets(rapidjson::kArrayType);
                rapidjson::Value global_in(rapidjson::kArrayType);
                rapidjson::Value global_out(rapidjson::kArrayType);
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
                }
                root.AddMember("nets", nets, allocator);
                root.AddMember("global_in", global_in, allocator);
                root.AddMember("global_out", global_out, allocator);
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
                root.AddMember("modules", modules, allocator);
            }

            document.AddMember("netlist", root, document.GetAllocator());
        }
    }    // namespace

    // deserializing functions
    namespace
    {
#define assert_availablility(MEMBER)                                                               \
    if (!root.HasMember(MEMBER))                                                                   \
    {                                                                                              \
        log_critical("netlist.persistent", "'netlist' node does not include a '{}' node", MEMBER); \
        return nullptr;                                                                            \
    }

        endpoint deserialize_endpoint(std::shared_ptr<netlist> nl, const rapidjson::Value& val)
        {
            return endpoint(nl->get_gate_by_id(val["gate_id"].GetUint()), val["pin_type"].GetString(), val["is_destination"].GetBool());
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
            auto gt_name    = val["type"].GetString();
            auto gate_types = nl->get_gate_library()->get_gate_types();
            auto it         = gate_types.find(gt_name);
            if (it != gate_types.end())
            {
                auto g = nl->create_gate(val["id"].GetUint(), it->second, val["name"].GetString());

                if (g == nullptr)
                {
                    return false;
                }

                if (val.HasMember("data"))
                {
                    deserialize_data(g, val["data"]);
                }

                if (val.HasMember("custom_functions"))
                {
                    auto functions = val["custom_functions"].GetObject();
                    for (auto f_it = functions.MemberBegin(); f_it != functions.MemberEnd(); ++f_it)
                    {
                        g->add_boolean_function(f_it->name.GetString(), boolean_function::from_string(f_it->value.GetString()));
                    }
                }

                return true;
            }

            return false;
        }

        bool deserialize_net(std::shared_ptr<netlist> nl, const rapidjson::Value& val)
        {
            auto n = nl->create_net(val["id"].GetUint(), val["name"].GetString());
            if (n == nullptr)
            {
                return false;
            }

            if (val.HasMember("srcs"))
            {
                for (const auto& src_node : val["srcs"].GetArray())
                {
                    n->add_source(deserialize_endpoint(nl, src_node));
                }
            }

            if (val.HasMember("dsts"))
            {
                for (const auto& dst_node : val["dsts"].GetArray())
                {
                    n->add_destination(deserialize_endpoint(nl, dst_node));
                }
            }

            if (val.HasMember("data"))
            {
                deserialize_data(n, val["data"]);
            }

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

            if (val.HasMember("gates"))
            {
                for (const auto& gate_node : val["gates"].GetArray())
                {
                    sm->assign_gate(nl->get_gate_by_id(gate_node.GetUint()));
                }
            }

            if (val.HasMember("data"))
            {
                deserialize_data(sm, val["data"]);
            }
            return true;
        }

        std::shared_ptr<netlist> deserialize(const rapidjson::Document& document)
        {
            if (!document.HasMember("netlist"))
            {
                log_critical("netlist.persistent", "file does not include a 'netlist' node");
                return nullptr;
            }
            auto root = document["netlist"].GetObject();
            assert_availablility("gate_library");

            auto lib = gate_library_manager::get_gate_library(root["gate_library"].GetString());
            if (lib == nullptr)
            {
                log_critical("netlist.persistent", "error loading gate library '{}'.", root["gate_library"].GetString());
                return nullptr;
            }

            std::shared_ptr<netlist> nl = std::make_shared<netlist>(lib);

            assert_availablility("id");
            nl->set_id(root["id"].GetUint());

            assert_availablility("input_file");
            nl->set_input_filename(root["input_file"].GetString());

            assert_availablility("design_name");
            nl->set_design_name(root["design_name"].GetString());

            assert_availablility("device_name");
            nl->set_device_name(root["device_name"].GetString());

            assert_availablility("gates");
            for (const auto& gate_node : root["gates"].GetArray())
            {
                if (!deserialize_gate(nl, gate_node))
                {
                    return nullptr;
                }
            }

            assert_availablility("global_vcc");
            for (const auto& gate_node : root["global_vcc"].GetArray())
            {
                nl->mark_vcc_gate(nl->get_gate_by_id(gate_node.GetUint()));
            }

            assert_availablility("global_gnd");
            for (const auto& gate_node : root["global_gnd"].GetArray())
            {
                nl->mark_gnd_gate(nl->get_gate_by_id(gate_node.GetUint()));
            }

            assert_availablility("nets");
            for (const auto& net_node : root["nets"].GetArray())
            {
                if (!deserialize_net(nl, net_node))
                {
                    return nullptr;
                }
            }

            assert_availablility("global_in");
            for (const auto& net_node : root["global_in"].GetArray())
            {
                nl->mark_global_input_net(nl->get_net_by_id(net_node.GetUint()));
            }

            assert_availablility("global_out");
            for (const auto& net_node : root["global_out"].GetArray())
            {
                nl->mark_global_output_net(nl->get_net_by_id(net_node.GetUint()));
            }

            assert_availablility("modules");
            for (const auto& module_node : root["modules"].GetArray())
            {
                if (!deserialize_module(nl, module_node))
                {
                    return nullptr;
                }
            }

            return nl;
        }
    }    // namespace

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

        rapidjson::Document document;
        document.SetObject();

        document.AddMember("serialization_format_version", SERIALIZON_FORMAT_VERSION, document.GetAllocator());

        serialize(nl, document);

        if (!hal_file_manager::serialize(hal_file, nl, document))
        {
            log_info("netlist.persistent", "serialization failed");
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

        log_info("netlist.persistent", "serialized netlist in {:2.2f} seconds", DURATION(begin_time));

        return true;
    }

    std::shared_ptr<netlist> deserialize_from_file(const hal::path& hal_file)
    {
        auto begin_time = std::chrono::high_resolution_clock::now();

        // event_controls::enable_all(false);

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
        fclose(pFile);

        if (document.HasParseError())
        {
            log_error("netlist.persistent", "invalid json string for deserialization");
            return nullptr;
        }

        if (document.HasMember("serialization_format_version"))
        {
            u32 encoded_version = document["serialization_format_version"].GetUint();
            if (encoded_version < SERIALIZON_FORMAT_VERSION)
            {
                log_warning("netlist.persistent", "the netlist was serialized with an older version of the serializer, deserialization may contain errors.");
            }
            else if (encoded_version > SERIALIZON_FORMAT_VERSION)
            {
                log_warning("netlist.persistent", "the netlist was serialized with a newer version of the serializer, deserialization may contain errors.");
            }
        }
        else
        {
            log_warning("netlist.persistent", "the netlist was serialized with an older version of the serializer, deserialization may contain errors.");
        }

        std::shared_ptr<netlist> netlist = deserialize(document);

        if (!hal_file_manager::deserialize(hal_file, netlist, document))
        {
            log_info("netlist.persistent", "deserialization failed");
            return nullptr;
        }

        // event_controls::enable_all(true);

        log_info("netlist.persistent", "deserialized '{}' in {:2.2f} seconds", hal_file.string(), DURATION(begin_time));
        return netlist;
    }
}    // namespace netlist_serializer

#undef DURATION
