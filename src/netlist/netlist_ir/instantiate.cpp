#include "hal_core/netlist/netlist_ir/instantiate.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/utils.h"

#include <algorithm>
#include <functional>
#include <map>
#include <numeric>
#include <unordered_map>
#include <unordered_set>

namespace hal
{
    namespace netlist_ir
    {
        namespace
        {
            /**
             * One visit of a module in the hierarchy walk: the top module once, every other module once per instance.
             */
            struct Visit
            {
                const Module* module     = nullptr;
                const Instance* instance = nullptr;    // the instance that created this visit, nullptr for the top
                u32 parent               = 0;          // index of the parent visit, itself for the top
                std::string path;                      // instance path without the top, e.g. "a/b"
                std::string name;                      // the plain instance name, or the top module name
                u64 offset = 0;                        // first global key of this visit's bits
                u32 depth  = 0;
            };

            struct UnionFind
            {
                std::vector<u64> parent;

                explicit UnionFind(u64 n) : parent(n)
                {
                    std::iota(parent.begin(), parent.end(), 0);
                }

                u64 find(u64 x)
                {
                    while (parent[x] != x)
                    {
                        parent[x] = parent[parent[x]];
                        x         = parent[x];
                    }
                    return x;
                }

                void unite(u64 a, u64 b)
                {
                    a = find(a);
                    b = find(b);
                    if (a != b)
                    {
                        parent[std::max(a, b)] = std::min(a, b);    // the constants keep the lowest keys, so they stay roots
                    }
                }
            };

            /**
             * Where a gate pin has to be connected to, collected while walking, applied once the nets exist.
             */
            struct PinRequest
            {
                u32 visit                = 0;
                const Instance* instance = nullptr;
                GateType* type           = nullptr;
                GatePin* pin             = nullptr;
                u64 key                  = 0;
            };

            /**
             * A candidate name of a class of bits.
             */
            struct NameCandidate
            {
                std::string name;
                std::string path;
                bool is_top_port = false;
                bool is_port     = false;
                u32 order        = 0;

                bool better_than(const NameCandidate& other) const
                {
                    if (is_top_port != other.is_top_port)
                    {
                        return is_top_port;
                    }
                    if (path.size() != other.path.size())
                    {
                        return path.size() < other.path.size();
                    }
                    if (is_port != other.is_port)
                    {
                        return is_port;
                    }
                    return order < other.order;
                }
            };

            /**
             * Names objects the way the netlist parsers have always done: the plain name if it is unique across the
             * netlist, the name prefixed with the instance path otherwise, and a numeric suffix if that is taken too.
             */
            class UniqueNamer
            {
            public:
                explicit UniqueNamer(const std::string& separator) : m_separator(separator)
                {
                }

                void count(const std::string& name)
                {
                    m_occurrences[name]++;
                }

                std::string assign(const std::string& name, const std::string& path)
                {
                    std::string candidate = name;
                    if (!path.empty() && m_occurrences[name] > 1)
                    {
                        candidate = path + m_separator + name;
                    }
                    std::string unique = candidate;
                    for (u32 i = 2; m_taken.find(unique) != m_taken.end(); i++)
                    {
                        unique = candidate + "_u" + std::to_string(i);
                    }
                    if (unique != candidate)
                    {
                        log_warning("netlist_ir", "name '{}' is used more than once after flattening the hierarchy, renaming one occurrence to '{}'", candidate, unique);
                    }
                    m_taken.insert(unique);
                    return unique;
                }

            private:
                std::string m_separator;
                std::unordered_map<std::string, u32> m_occurrences;
                std::unordered_set<std::string> m_taken;
            };

            std::string describe(const Visit& v, const Instance* instance)
            {
                std::string res = "instance '" + (v.path.empty() ? instance->name : v.path + "/" + instance->name) + "' of type '" + instance->type + "'";
                if (instance->location.line != 0)
                {
                    res += " (" + instance->location.to_string() + ")";
                }
                return res;
            }

