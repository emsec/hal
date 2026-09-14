#include "hal_core/netlist/netlist_ir/netlist_ir.h"

#include "hal_core/utilities/utils.h"

#include <algorithm>
#include <set>
#include <unordered_map>
#include <unordered_set>

namespace hal
{
    namespace netlist_ir
    {
        // ---------------------------------------------------------------------------------------------------------
        // Range
        // ---------------------------------------------------------------------------------------------------------

        u32 Range::size() const
        {
            return static_cast<u32>((left > right) ? (left - right) : (right - left)) + 1;
        }

        bool Range::is_descending() const
        {
            return left > right;
        }

        i32 Range::index_at(u32 offset) const
        {
            return is_descending() ? left - static_cast<i32>(offset) : left + static_cast<i32>(offset);
        }

        std::optional<u32> Range::offset_of(i32 index) const
        {
            const i32 lo = std::min(left, right);
            const i32 hi = std::max(left, right);
            if (index < lo || index > hi)
            {
                return std::nullopt;
            }
            return static_cast<u32>(is_descending() ? left - index : index - left);
        }

        bool Range::operator==(const Range& other) const
        {
            return left == other.left && right == other.right;
        }

        bool Range::operator!=(const Range& other) const
        {
            return !(*this == other);
        }

        // ---------------------------------------------------------------------------------------------------------
        // Location
        // ---------------------------------------------------------------------------------------------------------

        std::string Location::to_string() const
        {
            if (line == 0)
            {
                return "";
            }
            if (column == 0)
            {
                return "line " + std::to_string(line);
            }
            return "line " + std::to_string(line) + ", column " + std::to_string(column);
        }

        // ---------------------------------------------------------------------------------------------------------
        // Signal
        // ---------------------------------------------------------------------------------------------------------

        u32 Signal::width() const
        {
            return static_cast<u32>(bits.size());
        }

        Result<BitId> Signal::bit_at(const std::vector<i32>& indices) const
        {
            if (indices.size() != dims.size())
            {
                return ERR("signal '" + name + "' has " + std::to_string(dims.size()) + " dimension(s) but " + std::to_string(indices.size()) + " indices were given");
            }

            // declaration order: the outermost dimension is the slowest, the innermost the fastest
            u32 position = 0;
            for (u32 d = 0; d < dims.size(); d++)
            {
                const auto offset = dims.at(d).offset_of(indices.at(d));
                if (!offset.has_value())
                {
                    return ERR("index " + std::to_string(indices.at(d)) + " is outside dimension " + std::to_string(d) + " of signal '" + name + "', which runs from " + std::to_string(dims.at(d).left)
                               + " to " + std::to_string(dims.at(d).right));
                }
                position = position * dims.at(d).size() + offset.value();
            }

            if (position >= bits.size())
            {
                return ERR("signal '" + name + "' has fewer bits than its dimensions require");
            }
            return OK(bits.at(position));
        }

        Result<std::vector<BitId>> Signal::slice(const Range& range) const
        {
            if (dims.size() != 1)
            {
                return ERR("signal '" + name + "' is not one-dimensional, cannot take a slice");
            }

            std::vector<BitId> res;
            res.reserve(range.size());
            for (u32 offset = 0; offset < range.size(); offset++)
            {
                const i32 index = range.index_at(offset);
                const auto pos  = dims.front().offset_of(index);
                if (!pos.has_value() || pos.value() >= bits.size())
                {
                    return ERR("index " + std::to_string(index) + " is outside signal '" + name + "', which runs from " + std::to_string(dims.front().left) + " to "
                               + std::to_string(dims.front().right));
                }
                res.push_back(bits.at(pos.value()));
            }
            return OK(res);
        }

        std::string Signal::bit_name(u32 position) const
        {
            if (dims.empty())
            {
                return name;
            }

            // unravel the position into one index per dimension, innermost first
            std::vector<i32> indices(dims.size(), 0);
            u32 rest = position;
            for (u32 d = static_cast<u32>(dims.size()); d-- > 0;)
            {
                const u32 size = dims.at(d).size();
                indices.at(d)  = dims.at(d).index_at(rest % size);
                rest /= size;
            }

            std::string res = name;
            for (const i32 index : indices)
            {
                res += "(" + std::to_string(index) + ")";
            }
            return res;
        }

        // ---------------------------------------------------------------------------------------------------------
        // Instance
        // ---------------------------------------------------------------------------------------------------------

