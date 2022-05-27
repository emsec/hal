#include "hal_core/netlist/persistent/netlist_serializer.h"

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"
#include "hal_core/netlist/project_manager.h"
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
#include <queue>
#include <sstream>

#ifndef DURATION
#define DURATION(begin_time) ((double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - begin_time).count() / 1000)
#endif

namespace hal
{
    namespace netlist_serializer
    {
        // serializing functions
        namespace
        {
            const int SERIALIZATION_FORMAT_VERSION = 10;

#define JSON_STR_HELPER(x) rapidjson::Value{}.SetString(x.c_str(), x.length(), allocator)

#define assert_availablility(MEMBER)                                                               \
    if (!root.HasMember(MEMBER))                                                                   \
    {                                                                                              \
        log_critical("netlist_persistent", "'netlist' node does not include a '{}' node", MEMBER); \
        return nullptr;                                                                            \
    }

            namespace
            {
                struct PinGroupInformation
                {
                    struct PinInformation
                    {
                        i32 id = -1;
                        Net* net;
                        std::string name;
                        PinType type = PinType::none;
                    };

                    i32 id = -1;
                    std::string name;
                    PinDirection direction = PinDirection::none;
                    PinType type           = PinType::none;
                    std::vector<PinInformation> pins;
                    bool ascending  = false;
                    u32 start_index = 0;
                };

            }    // namespace

            // serialize container data
            rapidjson::Value serialize(const std::map<std::tuple<std::string, std::string>, std::tuple<std::string, std::string>>& data, rapidjson::Document::AllocatorType& allocator)
            {
                rapidjson::Value val(rapidjson::kArrayType);
                for (const std::pair<std::tuple<std::string, std::string>, std::tuple<std::string, std::string>>& it : data)
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

            void deserialize_data(DataContainer* c, const rapidjson::Value& val)
            {
                for (const auto& entry : val.GetArray())
                {
                    c->set_data(entry[0].GetString(), entry[1].GetString(), entry[2].GetString(), entry[3].GetString());
                }
            }

            // serialize endpoint
            rapidjson::Value serialize(const Endpoint* ep, rapidjson::Document::AllocatorType& allocator)
            {
                rapidjson::Value val(rapidjson::kObjectType);
                val.AddMember("gate_id", ep->get_gate()->get_id(), allocator);
                val.AddMember("pin_type", ep->get_pin(), allocator);
                return val;
            }

            bool deserialize_destination(Netlist* nl, Net* net, const rapidjson::Value& val)
            {
                return net->add_destination(nl->get_gate_by_id(val["gate_id"].GetUint()), val["pin_type"].GetString());
            }

            bool deserialize_source(Netlist* nl, Net* net, const rapidjson::Value& val)
            {
                return net->add_source(nl->get_gate_by_id(val["gate_id"].GetUint()), val["pin_type"].GetString());
            }

            // serialize gate
            rapidjson::Value serialize(const Gate* g, rapidjson::Document::AllocatorType& allocator)
            {
                rapidjson::Value val(rapidjson::kObjectType);
                val.AddMember("id", g->get_id(), allocator);
                val.AddMember("name", g->get_name(), allocator);
                val.AddMember("type", g->get_type()->get_name(), allocator);
                auto data_val = serialize(g->get_data_map(), allocator);
                if (!data_val.Empty())
                {
                    val.AddMember("data", data_val, allocator);
                }
                {
                    rapidjson::Value functions(rapidjson::kObjectType);
                    for (const auto& [name, function] : g->get_boolean_functions(true))
                    {
                        const std::string function_str = function.to_string();
                        functions.AddMember(JSON_STR_HELPER(name), JSON_STR_HELPER(function_str), allocator);
                    }
                    if (functions.MemberCount() > 0)
                    {
                        val.AddMember("custom_functions", functions, allocator);
                    }
                }
                return val;
            }

            bool deserialize_gate(Netlist* nl, const rapidjson::Value& val, const std::unordered_map<std::string, hal::GateType*>& gate_types)
            {
                auto gt_name = val["type"].GetString();
                auto it      = gate_types.find(gt_name);
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
                            auto function = BooleanFunction::from_string(f_it->value.GetString());
                            g->add_boolean_function(f_it->name.GetString(), (function.is_ok()) ? function.get() : BooleanFunction());
                        }
                    }

                    return true;
                }

                return false;
            }