            /**
             * Look a name up exactly first, then case-insensitively if that gives exactly one match.
             */
            template<typename T>
            T* resolve_name(const std::string& name, const std::vector<T*>& candidates, const std::function<std::string(const T*)>& name_of)
            {
                for (T* c : candidates)
                {
                    if (name_of(c) == name)
                    {
                        return c;
                    }
                }
                const std::string lower = utils::to_lower(name);
                T* match                = nullptr;
                for (T* c : candidates)
                {
                    if (utils::to_lower(name_of(c)) == lower)
                    {
                        if (match != nullptr)
                        {
                            return nullptr;    // ambiguous
                        }
                        match = c;
                    }
                }
                return match;
            }

            bool is_constant(BitId bit)
            {
                return bit < FIRST_USER_BIT;
            }

            /**
             * Pair the bits of an expression with the pins of a pin group, low bits first. Returns pairs of (pin, bit);
             * a narrower constant expression is zero-extended, a narrower signal leaves the high pins open, a wider
             * expression keeps its low bits.
             */
            std::vector<std::pair<GatePin*, BitId>> pair_bits_with_pins(const std::vector<BitId>& bits, PinGroup<GatePin>* group, const std::string& what)
            {
                std::vector<std::pair<GatePin*, BitId>> res;
                const u32 width = static_cast<u32>(group->get_pins().size());
                if (bits.size() > width)
                {
                    log_warning("netlist_ir", "{}: {} bits are connected to pin group '{}' of width {}, the high bits are dropped", what, bits.size(), group->get_name(), width);
                }
                const bool all_constant = std::all_of(bits.begin(), bits.end(), is_constant);
                for (u32 k = 0; k < width; k++)
                {
                    const i32 index = group->get_lowest_index() + static_cast<i32>(k);
                    auto pin_res    = group->get_pin_at_index(index);
                    if (pin_res.is_error())
                    {
                        continue;
                    }
                    if (k < bits.size())
                    {
                        res.emplace_back(pin_res.get(), bits.at(bits.size() - 1 - k));
                    }
                    else if (all_constant && !bits.empty())
                    {
                        res.emplace_back(pin_res.get(), ZERO);
                    }
                }
                return res;
            }

            Result<std::monostate> apply_typed_values(DataContainer* target, const std::vector<TypedValue>& values, bool as_parameters, const GateType* declaring_type, const std::string& what)
            {
                for (const TypedValue& v : values)
                {
                    Parameter declaration = v.declaration;
                    if (as_parameters && declaring_type != nullptr)
                    {
                        if (auto declared = declaring_type->get_parameter(v.declaration.get_name()); declared.is_ok())
                        {
                            declaration = declared.get();
                        }
                    }
                    const auto res = as_parameters ? target->set_parameter(declaration, v.value) : target->set_attribute(declaration, v.value);
                    if (res.is_error())
                    {
                        return ERR_APPEND(res.get_error(), what + ": cannot set " + (as_parameters ? "parameter '" : "attribute '") + v.declaration.get_name() + "' to '" + v.value + "'");
                    }
                }
                return OK({});
            }
        }    // namespace