        Connection& Instance::add_connection(const std::string& port, const std::vector<BitId>& bits, const std::optional<Range>& port_slice)
        {
            Connection& c = connections.emplace_back();
            c.port        = port;
            c.port_slice  = port_slice;
            c.bits        = bits;
            return c;
        }

        const Connection* Instance::find_connection(const std::string& port) const
        {
            for (const Connection& c : connections)
            {
                if (c.port == port)
                {
                    return &c;
                }
            }
            return nullptr;
        }

        // ---------------------------------------------------------------------------------------------------------
        // Module
        // ---------------------------------------------------------------------------------------------------------

        namespace
        {
            u32 total_width(const std::vector<Range>& dims)
            {
                u32 width = 1;
                for (const Range& r : dims)
                {
                    width *= r.size();
                }
                return width;
            }
        }    // namespace

        BitId Module::new_bit()
        {
            return m_next_bit++;
        }

        std::vector<BitId> Module::new_bits(u32 count)
        {
            std::vector<BitId> res;
            res.reserve(count);
            for (u32 i = 0; i < count; i++)
            {
                res.push_back(m_next_bit++);
            }
            return res;
        }

        BitId Module::next_bit() const
        {
            return m_next_bit;
        }

        Port& Module::add_port(const std::string& port_name, PinDirection direction, const std::vector<Range>& dims)
        {
            Port& p     = ports.emplace_back();
            p.name      = port_name;
            p.direction = direction;
            p.dims      = dims;
            p.bits      = new_bits(total_width(dims));
            return p;
        }

        Signal& Module::add_signal(const std::string& signal_name, const std::vector<Range>& dims)
        {
            Signal& s = signals.emplace_back();
            s.name    = signal_name;
            s.dims    = dims;
            s.bits    = new_bits(total_width(dims));
            return s;
        }

        Instance& Module::add_instance(const std::string& instance_name, const std::string& type, InstanceKind kind)
        {
            Instance& i = instances.emplace_back();
            i.name      = instance_name;
            i.type      = type;
            i.kind      = kind;
            return i;
        }

        void Module::add_alias(BitId a, BitId b)
        {
            aliases.emplace_back(a, b);
        }

        const Port* Module::find_port(const std::string& port_name) const
        {
            for (const Port& p : ports)
            {
                if (p.name == port_name)
                {
                    return &p;
                }
            }
            return nullptr;
        }

        const Signal* Module::find_signal(const std::string& signal_name) const
        {
            if (const Port* p = find_port(signal_name); p != nullptr)
            {
                return p;
            }
            for (const Signal& s : signals)
            {
                if (s.name == signal_name)
                {
                    return &s;
                }
            }
            return nullptr;
        }

        const Instance* Module::find_instance(const std::string& instance_name) const
        {
            for (const Instance& i : instances)
            {
                if (i.name == instance_name)
                {
                    return &i;
                }
            }
            return nullptr;
        }

        Instance* Module::find_instance(const std::string& instance_name)
        {
            for (Instance& i : instances)
            {
                if (i.name == instance_name)
                {
                    return &i;
                }
            }
            return nullptr;
        }

        // ---------------------------------------------------------------------------------------------------------
        // Design
        // ---------------------------------------------------------------------------------------------------------

        Module& Design::add_module(const std::string& module_name)
        {
            Module& m = modules.emplace_back();
            m.name    = module_name;
            return m;
        }

        const Module* Design::find_module(const std::string& module_name) const
        {
            for (const Module& m : modules)
            {
                if (m.name == module_name)
                {
                    return &m;
                }
            }
            return nullptr;
        }

        Module* Design::find_module(const std::string& module_name)
        {
            for (Module& m : modules)
            {
                if (m.name == module_name)
                {
                    return &m;
                }
            }
            return nullptr;
        }

        Result<std::string> Design::find_top() const
        {
            if (top.has_value())
            {
                if (find_module(top.value()) == nullptr)
                {
                    return ERR("the design names '" + top.value() + "' as its top module, but no module of that name exists");
                }
                return OK(top.value());
            }

            std::unordered_set<std::string> instantiated;
            for (const Module& m : modules)
            {
                for (const Instance& i : m.instances)
                {
                    if (i.kind == InstanceKind::Module)
                    {
                        instantiated.insert(i.type);
                    }
                }
            }

            std::vector<std::string> candidates;
            for (const Module& m : modules)
            {
                if (instantiated.find(m.name) == instantiated.end())
                {
                    candidates.push_back(m.name);
                }
            }

            if (candidates.empty())
            {
                return ERR("no module qualifies as the top module: every module is instantiated by another one");
            }
            if (candidates.size() > 1)
            {
                return ERR("found multiple modules as candidates for the top module, none of them marked as such: " + utils::join(", ", candidates));
            }
            return OK(candidates.front());
        }

