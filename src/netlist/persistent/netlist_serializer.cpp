#include "hal_core/netlist/persistent/netlist_serializer.h"

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/project_manager.h"
#include "hal_core/utilities/log.h"
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
            const int SERIALIZATION_FORMAT_VERSION = 12;

            // Ver 12 : location of gates
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
                val.AddMember("pin_id", ep->get_pin()->get_id(), allocator);
                return val;
            }

            bool deserialize_destination(Netlist* nl, Net* net, const rapidjson::Value& val)
            {
                Gate* gate = nl->get_gate_by_id(val["gate_id"].GetUint());
                GatePin* pin;
                if (val.HasMember("pin_id"))
                {
                    const u32 pin_id = val["pin_id"].GetUint();
                    pin              = gate->get_type()->get_pin_by_id(pin_id);
                    if (pin == nullptr)
                    {
                        log_error("netlist_persistent",
                                  "could not deserialize destination of net '" + net->get_name() + "' with ID " + std::to_string(net->get_id()) + ": failed to get pin with ID "
                                      + std::to_string(pin_id));
                        return false;
                    }
                }
                else
                {
                    // legacy code for backward compatibility
                    const std::string pin_name = val["pin_type"].GetString();
                    pin                        = gate->get_type()->get_pin_by_name(pin_name);
                    if (pin == nullptr)
                    {
                        log_error("netlist_persistent",
                                  "could not deserialize destination of net '" + net->get_name() + "' with ID " + std::to_string(net->get_id()) + ": failed to get pin with name '" + pin_name + "'");
                        return false;
                    }
                }
                if (!net->add_destination(gate, pin))
                {
                    log_error("netlist_persistent",
                              "could not deserialize destination of net '" + net->get_name() + "' with ID " + std::to_string(net->get_id()) + ": failed to add pin '" + pin->get_name()
                                  + "' as destination to net '" + net->get_name() + "' with ID " + std::to_string(net->get_id()));
                    return false;
                }
                return true;
            }

            bool deserialize_source(Netlist* nl, Net* net, const rapidjson::Value& val)
            {
                Gate* gate = nl->get_gate_by_id(val["gate_id"].GetUint());
                GatePin* pin;
                if (val.HasMember("pin_id"))
                {
                    const u32 pin_id = val["pin_id"].GetUint();
                    pin              = gate->get_type()->get_pin_by_id(pin_id);
                    if (pin == nullptr)
                    {
                        log_error("netlist_persistent",
                                  "could not deserialize source of net '" + net->get_name() + "' with ID " + std::to_string(net->get_id()) + ": failed to get pin with ID " + std::to_string(pin_id));
                        return false;
                    }
                }
                else
                {
                    // legacy code for backward compatibility
                    const std::string pin_name = val["pin_type"].GetString();
                    pin                        = gate->get_type()->get_pin_by_name(pin_name);
                    if (pin == nullptr)
                    {
                        log_error("netlist_persistent",
                                  "could not deserialize source of net '" + net->get_name() + "' with ID " + std::to_string(net->get_id()) + ": failed to get pin with name '" + pin_name + "'");
                        return false;
                    }
                }
                if (net->add_source(gate, pin) == nullptr)
                {
                    log_error("netlist_persistent",
                              "could not deserialize source of net '" + net->get_name() + "' with ID " + std::to_string(net->get_id()) + ": failed to add pin '" + pin->get_name()
                                  + "' as source to net '" + net->get_name() + "' with ID " + std::to_string(net->get_id()));
                    return false;
                }
                return true;
            }

            // serialize gate
            rapidjson::Value serialize(const Gate* gate, rapidjson::Document::AllocatorType& allocator)
            {
                rapidjson::Value val(rapidjson::kObjectType);
                val.AddMember("id", gate->get_id(), allocator);
                val.AddMember("name", gate->get_name(), allocator);
                val.AddMember("type", gate->get_type()->get_name(), allocator);
                if (gate->has_location())
                {
                    val.AddMember("location_x", gate->get_location_x(), allocator);
                    val.AddMember("location_y", gate->get_location_y(), allocator);
                }
                auto data = serialize(gate->get_data_map(), allocator);
                if (!data.Empty())
                {
                    val.AddMember("data", data, allocator);
                }
                {
                    rapidjson::Value functions(rapidjson::kObjectType);
                    for (const auto& [name, function] : gate->get_boolean_functions(true))
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
                const u32 gate_id           = val["id"].GetUint();
                const std::string gate_name = val["name"].GetString();
                const std::string gate_type = val["type"].GetString();
                i32 lx = -1;
                i32 ly = -1;
                if (val.HasMember("location_x") && val.HasMember("location_y"))
                {
                    lx = val["location_x"].GetInt();
                    ly = val["location_y"].GetInt();
                }
                if (auto it = gate_types.find(gate_type); it != gate_types.end())
                {
                    auto gate = nl->create_gate(gate_id, it->second, gate_name, lx, ly);
                    if (gate == nullptr)
                    {
                        log_error("netlist_persistent", "could not deserialize gate '" + gate_name + "' with ID " + std::to_string(gate_id) + ": failed to create gate");
                        return false;
                    }

                    if (val.HasMember("data"))
                    {
                        deserialize_data(gate, val["data"]);
                    }

                    if (val.HasMember("custom_functions"))
                    {
                        auto functions = val["custom_functions"].GetObject();
                        for (auto f_it = functions.MemberBegin(); f_it != functions.MemberEnd(); ++f_it)
                        {
                            auto func = BooleanFunction::from_string(f_it->value.GetString());
                            if (func.is_error())
                            {
                                log_error("netlist_persistent",
                                          "could not deserialize gate '" + gate_name + "' with ID " + std::to_string(gate_id) + ": failed to parse Boolean function from string\n{}",
                                          func.get_error().get());
                                return false;
                            }
                            gate->add_boolean_function(f_it->name.GetString(), func.get());
                        }
                    }

                    return true;
                }

                log_error("netlist_persistent",
                          "could not deserialize gate '" + gate_name + "' with ID " + std::to_string(gate_id) + ": failed to find gate '" + gate_type + "' in gate library '"
                              + nl->get_gate_library()->get_name() + "'");
                return false;
            }

            // serialize net
            rapidjson::Value serialize(const Net* net, rapidjson::Document::AllocatorType& allocator)
            {
                rapidjson::Value val(rapidjson::kObjectType);
                val.AddMember("id", net->get_id(), allocator);
                val.AddMember("name", net->get_name(), allocator);

                {
                    rapidjson::Value srcs(rapidjson::kArrayType);
                    std::vector<Endpoint*> sorted = net->get_sources();
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
                    std::vector<Endpoint*> sorted = net->get_destinations();
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

                auto data = serialize(net->get_data_map(), allocator);
                if (!data.Empty())
                {
                    val.AddMember("data", data, allocator);
                }
                return val;
            }

            bool deserialize_net(Netlist* nl, const rapidjson::Value& val)
            {
                const u32 net_id           = val["id"].GetUint();
                const std::string net_name = val["name"].GetString();
                auto net                   = nl->create_net(net_id, net_name);
                if (net == nullptr)
                {
                    log_error("netlist_persistent", "could not deserialize net '" + net_name + "' with ID " + std::to_string(net_id) + ": failed to create net");
                    return false;
                }

                if (val.HasMember("srcs"))
                {
                    for (const auto& src_node : val["srcs"].GetArray())
                    {
                        if (!deserialize_source(nl, net, src_node))
                        {
                            log_error("netlist_persistent", "could not deserialize net '" + net_name + "' with ID " + std::to_string(net_id) + ": failed to deserialize source");
                            return false;
                        }
                    }
                }

                if (val.HasMember("dsts"))
                {
                    for (const auto& dst_node : val["dsts"].GetArray())
                    {
                        if (!deserialize_destination(nl, net, dst_node))
                        {
                            log_error("netlist_persistent", "could not deserialize net '" + net_name + "' with ID " + std::to_string(net_id) + ": failed to deserialize destination");
                            return false;
                        }
                    }
                }

                if (val.HasMember("data"))
                {
                    deserialize_data(net, val["data"]);
                }

                return true;
            }

            // serialize module
            rapidjson::Value serialize(const Module* module, rapidjson::Document::AllocatorType& allocator)
            {
                rapidjson::Value val(rapidjson::kObjectType);
                val.AddMember("id", module->get_id(), allocator);
                val.AddMember("type", module->get_type(), allocator);
                val.AddMember("name", module->get_name(), allocator);
                Module* parent = module->get_parent_module();
                if (parent == nullptr)
                {
                    val.AddMember("parent", 0, allocator);
                }
                else
                {
                    val.AddMember("parent", parent->get_id(), allocator);
                }
                {
                    rapidjson::Value gates(rapidjson::kArrayType);
                    std::vector<Gate*> sorted = module->get_gates(nullptr, false);
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
                    for (const PinGroup<ModulePin>* pin_group : module->get_pin_groups())
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

                auto data = serialize(module->get_data_map(), allocator);
                if (!data.Empty())
                {
                    val.AddMember("data", data, allocator);
                }
                return val;
            }

            bool deserialize_module(Netlist* nl, const rapidjson::Value& val, std::unordered_map<Module*, std::vector<PinGroupInformation>>& pin_group_cache)
            {
                const u32 module_id           = val["id"].GetUint();
                const std::string module_name = val["name"].GetString();
                const u32 parent_id           = val["parent"].GetUint();
                Module* sm                    = nl->get_top_module();

                if (parent_id == 0)
                {
                    // top_module must not be created but might be renamed
                    const std::string top_module_name = val["name"].GetString();
                    if (top_module_name != sm->get_name())
                    {
                        sm->set_name(top_module_name);
                    }
                }
                else
                {
                    sm = nl->create_module(module_id, module_name, nl->get_module_by_id(parent_id));
                    if (sm == nullptr)
                    {
                        log_error("netlist_persistent", "could not deserialize module '" + module_name + "' with ID " + std::to_string(module_id) + ": failed to create module");
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

                if (val.HasMember("pin_groups"))
                {
                    // pins need to be cached until all modules have been instantiated
                    for (const auto& json_pin_group : val["pin_groups"].GetArray())
                    {
                        PinGroupInformation pin_group;
                        pin_group.id   = json_pin_group["id"].GetUint();
                        pin_group.name = json_pin_group["name"].GetString();
                        if (json_pin_group.HasMember("direction"))
                        {
                            pin_group.direction = enum_from_string<PinDirection>(json_pin_group["direction"].GetString());
                        }
                        else
                        {
                            pin_group.direction = PinDirection::none;
                        }
                        if (json_pin_group.HasMember("type"))
                        {
                            pin_group.type = enum_from_string<PinType>(json_pin_group["type"].GetString());
                        }
                        else
                        {
                            pin_group.type = PinType::none;
                        }
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
                                log_error("netlist_persistent",
                                          "could not deserialize pin '" + p.name + "' of module '" + sm->get_name() + "' with ID " + std::to_string(sm->get_id()) + ": failed to create pin\n{}",
                                          res.get_error().get());
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
                            log_error("netlist_persistent",
                                      "could not deserialize pin group '" + pg.name + "' of module '" + sm->get_name() + "' with ID " + std::to_string(sm->get_id())
                                          + ": failed to create pin group\n{}",
                                      res.get_error().get());
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
                        const Module* module = q.front();
                        q.pop();

                        modules.PushBack(serialize(module, allocator), allocator);

                        for (const Module* sm : module->get_submodules())
                        {
                            q.push(sm);
                        }
                    }
                    root.AddMember("modules", modules, allocator);
                }

                document.AddMember("netlist", root, document.GetAllocator());
            }

            std::unique_ptr<Netlist> deserialize(const rapidjson::Document& document, GateLibrary* gatelib)
            {
                if (!document.HasMember("netlist"))
                {
                    log_error("netlist_persistent", "could not deserialize netlist: file has no 'netlist' node");
                    return nullptr;
                }
                auto root = document["netlist"].GetObject();

                if (!gatelib)
                {
                    // no preferred gate library explicitly given
                    if (!root.HasMember("gate_library"))
                    {
                        log_error("netlist_persistent", "could not deserialize netlist: node 'netlist' has no node 'gate_library'");
                        return nullptr;
                    }

                    std::filesystem::path glib_path(root["gate_library"].GetString());

                    if (glib_path.is_relative())
                    {
                        ProjectManager* pm = ProjectManager::instance();
                        if (pm)
                            glib_path = pm->get_project_directory() / glib_path;
                    }
                    gatelib = gate_library_manager::get_gate_library(glib_path.string());

                    if (gatelib == nullptr)
                    {
                        // not found : try the other possible gate library extension
                        if (glib_path.extension() == ".hgl")
                        {
                            glib_path.replace_extension(".lib");
                        }
                        else
                        {
                            glib_path.replace_extension(".hgl");
                        }

                        gatelib = gate_library_manager::get_gate_library(glib_path.string());
                        if (gatelib == nullptr)
                        {
                            log_critical("netlist_persistent", "could not deserialize netlist: failed to load gate library '" + std::string(root["gate_library"].GetString()) + "'");
                            return nullptr;
                        }
                        else
                        {
                            log_info("netlist_persistent", "gate library '{}' required but using '{}' instead.", root["gate_library"].GetString(), glib_path.string());
                        }
                    }
                }

                auto nl = std::make_unique<Netlist>(gatelib);

                // disable automatically checking module nets
                nl->enable_automatic_net_checks(false);

                if (!root.HasMember("id"))
                {
                    log_error("netlist_persistent", "could not deserialize netlist: node 'netlist' has no node 'id'");
                    return nullptr;
                }
                nl->set_id(root["id"].GetUint());

                if (!root.HasMember("input_file"))
                {
                    log_error("netlist_persistent", "could not deserialize netlist: node 'netlist' has no node 'input_file'");
                    return nullptr;
                }
                nl->set_input_filename(root["input_file"].GetString());

                if (!root.HasMember("design_name"))
                {
                    log_error("netlist_persistent", "could not deserialize netlist: node 'netlist' has no node 'design_name'");
                    return nullptr;
                }
                nl->set_design_name(root["design_name"].GetString());

                if (!root.HasMember("device_name"))
                {
                    log_error("netlist_persistent", "could not deserialize netlist: node 'netlist' has no node 'device_name'");
                    return nullptr;
                }
                nl->set_device_name(root["device_name"].GetString());

                if (!root.HasMember("gates"))
                {
                    log_error("netlist_persistent", "could not deserialize netlist: node 'netlist' has no node 'gates'");
                    return nullptr;
                }
                auto gate_types = nl->get_gate_library()->get_gate_types();
                for (auto& gate_node : root["gates"].GetArray())
                {
                    if (!deserialize_gate(nl.get(), gate_node, gate_types))
                    {
                        log_error("netlist_persistent", "could not deserialize netlist: failed to deserialize gate");
                        return nullptr;
                    }
                }

                if (!root.HasMember("global_vcc"))
                {
                    log_error("netlist_persistent", "could not deserialize netlist: node 'netlist' has no node 'global_vcc'");
                    return nullptr;
                }
                for (auto& gate_node : root["global_vcc"].GetArray())
                {
                    if (!nl->mark_vcc_gate(nl->get_gate_by_id(gate_node.GetUint())))
                    {
                        log_error("netlist_persistent", "could not deserialize netlist: failed to mark VCC gate");
                        return nullptr;
                    }
                }

                if (!root.HasMember("global_gnd"))
                {
                    log_error("netlist_persistent", "could not deserialize netlist: node 'netlist' has no node 'global_gnd'");
                    return nullptr;
                }
                for (auto& gate_node : root["global_gnd"].GetArray())
                {
                    if (!nl->mark_gnd_gate(nl->get_gate_by_id(gate_node.GetUint())))
                    {
                        log_error("netlist_persistent", "could not deserialize netlist: failed to mark GND gate");
                        return nullptr;
                    }
                }

                if (!root.HasMember("nets"))
                {
                    log_error("netlist_persistent", "could not deserialize netlist: node 'netlist' has no node 'nets'");
                    return nullptr;
                }
                for (auto& net_node : root["nets"].GetArray())
                {
                    if (!deserialize_net(nl.get(), net_node))
                    {
                        log_error("netlist_persistent", "could not deserialize netlist: failed to deserialize net");
                        return nullptr;
                    }
                }

                if (!root.HasMember("global_in"))
                {
                    log_error("netlist_persistent", "could not deserialize netlist: node 'netlist' has no node 'global_in'");
                    return nullptr;
                }
                for (auto& net_node : root["global_in"].GetArray())
                {
                    if (!nl->mark_global_input_net(nl->get_net_by_id(net_node.GetUint())))
                    {
                        log_error("netlist_persistent", "could not deserialize netlist: failed to mark global input net");
                        return nullptr;
                    }
                }

                if (!root.HasMember("global_out"))
                {
                    log_error("netlist_persistent", "could not deserialize netlist: node 'netlist' has no node 'global_out'");
                    return nullptr;
                }
                for (auto& net_node : root["global_out"].GetArray())
                {
                    if (!nl->mark_global_output_net(nl->get_net_by_id(net_node.GetUint())))
                    {
                        log_error("netlist_persistent", "could not deserialize netlist: failed to mark global output net");
                        return nullptr;
                    }
                }

                if (!root.HasMember("modules"))
                {
                    log_error("netlist_persistent", "could not deserialize netlist: node 'netlist' has no node 'modules'");
                    return nullptr;
                }
                std::unordered_map<Module*, std::vector<PinGroupInformation>> pin_group_cache;
                for (auto& module_node : root["modules"].GetArray())
                {
                    if (!deserialize_module(nl.get(), module_node, pin_group_cache))
                    {
                        log_error("netlist_persistent", "could not deserialize netlist: failed to deserialize module");
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
                    log_error("netlist_persistent", "could not deserialize netlist: failed to deserialize module pins");
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

            std::filesystem::path serialize_to_dir = hal_file.parent_path();
            if (serialize_to_dir.empty())
                return false;

            if (serialize_to_dir.is_relative())
                serialize_to_dir = ProjectManager::instance()->get_project_directory() / serialize_to_dir;

            // create directory if it got erased in the meantime
            if (!std::filesystem::exists(serialize_to_dir))
            {
                std::error_code err;
                if (!std::filesystem::create_directories(serialize_to_dir, err))
                {
                    log_error("netlist_persistent", "Could not create directory '{}', error was '{}'.", serialize_to_dir.string(), err.message());
                    return false;
                }
            }

            std::ofstream hal_file_stream;
            hal_file_stream.open(hal_file.string());
            if (hal_file_stream.fail())
            {
                log_error("netlist_persistent", "could not open or create file {}: please verify that the file and the containing directory is writable", hal_file.string());
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

        std::unique_ptr<Netlist> deserialize_from_file(const std::filesystem::path& hal_file, GateLibrary* gatelib)
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

            auto netlist = deserialize(document, gatelib);

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