        Result<std::unique_ptr<Netlist>> instantiate(const Design& design, const GateLibrary* gate_library, const InstantiationOptions& options)
        {
            if (gate_library == nullptr)
            {
                return ERR("cannot instantiate design '" + design.source + "': no gate library given");
            }
            if (auto res = design.validate(); res.is_error())
            {
                return ERR_APPEND(res.get_error(), "cannot instantiate design '" + design.source + "'");
            }
            const auto top_res = design.find_top();
            if (top_res.is_error())
            {
                return ERR_APPEND(top_res.get_error(), "cannot instantiate design '" + design.source + "'");
            }
            const Module* top = design.find_module(top_res.get());

            // ---- walk the hierarchy: one visit per module instance, keys for every bit of every visit -------------
            std::vector<Visit> visits;
            {
                Visit root;
                root.module = top;
                root.name   = options.top_module_name;
                root.offset = FIRST_USER_BIT;
                visits.push_back(root);
            }
            u64 next_key = FIRST_USER_BIT + top->next_bit();
            for (u32 v = 0; v < visits.size(); v++)
            {
                const Module* m = visits.at(v).module;
                for (const Instance& inst : m->instances)
                {
                    if (inst.kind != InstanceKind::Module)
                    {
                        continue;
                    }
                    const Module* child = design.find_module(inst.type);

                    // a module on the stack of ancestors instantiating itself is a cycle
                    for (u32 a = v;; a = visits.at(a).parent)
                    {
                        if (visits.at(a).module == child)
                        {
                            return ERR("cannot instantiate design '" + design.source + "': module '" + child->name + "' instantiates itself through " + describe(visits.at(v), &inst));
                        }
                        if (visits.at(a).parent == a)
                        {
                            break;
                        }
                    }

                    Visit c;
                    c.module   = child;
                    c.instance = &inst;
                    c.parent   = v;
                    c.path     = visits.at(v).path.empty() ? inst.name : visits.at(v).path + options.instance_name_separator + inst.name;
                    c.name     = inst.name;
                    c.offset   = next_key;
                    c.depth    = visits.at(v).depth + 1;
                    next_key += child->next_bit();
                    visits.push_back(c);
                }
            }

            const auto key = [&visits](u32 v, BitId bit) -> u64 { return is_constant(bit) ? static_cast<u64>(bit) : visits.at(v).offset + bit; };

            // ---- resolve aliases, module port connections and gate pins ---------------------------------------------
            UnionFind uf(next_key);
            std::vector<PinRequest> pin_requests;
            std::map<std::pair<u32, const Instance*>, GateType*> gate_type_of;

            std::vector<GateType*> library_types;
            for (const auto& [name, type] : gate_library->get_gate_types())
            {
                library_types.push_back(type);
            }
            std::sort(library_types.begin(), library_types.end(), [](const GateType* a, const GateType* b) { return a->get_name() < b->get_name(); });

            for (u32 v = 0; v < visits.size(); v++)
            {
                const Visit& visit = visits.at(v);
                const Module* m    = visit.module;

                for (const auto& [a, b] : m->aliases)
                {
                    uf.unite(key(v, a), key(v, b));
                }
            }

            // child visits are in creation order, so the connection of every child can be resolved from its parent
            for (u32 v = 1; v < visits.size(); v++)
            {
                const Visit& visit     = visits.at(v);
                const Instance& inst   = *visit.instance;
                const Module* child    = visit.module;
                const std::string what = describe(visits.at(visit.parent), &inst);

                u32 positional = 0;
                for (const Connection& c : inst.connections)
                {
                    const Port* port = nullptr;
                    std::vector<BitId> port_bits;
                    if (c.port.empty())
                    {
                        if (positional >= child->ports.size())
                        {
                            return ERR(what + ": more positional connections than module '" + child->name + "' has ports");
                        }
                        port      = &child->ports.at(positional++);
                        port_bits = port->bits;
                    }
                    else
                    {
                        port = child->find_port(c.port);    // validated to exist
                        if (c.port_slice.has_value())
                        {
                            auto slice = port->slice(c.port_slice.value());
                            if (slice.is_error())
                            {
                                return ERR_APPEND(slice.get_error(), what + ": invalid slice of port '" + c.port + "'");
                            }
                            port_bits = slice.get();
                        }
                        else
                        {
                            port_bits = port->bits;
                        }
                    }

                    // low bits pair up; a mismatch on a positional connection is tolerated like on a gate pin group
                    if (c.bits.size() != port_bits.size())
                    {
                        log_warning("netlist_ir", "{}: {} bits are connected to port '{}' of width {}, only the low bits are connected", what, c.bits.size(), port->name, port_bits.size());
                    }
                    const bool all_constant = std::all_of(c.bits.begin(), c.bits.end(), is_constant);
                    for (u32 k = 0; k < port_bits.size(); k++)
                    {
                        const BitId port_bit = port_bits.at(port_bits.size() - 1 - k);
                        if (k < c.bits.size())
                        {
                            uf.unite(key(v, port_bit), key(visit.parent, c.bits.at(c.bits.size() - 1 - k)));
                        }
                        else if (all_constant && !c.bits.empty())
                        {
                            uf.unite(key(v, port_bit), ZERO);
                        }
                    }
                }
            }

            for (u32 v = 0; v < visits.size(); v++)
            {
                const Visit& visit = visits.at(v);
                for (const Instance& inst : visit.module->instances)
                {
                    if (inst.kind != InstanceKind::Gate)
                    {
                        continue;
                    }
                    const std::string what = describe(visit, &inst);

                    GateType* type = resolve_name<GateType>(inst.type, library_types, [](const GateType* t) { return t->get_name(); });
                    if (type == nullptr)
                    {
                        return ERR(what + ": gate type '" + inst.type + "' does not exist in gate library '" + gate_library->get_name() + "'");
                    }
                    gate_type_of[{v, &inst}] = type;

                    const std::vector<PinGroup<GatePin>*> groups = type->get_pin_groups();
                    const std::vector<GatePin*> pins             = type->get_pins();

                    u32 positional = 0;
                    for (const Connection& c : inst.connections)
                    {
                        std::vector<std::pair<GatePin*, BitId>> pairs;
                        if (c.port.empty())
                        {
                            if (positional >= groups.size())
                            {
                                return ERR(what + ": more positional connections than gate type '" + type->get_name() + "' has pin groups");
                            }
                            pairs = pair_bits_with_pins(c.bits, groups.at(positional++), what);
                        }
                        else if (PinGroup<GatePin>* group = resolve_name<PinGroup<GatePin>>(c.port, groups, [](const PinGroup<GatePin>* g) { return g->get_name(); }); group != nullptr)
                        {
                            if (c.port_slice.has_value())
                            {
                                // a slice of a pin group: the bits go to the pins at the indices of the slice
                                const Range& r = c.port_slice.value();
                                if (c.bits.size() != r.size())
                                {
                                    return ERR(what + ": " + std::to_string(c.bits.size()) + " bits are connected to a slice of width " + std::to_string(r.size()) + " of pin group '"
                                               + group->get_name() + "'");
                                }
                                for (u32 k = 0; k < r.size(); k++)
                                {
                                    auto pin_res = group->get_pin_at_index(r.index_at(k));
                                    if (pin_res.is_error())
                                    {
                                        return ERR(what + ": index " + std::to_string(r.index_at(k)) + " is outside pin group '" + group->get_name() + "'");
                                    }
                                    pairs.emplace_back(pin_res.get(), c.bits.at(k));
                                }
                            }
                            else
                            {
                                pairs = pair_bits_with_pins(c.bits, group, what);
                            }
                        }
                        else if (GatePin* pin = resolve_name<GatePin>(c.port, pins, [](const GatePin* p) { return p->get_name(); }); pin != nullptr)
                        {
                            if (c.bits.size() != 1)
                            {
                                log_warning("netlist_ir", "{}: {} bits are connected to the single pin '{}', only the low bit is connected", what, c.bits.size(), pin->get_name());
                            }
                            if (!c.bits.empty())
                            {
                                pairs.emplace_back(pin, c.bits.back());
                            }
                        }
                        else
                        {
                            return ERR(what + ": gate type '" + type->get_name() + "' has no pin or pin group named '" + c.port + "'");
                        }

                        for (const auto& [pin, bit] : pairs)
                        {
                            pin_requests.push_back({v, &inst, type, pin, key(v, bit)});
                        }
                    }
                }
            }

            // ---- decide which classes become nets, and how they are named ------------------------------------------
            struct ClassInfo
            {
                bool needs_net     = false;
                bool global_input  = false;
                bool global_output = false;
                std::optional<NameCandidate> best;
                std::vector<TypedValue> attributes;
            };
            std::map<u64, ClassInfo> classes;

            for (const PinRequest& r : pin_requests)
            {
                classes[uf.find(r.key)].needs_net = true;
            }

            u32 order = 0;
            for (u32 v = 0; v < visits.size(); v++)
            {
                const Visit& visit  = visits.at(v);
                const auto consider = [&](const Signal& s, bool is_port, PinDirection direction) {
                    for (u32 i = 0; i < s.bits.size(); i++)
                    {
                        const u64 root      = uf.find(key(v, s.bits.at(i)));
                        ClassInfo& info     = classes[root];
                        const bool top_port = is_port && v == 0;
                        if (top_port || options.keep_unconnected_signals || !s.attributes.empty())
                        {
                            info.needs_net = true;
                        }
                        if (top_port && (direction == PinDirection::input || direction == PinDirection::inout))
                        {
                            info.global_input = true;
                        }
                        if (top_port && (direction == PinDirection::output || direction == PinDirection::inout))
                        {
                            info.global_output = true;
                        }
                        NameCandidate cand{s.bit_name(i), visit.path, top_port, is_port, order++};
                        if (!info.best.has_value() || cand.better_than(info.best.value()))
                        {
                            info.best = cand;
                        }
                        info.attributes.insert(info.attributes.end(), s.attributes.begin(), s.attributes.end());
                    }
                };
                for (const Port& p : visit.module->ports)
                {
                    consider(p, true, p.direction);
                }
                for (const Signal& s : visit.module->signals)
                {
                    consider(s, false, PinDirection::none);
                }
            }

            // ---- create the netlist ------------------------------------------------------------------------------
            auto netlist = netlist_factory::create_netlist(gate_library);
            if (netlist == nullptr)
            {
                return ERR("cannot instantiate design '" + design.source + "': failed to create a netlist for gate library '" + gate_library->get_name() + "'");
            }
            Netlist* nl = netlist.get();
            nl->enable_automatic_net_checks(false);
            nl->set_design_name(top->name);

            // modules, in visit order so that parents exist before children
            UniqueNamer module_namer(options.instance_name_separator);
            for (u32 v = 1; v < visits.size(); v++)
            {
                module_namer.count(visits.at(v).name);
            }
            std::vector<hal::Module*> module_of_visit(visits.size(), nullptr);
            module_of_visit.at(0) = nl->get_top_module();
            module_of_visit.at(0)->set_name(options.top_module_name);
            module_of_visit.at(0)->set_type(top->name);
            for (u32 v = 1; v < visits.size(); v++)
            {
                const Visit& visit     = visits.at(v);
                const std::string name = module_namer.assign(visit.name, visits.at(visit.parent).path);
                hal::Module* m         = nl->create_module(name, module_of_visit.at(visit.parent));
                if (m == nullptr)
                {
                    return ERR("cannot instantiate design '" + design.source + "': failed to create module '" + name + "'");
                }
                m->set_type(visit.module->name);
                module_of_visit.at(v) = m;
            }
            for (u32 v = 0; v < visits.size(); v++)
            {
                const Visit& visit     = visits.at(v);
                hal::Module* m         = module_of_visit.at(v);
                const std::string what = v == 0 ? "top module '" + top->name + "'" : describe(visits.at(visit.parent), visit.instance);

                // the module's declared defaults first, then the instance's overrides on top
                if (auto res = apply_typed_values(m, visit.module->parameters, true, nullptr, what); res.is_error())
                {
                    return ERR(res.get_error());
                }
                if (auto res = apply_typed_values(m, visit.module->attributes, false, nullptr, what); res.is_error())
                {
                    return ERR(res.get_error());
                }
                if (visit.instance != nullptr)
                {
                    std::vector<TypedValue> overrides;
                    for (const TypedValue& tv : visit.instance->parameters)
                    {
                        TypedValue effective = tv;
                        for (const TypedValue& declared : visit.module->parameters)
                        {
                            if (declared.declaration.get_name() == tv.declaration.get_name() && declared.declaration.validate(tv.value))
                            {
                                effective.declaration = declared.declaration;
                            }
                        }
                        overrides.push_back(effective);
                    }
                    if (auto res = apply_typed_values(m, overrides, true, nullptr, what); res.is_error())
                    {
                        return ERR(res.get_error());
                    }
                    if (auto res = apply_typed_values(m, visit.instance->attributes, false, nullptr, what); res.is_error())
                    {
                        return ERR(res.get_error());
                    }
                }
            }

            // nets
            UniqueNamer net_namer(options.instance_name_separator);
            for (const auto& [root, info] : classes)
            {
                if (info.needs_net && info.best.has_value() && !is_constant(root))
                {
                    net_namer.count(info.best->name);
                }
            }
            std::unordered_map<u64, Net*> net_of_class;
            for (auto& [root, info] : classes)
            {
                if (!info.needs_net)
                {
                    continue;
                }
                std::string name;
                if (is_constant(root) && (!info.best.has_value() || !info.best->is_top_port))
                {
                    name = root == ZERO ? "'0'" : "'1'";
                }
                else
                {
                    name = net_namer.assign(info.best->name, info.best->path);
                }
                Net* net = nl->create_net(name);
                if (net == nullptr)
                {
                    return ERR("cannot instantiate design '" + design.source + "': failed to create net '" + name + "'");
                }
                if (info.global_input)
                {
                    net->mark_global_input_net();
                }
                if (info.global_output)
                {
                    net->mark_global_output_net();
                }
                if (auto res = apply_typed_values(net, info.attributes, false, nullptr, "net '" + name + "'"); res.is_error())
                {
                    return ERR(res.get_error());
                }
                net_of_class[root] = net;
            }

            // the constants are driven by one GND and one VCC gate each, when anything uses them
            for (const BitId constant : {ZERO, ONE})
            {
                const auto net_it = net_of_class.find(constant);
                if (net_it == net_of_class.end())
                {
                    continue;
                }
                const bool zero        = constant == ZERO;
                const auto types       = zero ? gate_library->get_gnd_gate_types() : gate_library->get_vcc_gate_types();
                const std::string want = zero ? options.gnd_gate_type : options.vcc_gate_type;
                GateType* type         = nullptr;
                if (!want.empty())
                {
                    type = gate_library->get_gate_type_by_name(want);
                }
                else if (!types.empty())
                {
                    type = std::min_element(types.begin(), types.end(), [](const auto& a, const auto& b) { return a.first < b.first; })->second;
                }
                if (type == nullptr || type->get_output_pins().empty())
                {
                    return ERR("cannot instantiate design '" + design.source + "': the design uses the constant " + (zero ? "0" : "1") + " but gate library '" + gate_library->get_name()
                               + "' has no usable " + (zero ? "GND" : "VCC") + " gate type");
                }
                Gate* g = nl->create_gate(type, zero ? "global_gnd" : "global_vcc");
                if (g == nullptr)
                {
                    return ERR("cannot instantiate design '" + design.source + "': failed to create the " + (zero ? "GND" : "VCC") + " gate");
                }
                if (zero)
                {
                    nl->mark_gnd_gate(g);
                }
                else
                {
                    nl->mark_vcc_gate(g);
                }
                if (net_it->second->add_source(g, type->get_output_pins().front()) == nullptr)
                {
                    return ERR("cannot instantiate design '" + design.source + "': failed to connect the " + (zero ? "GND" : "VCC") + " gate");
                }
            }

            // gates
            UniqueNamer gate_namer(options.instance_name_separator);
            for (const Visit& visit : visits)
            {
                for (const Instance& inst : visit.module->instances)
                {
                    if (inst.kind == InstanceKind::Gate)
                    {
                        gate_namer.count(inst.name);
                    }
                }
            }
            std::map<std::pair<u32, const Instance*>, Gate*> gate_of;
            for (u32 v = 0; v < visits.size(); v++)
            {
                const Visit& visit = visits.at(v);
                for (const Instance& inst : visit.module->instances)
                {
                    if (inst.kind != InstanceKind::Gate)
                    {
                        continue;
                    }
                    const std::string what = describe(visit, &inst);
                    GateType* type         = gate_type_of.at({v, &inst});
                    Gate* g                = nl->create_gate(type, gate_namer.assign(inst.name, visit.path));
                    if (g == nullptr)
                    {
                        return ERR(what + ": failed to create the gate");
                    }
                    if (v != 0 && !module_of_visit.at(v)->assign_gate(g))
                    {
                        return ERR(what + ": failed to assign the gate to its module");
                    }
                    if (auto res = apply_typed_values(g, inst.parameters, true, type, what); res.is_error())
                    {
                        return ERR(res.get_error());
                    }
                    if (auto res = apply_typed_values(g, inst.attributes, false, nullptr, what); res.is_error())
                    {
                        return ERR(res.get_error());
                    }
                    gate_of[{v, &inst}] = g;
                }
            }

            for (const PinRequest& r : pin_requests)
            {
                Gate* g                      = gate_of.at({r.visit, r.instance});
                Net* net                     = net_of_class.at(uf.find(r.key));
                const PinDirection direction = r.pin->get_direction();
                const std::string what       = describe(visits.at(r.visit), r.instance);
                if (direction == PinDirection::input || direction == PinDirection::inout)
                {
                    if (net->add_destination(g, r.pin) == nullptr)
                    {
                        return ERR(what + ": failed to connect net '" + net->get_name() + "' to pin '" + r.pin->get_name() + "'");
                    }
                }
                if (direction == PinDirection::output || direction == PinDirection::inout)
                {
                    if (net->add_source(g, r.pin) == nullptr)
                    {
                        return ERR(what + ": failed to connect net '" + net->get_name() + "' to pin '" + r.pin->get_name() + "'");
                    }
                }
                if (direction != PinDirection::input && direction != PinDirection::output && direction != PinDirection::inout)
                {
                    log_warning("netlist_ir", "{}: pin '{}' has no usable direction, the connection is dropped", what, r.pin->get_name());
                }
            }

            // module pins carry the port names of the design
            for (u32 v = 1; v < visits.size(); v++)
            {
                const Visit& visit = visits.at(v);
                hal::Module* m     = module_of_visit.at(v);
                m->update_nets();
                for (const Port& p : visit.module->ports)
                {
                    for (u32 i = 0; i < p.bits.size(); i++)
                    {
                        const auto net_it = net_of_class.find(uf.find(key(v, p.bits.at(i))));
                        if (net_it == net_of_class.end())
                        {
                            continue;
                        }
                        Net* net = net_it->second;
                        if ((!m->is_input_net(net) && !m->is_output_net(net)) || m->get_pin_by_net(net) != nullptr)
                        {
                            continue;
                        }
                        if (auto res = m->create_pin(p.bit_name(i), net); res.is_error())
                        {
                            return ERR_APPEND(res.get_error(), "cannot instantiate design '" + design.source + "': failed to create pin '" + p.bit_name(i) + "' of module '" + m->get_name() + "'");
                        }
                    }
                }
            }

            nl->enable_automatic_net_checks(true);
            return OK(std::move(netlist));
        }
    }    // namespace netlist_ir
}    // namespace hal