        namespace
        {
            Result<std::monostate> validate_signal(const Module& m, const Signal& s, const char* what)
            {
                if (s.name.empty())
                {
                    return ERR("module '" + m.name + "' has a " + what + " without a name");
                }
                if (s.bits.size() != total_width(s.dims))
                {
                    return ERR(std::string(what) + " '" + s.name + "' of module '" + m.name + "' has " + std::to_string(s.bits.size()) + " bit(s) but its dimensions require "
                               + std::to_string(total_width(s.dims)));
                }
                for (const BitId bit : s.bits)
                {
                    if (bit < FIRST_USER_BIT || bit >= m.next_bit())
                    {
                        return ERR(std::string(what) + " '" + s.name + "' of module '" + m.name + "' owns bit " + std::to_string(bit) + ", which the module never allocated");
                    }
                }
                return OK({});
            }

            Result<std::monostate> validate_bits(const Module& m, const std::vector<BitId>& bits, const std::string& what)
            {
                for (const BitId bit : bits)
                {
                    if (bit >= m.next_bit())
                    {
                        return ERR(what + " of module '" + m.name + "' refers to bit " + std::to_string(bit) + ", which the module never allocated");
                    }
                }
                return OK({});
            }

            Result<std::monostate> validate_unique_names(const std::vector<TypedValue>& values, const std::string& owner, const char* what)
            {
                std::unordered_set<std::string> names;
                for (const TypedValue& v : values)
                {
                    if (v.declaration.get_name().empty())
                    {
                        return ERR(owner + " has a " + what + " without a name");
                    }
                    if (!names.insert(v.declaration.get_name()).second)
                    {
                        return ERR(owner + " has " + what + " '" + v.declaration.get_name() + "' twice");
                    }
                    if (!v.declaration.validate(v.value))
                    {
                        return ERR(owner + " has " + what + " '" + v.declaration.get_name() + "' with value '" + v.value + "', which is not valid for its type");
                    }
                }
                return OK({});
            }