            // serialize net
            rapidjson::Value serialize(const Net* n, rapidjson::Document::AllocatorType& allocator)
            {
                rapidjson::Value val(rapidjson::kObjectType);
                val.AddMember("id", n->get_id(), allocator);
                val.AddMember("name", n->get_name(), allocator);

                {
                    rapidjson::Value srcs(rapidjson::kArrayType);
                    std::vector<Endpoint*> sorted = n->get_sources();
                    std::sort(sorted.begin(), sorted.end(), [](Endpoint* lhs, Endpoint* rhs) { return lhs->get_gate()->get_id() < rhs->get_gate()->get_id(); });
                    for (const Endpoint* src : sorted)
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
                    std::vector<Endpoint*> sorted = n->get_destinations();
                    std::sort(sorted.begin(), sorted.end(), [](Endpoint* lhs, Endpoint* rhs) { return lhs->get_gate()->get_id() < rhs->get_gate()->get_id(); });
                    for (const Endpoint* dst : sorted)
                    {
                        dsts.PushBack(serialize(dst, allocator), allocator);
                    }
                    if (!dsts.Empty())
                    {
                        val.AddMember("dsts", dsts, allocator);
                    }
                }

                auto data_val = serialize(n->get_data_map(), allocator);
                if (!data_val.Empty())
                {
                    val.AddMember("data", data_val, allocator);
                }
                return val;
            }

            bool deserialize_net(Netlist* nl, const rapidjson::Value& val)
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
                        if (!deserialize_source(nl, n, src_node))
                        {
                            return false;
                        }
                    }
                }

                if (val.HasMember("dsts"))
                {
                    for (const auto& dst_node : val["dsts"].GetArray())
                    {
                        if (!deserialize_destination(nl, n, dst_node))
                        {
                            return false;
                        }
                    }
                }

                if (val.HasMember("data"))
                {
                    deserialize_data(n, val["data"]);
                }

                return true;
            }

            // serialize module
            rapidjson::Value serialize(const Module* m, rapidjson::Document::AllocatorType& allocator)
            {
                rapidjson::Value val(rapidjson::kObjectType);
                val.AddMember("id", m->get_id(), allocator);
                val.AddMember("type", m->get_type(), allocator);
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
                    std::vector<Gate*> sorted = m->get_gates(nullptr, false);
                    std::sort(sorted.begin(), sorted.end(), [](Gate* lhs, Gate* rhs) { return lhs->get_id() < rhs->get_id(); });
                    for (const Gate* g : sorted)
                    {
                        gates.PushBack(g->get_id(), allocator);
                    }
                    if (!gates.Empty())
                    {
                        val.AddMember("gates", gates, allocator);
                    }
                }
                {
                    rapidjson::Value json_pin_groups(rapidjson::kArrayType);
                    for (const PinGroup<ModulePin>* pin_group : m->get_pin_groups())
                    {
                        rapidjson::Value json_pin_group(rapidjson::kObjectType);
                        json_pin_group.AddMember("id", pin_group->get_id(), allocator);
                        json_pin_group.AddMember("name", pin_group->get_name(), allocator);
                        json_pin_group.AddMember("direction", enum_to_string(pin_group->get_direction()), allocator);
                        json_pin_group.AddMember("type", enum_to_string(pin_group->get_type()), allocator);
                        json_pin_group.AddMember("ascending", pin_group->is_ascending(), allocator);
                        json_pin_group.AddMember("start_index", pin_group->get_start_index(), allocator);
                        rapidjson::Value json_pins(rapidjson::kArrayType);
                        for (const ModulePin* pin : pin_group->get_pins())
                        {
                            rapidjson::Value json_pin(rapidjson::kObjectType);
                            json_pin.AddMember("id", pin->get_id(), allocator);
                            json_pin.AddMember("name", pin->get_name(), allocator);
                            json_pin.AddMember("type", enum_to_string(pin->get_type()), allocator);
                            json_pin.AddMember("net_id", pin->get_net()->get_id(), allocator);
                            json_pins.PushBack(json_pin, allocator);
                        }
                        json_pin_group.AddMember("pins", json_pins, allocator);
                        json_pin_groups.PushBack(json_pin_group, allocator);
                    }
                    if (!json_pin_groups.Empty())
                    {
                        val.AddMember("pin_groups", json_pin_groups, allocator);
                    }
                }

                auto data_val = serialize(m->get_data_map(), allocator);
                if (!data_val.Empty())
                {
                    val.AddMember("data", data_val, allocator);
                }
                return val;
            }

            bool deserialize_module(Netlist* nl, const rapidjson::Value& val, std::unordered_map<Module*, std::vector<PinGroupInformation>>& pin_group_cache)
            {
                u32 parent_id = val["parent"].GetUint();
                Module* sm    = nl->get_top_module();
                if (parent_id == 0)
                {
                    // top_module must not be created but might be renamed
                    std::string top_module_name = val["name"].GetString();
                    if (top_module_name != sm->get_name())
                        sm->set_name(top_module_name);
                }
                else
                {
                    sm = nl->create_module(val["id"].GetUint(), val["name"].GetString(), nl->get_module_by_id(parent_id));
                    if (sm == nullptr)
                    {
                        return false;
                    }
                }

                if (val.HasMember("type"))
                {
                    sm->set_type(val["type"].GetString());
                }

                if (val.HasMember("gates"))
                {
                    std::vector<Gate*> gates;
                    for (auto& gate_node : val["gates"].GetArray())
                    {
                        if (!sm->is_top_module())
                        {
                            gates.push_back(nl->get_gate_by_id(gate_node.GetUint()));
                        }
                    }
                    sm->assign_gates(gates);
                }

                if (val.HasMember("data"))
                {
                    deserialize_data(sm, val["data"]);
                }

                // u32 ctr = 0;
                // for (ModulePin* pin : sm->get_pins())
                // {
                //     // pin names must be unique, hence automatically generated groups may conflict with deserialized ones
                //     sm->set_pin_name(pin, "____deserialize_tmp_pin_[" + std::to_string(ctr) + "]____");
                // }

                // ctr = 0;
                // for (PinGroup<ModulePin>* pin_group : sm->get_pin_groups())
                // {
                //     // pin group names must be unique, hence automatically generated groups may conflict with deserialized ones
                //     sm->set_pin_group_name(pin_group, "____deserialize_tmp_pin_group_[" + std::to_string(ctr) + "]____");
                // }

                if (val.HasMember("pin_groups"))
                {
                    // pins need to be cached until all modules have been instantiated
                    for (const auto& json_pin_group : val["pin_groups"].GetArray())
                    {
                        PinGroupInformation pin_group;
                        pin_group.id          = json_pin_group["id"].GetUint();
                        pin_group.name        = json_pin_group["name"].GetString();
                        pin_group.direction   = enum_from_string<PinDirection>(json_pin_group["direction"].GetString());
                        pin_group.type        = enum_from_string<PinType>(json_pin_group["type"].GetString());
                        pin_group.ascending   = json_pin_group["ascending"].GetBool();
                        pin_group.start_index = json_pin_group["start_index"].GetUint();

                        for (const auto& pin_node : json_pin_group["pins"].GetArray())
                        {
                            PinGroupInformation::PinInformation pin;
                            pin.id   = pin_node["id"].GetUint();
                            pin.name = pin_node["name"].GetString();
                            pin.net  = nl->get_net_by_id(pin_node["net_id"].GetUint());
                            pin.type = enum_from_string<PinType>(pin_node["type"].GetString());
                            pin_group.pins.push_back(pin);
                        }
                        pin_group_cache[sm].push_back(pin_group);
                    }
                }

                // legacy code below
                if (val.HasMember("input_ports"))
                {
                    for (const auto& json_pin_legacy : val["input_ports"].GetArray())
                    {
                        PinGroupInformation pin_group;
                        pin_group.name = json_pin_legacy["port_name"].GetString();
                        PinGroupInformation::PinInformation pin;
                        pin.name = json_pin_legacy["port_name"].GetString();
                        pin.net  = nl->get_net_by_id(json_pin_legacy["net_id"].GetUint());
                        pin_group.pins.push_back(pin);
                        pin_group_cache[sm].push_back(pin_group);
                    }
                }

                if (val.HasMember("output_ports"))
                {
                    for (const auto& json_pin_legacy : val["output_ports"].GetArray())
                    {
                        PinGroupInformation pin_group;
                        pin_group.name = json_pin_legacy["port_name"].GetString();
                        PinGroupInformation::PinInformation pin;
                        pin.name = json_pin_legacy["port_name"].GetString();
                        pin.net  = nl->get_net_by_id(json_pin_legacy["net_id"].GetUint());
                        pin_group.pins.push_back(pin);
                        pin_group_cache[sm].push_back(pin_group);
                    }
                }
                // legacy code above

                return true;
            }

            bool deserialize_module_pins(const std::unordered_map<Module*, std::vector<PinGroupInformation>>& pin_group_cache)
            {
                for (const auto& [sm, pin_groups] : pin_group_cache)
                {
                    for (const PinGroupInformation& pg : pin_groups)
                    {
                        std::vector<ModulePin*> pins;
                        for (const PinGroupInformation::PinInformation& p : pg.pins)
                        {
                            u32 pid = (p.id > 0) ? (u32)p.id : sm->get_unique_pin_id();
                            if (auto res = sm->create_pin(pid, p.name, p.net, p.type, false); res.is_error())
                            {
                                log_error("netlist_persistent", "{}", res.get_error().get());
                                return false;
                            }
                            else
                            {
                                pins.push_back(res.get());
                            }
                        }
                        u32 pgid = (pg.id > 0) ? (u32)pg.id : sm->get_unique_pin_group_id();
                        if (auto res = sm->create_pin_group(pgid, pg.name, pins, pg.direction, pg.type, pg.ascending, pg.start_index); res.is_error())
                        {
                            log_error("netlist_persistent", "{}", res.get_error().get());
                            return false;
                        }
                    }
                }
                return true;
            }

             // serialize netlist
            void serialize(const Netlist* nl, rapidjson::Document& document)
            {
                rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
                rapidjson::Value root(rapidjson::kObjectType);

                root.AddMember("gate_library", nl->get_gate_library()->get_path().string(), allocator);
                root.AddMember("id", nl->get_id(), allocator);
                root.AddMember("input_file", nl->get_input_filename().string(), allocator);
                root.AddMember("design_name", nl->get_design_name(), allocator);
                root.AddMember("device_name", nl->get_device_name(), allocator);

                {
                    rapidjson::Value gates(rapidjson::kArrayType);
                    rapidjson::Value global_vccs(rapidjson::kArrayType);
                    rapidjson::Value global_gnds(rapidjson::kArrayType);
                    std::vector<Gate*> sorted = nl->get_gates();
                    std::sort(sorted.begin(), sorted.end(), [](Gate* lhs, Gate* rhs) { return lhs->get_id() < rhs->get_id(); });
                    for (const Gate* gate : sorted)
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
                    std::vector<Net*> sorted = nl->get_nets();
                    std::sort(sorted.begin(), sorted.end(), [](Net* lhs, Net* rhs) { return lhs->get_id() < rhs->get_id(); });
                    for (const Net* net : sorted)
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

                    // module ids are not sorted to preserve hierarchy
                    std::queue<const Module*> q;
                    q.push(nl->get_top_module());
                    while (!q.empty())
                    {
                        const Module* m = q.front();
                        q.pop();

                        modules.PushBack(serialize(m, allocator), allocator);

                        for (const Module* sm : m->get_submodules())
                        {
                            q.push(sm);
                        }
                    }
                    root.AddMember("modules", modules, allocator);
                }

                document.AddMember("netlist", root, document.GetAllocator());
            }

            std::unique_ptr<Netlist> deserialize(const rapidjson::Document& document)
            {
                if (!document.HasMember("netlist"))
                {
                    log_critical("netlist_persistent", "file does not include a 'netlist' node");
                    return nullptr;
                }
                auto root = document["netlist"].GetObject();
                assert_availablility("gate_library");

                std::filesystem::path glib_path(root["gate_library"].GetString());

                GateLibrary* glib = gate_library_manager::get_gate_library(glib_path.string());

                if (glib == nullptr)
                {
                    if (glib_path.extension() == ".hgl")
                        glib_path.replace_extension(".lib");
                    else
                        glib_path.replace_extension(".hgl");
                    glib = gate_library_manager::get_gate_library(glib_path.string());

                    if (glib == nullptr)
                    {
                        log_critical("netlist_persistent", "error loading gate library '{}'.", root["gate_library"].GetString());
                        return nullptr;
                    }
                    else
                    {
                        log_info("netlist_persistent", "gate library '{}' demanded but using '{}' instead.", root["gate_library"].GetString(), glib_path.string());
                    }
                }

                auto nl = std::make_unique<Netlist>(glib);

                // disable automatically checking module nets
                nl->enable_automatic_net_checks(false);

                assert_availablility("id");
                nl->set_id(root["id"].GetUint());

                assert_availablility("input_file");
                nl->set_input_filename(root["input_file"].GetString());

                assert_availablility("design_name");
                nl->set_design_name(root["design_name"].GetString());

                assert_availablility("device_name");
                nl->set_device_name(root["device_name"].GetString());

                assert_availablility("gates");
                auto gate_types = nl->get_gate_library()->get_gate_types();
                for (auto& gate_node : root["gates"].GetArray())
                {
                    if (!deserialize_gate(nl.get(), gate_node, gate_types))
                    {
                        return nullptr;
                    }
                }

                assert_availablility("global_vcc");
                for (auto& gate_node : root["global_vcc"].GetArray())
                {
                    nl->mark_vcc_gate(nl->get_gate_by_id(gate_node.GetUint()));
                }

                assert_availablility("global_gnd");
                for (auto& gate_node : root["global_gnd"].GetArray())
                {
                    nl->mark_gnd_gate(nl->get_gate_by_id(gate_node.GetUint()));
                }

                assert_availablility("nets");
                for (auto& net_node : root["nets"].GetArray())
                {
                    if (!deserialize_net(nl.get(), net_node))
                    {
                        return nullptr;
                    }
                }

                assert_availablility("global_in");
                for (auto& net_node : root["global_in"].GetArray())
                {
                    nl->mark_global_input_net(nl->get_net_by_id(net_node.GetUint()));
                }

                assert_availablility("global_out");
                for (auto& net_node : root["global_out"].GetArray())
                {
                    nl->mark_global_output_net(nl->get_net_by_id(net_node.GetUint()));
                }

                assert_availablility("modules");
                std::unordered_map<Module*, std::vector<PinGroupInformation>> pin_group_cache;
                for (auto& module_node : root["modules"].GetArray())
                {
                    if (!deserialize_module(nl.get(), module_node, pin_group_cache))
                    {
                        return nullptr;
                    }
                }

                // update module nets, internal nets, input nets, and output nets
                for (Module* mod : nl->get_modules())
                {
                    mod->update_nets();
                }

                // load module pins (nets must have been updated beforehand)
                if (!deserialize_module_pins(pin_group_cache))
                {
                    return nullptr;
                }

                // re-enable automatically checking module nets
                nl->enable_automatic_net_checks(true);

                return nl;
            }
        }    // namespace

        bool serialize_to_file(const Netlist* nl, const std::filesystem::path& hal_file)
        {
            if (nl == nullptr)
            {
                return false;
            }

            auto begin_time = std::chrono::high_resolution_clock::now();

            // create directory if it got erased in the meantime
            std::filesystem::path serialize_to_dir = hal_file.parent_path();
            if (!std::filesystem::exists(serialize_to_dir))
                std::filesystem::create_directory(serialize_to_dir);

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

        std::unique_ptr<Netlist> deserialize_from_file(const std::filesystem::path& hal_file)
        {
            auto begin_time = std::chrono::high_resolution_clock::now();

            // event_controls::enable_all(false);

            FILE* pFile = fopen(hal_file.string().c_str(), "rb");
            if (pFile == NULL)
            {
                log_error("netlist_persistent", "unable to open '{}'.", hal_file.string());
                return nullptr;
            }

            char buffer[65536];
            rapidjson::FileReadStream is(pFile, buffer, sizeof(buffer));
            rapidjson::Document document;
            document.ParseStream<0, rapidjson::UTF8<>, rapidjson::FileReadStream>(is);
            fclose(pFile);

            if (document.HasParseError())
            {
                log_error("netlist_persistent", "invalid json string for deserialization");
                return nullptr;
            }

            if (document.HasMember("serialization_format_version"))
            {
                u32 encoded_version = document["serialization_format_version"].GetUint();
                if (encoded_version < SERIALIZATION_FORMAT_VERSION)
                {
                    log_warning("netlist_persistent", "the netlist was serialized with an older version of the serializer, deserialization may contain errors.");
                }
                else if (encoded_version > SERIALIZATION_FORMAT_VERSION)
                {
                    log_warning("netlist_persistent", "the netlist was serialized with a newer version of the serializer, deserialization may contain errors.");
                }
            }
            else
            {
                log_warning("netlist_persistent", "the netlist was serialized with an older version of the serializer, deserialization may contain errors.");
            }

            auto netlist = deserialize(document);

            if (netlist)
            {
                log_info("netlist_persistent", "deserialized '{}' in {:2.2f} seconds", hal_file.string(), DURATION(begin_time));
            }

            // event_controls::enable_all(true);
            return netlist;
        }
    }    // namespace netlist_serializer
}    // namespace hal

#undef DURATION