            Result<std::monostate> validate_module(const Design& design, const Module& m)
            {
                if (m.name.empty())
                {
                    return ERR("a module has no name");
                }

                // ports and signals share one name space
                std::unordered_set<std::string> names;
                std::unordered_set<BitId> owned;
                for (const Port& p : m.ports)
                {
                    if (auto res = validate_signal(m, p, "port"); res.is_error())
                    {
                        return res;
                    }
                    if (p.direction != PinDirection::input && p.direction != PinDirection::output && p.direction != PinDirection::inout)
                    {
                        return ERR("port '" + p.name + "' of module '" + m.name + "' has no valid direction");
                    }
                    if (!names.insert(p.name).second)
                    {
                        return ERR("module '" + m.name + "' declares '" + p.name + "' twice");
                    }
                    for (const BitId bit : p.bits)
                    {
                        if (!owned.insert(bit).second)
                        {
                            return ERR("bit " + std::to_string(bit) + " of module '" + m.name + "' belongs to more than one port or signal");
                        }
                    }
                }
                for (const Signal& s : m.signals)
                {
                    if (auto res = validate_signal(m, s, "signal"); res.is_error())
                    {
                        return res;
                    }
                    if (!names.insert(s.name).second)
                    {
                        return ERR("module '" + m.name + "' declares '" + s.name + "' twice");
                    }
                    for (const BitId bit : s.bits)
                    {
                        if (!owned.insert(bit).second)
                        {
                            return ERR("bit " + std::to_string(bit) + " of module '" + m.name + "' belongs to more than one port or signal");
                        }
                    }
                }

                for (const auto& [a, b] : m.aliases)
                {
                    if (auto res = validate_bits(m, {a, b}, "an alias"); res.is_error())
                    {
                        return res;
                    }
                    if (a == b)
                    {
                        return ERR("module '" + m.name + "' aliases bit " + std::to_string(a) + " with itself");
                    }
                }

                if (auto res = validate_unique_names(m.parameters, "module '" + m.name + "'", "parameter"); res.is_error())
                {
                    return res;
                }
                if (auto res = validate_unique_names(m.attributes, "module '" + m.name + "'", "attribute"); res.is_error())
                {
                    return res;
                }
                for (const Port& p : m.ports)
                {
                    if (auto res = validate_unique_names(p.attributes, "port '" + p.name + "' of module '" + m.name + "'", "attribute"); res.is_error())
                    {
                        return res;
                    }
                }
                for (const Signal& sig : m.signals)
                {
                    if (auto res = validate_unique_names(sig.attributes, "signal '" + sig.name + "' of module '" + m.name + "'", "attribute"); res.is_error())
                    {
                        return res;
                    }
                }

                std::unordered_set<std::string> instance_names;
                for (const Instance& i : m.instances)
                {
                    if (i.name.empty())
                    {
                        return ERR("module '" + m.name + "' has an instance without a name");
                    }
                    if (!instance_names.insert(i.name).second)
                    {
                        return ERR("module '" + m.name + "' has two instances named '" + i.name + "'");
                    }
                    if (i.type.empty())
                    {
                        return ERR("instance '" + i.name + "' of module '" + m.name + "' has no type");
                    }

                    const Module* target = nullptr;
                    if (i.kind == InstanceKind::Module)
                    {
                        target = design.find_module(i.type);
                        if (target == nullptr)
                        {
                            return ERR("instance '" + i.name + "' of module '" + m.name + "' refers to module '" + i.type + "', which the design does not contain");
                        }
                        if (target == &m)
                        {
                            return ERR("module '" + m.name + "' instantiates itself as '" + i.name + "'");
                        }
                    }

                    std::unordered_set<std::string> connected_ports;
                    u32 named      = 0;
                    u32 positional = 0;
                    for (const Connection& c : i.connections)
                    {
                        if (auto res = validate_bits(m, c.bits, "a connection of instance '" + i.name + "'"); res.is_error())
                        {
                            return res;
                        }
                        if (c.port.empty())
                        {
                            positional++;
                            continue;
                        }
                        named++;
                        if (!connected_ports.insert(c.port).second)
                        {
                            return ERR("instance '" + i.name + "' of module '" + m.name + "' connects port '" + c.port + "' twice");
                        }
                        if (target != nullptr)
                        {
                            const Port* port = target->find_port(c.port);
                            if (port == nullptr)
                            {
                                return ERR("instance '" + i.name + "' of module '" + m.name + "' connects port '" + c.port + "', which module '" + target->name + "' does not have");
                            }
                            u32 expected = port->width();
                            if (c.port_slice.has_value())
                            {
                                if (port->dims.size() != 1)
                                {
                                    return ERR("instance '" + i.name + "' of module '" + m.name + "' connects a slice of port '" + c.port + "', which is not one-dimensional");
                                }
                                if (!port->dims.front().offset_of(c.port_slice->left).has_value() || !port->dims.front().offset_of(c.port_slice->right).has_value())
                                {
                                    return ERR("instance '" + i.name + "' of module '" + m.name + "' connects a slice of port '" + c.port + "' that lies outside the port");
                                }
                                expected = c.port_slice->size();
                            }
                            if (c.bits.size() != expected)
                            {
                                return ERR("instance '" + i.name + "' of module '" + m.name + "' connects " + std::to_string(c.bits.size()) + " bit(s) to port '" + c.port + "' of module '"
                                           + target->name + "', which has " + std::to_string(expected));
                            }
                        }
                    }
                    if (named > 0 && positional > 0)
                    {
                        return ERR("instance '" + i.name + "' of module '" + m.name + "' mixes named and positional connections");
                    }
                    if (target != nullptr && positional > target->ports.size())
                    {
                        return ERR("instance '" + i.name + "' of module '" + m.name + "' has " + std::to_string(positional) + " positional connections but module '" + target->name + "' has only "
                                   + std::to_string(target->ports.size()) + " ports");
                    }

                    if (auto res = validate_unique_names(i.parameters, "instance '" + i.name + "' of module '" + m.name + "'", "parameter"); res.is_error())
                    {
                        return res;
                    }
                    if (auto res = validate_unique_names(i.attributes, "instance '" + i.name + "' of module '" + m.name + "'", "attribute"); res.is_error())
                    {
                        return res;
                    }
                }

                return OK({});
            }
        }    // namespace

        Result<std::monostate> Design::validate() const
        {
            std::unordered_set<std::string> module_names;
            for (const Module& m : modules)
            {
                if (!module_names.insert(m.name).second)
                {
                    return ERR("the design declares module '" + m.name + "' twice");
                }
            }

            for (const Module& m : modules)
            {
                if (auto res = validate_module(*this, m); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "design '" + source + "' is not valid");
                }
            }

            if (top.has_value() && find_module(top.value()) == nullptr)
            {
                return ERR("the design names '" + top.value() + "' as its top module, but no module of that name exists");
            }

            return OK({});
        }
    }    // namespace netlist_ir
}    // namespace hal
