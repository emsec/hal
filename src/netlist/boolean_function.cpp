#include "hal_core/netlist/boolean_function.h"

#include "hal_core/netlist/boolean_function/parser.h"
#include "hal_core/netlist/boolean_function/simplification.h"
#include "hal_core/netlist/boolean_function/symbolic_execution.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/utils.h"

#include <algorithm>
#include <bitset>
#include <boost/spirit/home/x3.hpp>
#include <chrono>
#include <map>

namespace hal
{
    template<>
    std::map<BooleanFunction::Value, std::string> EnumStrings<BooleanFunction::Value>::data = {{BooleanFunction::Value::ZERO, "0"},
                                                                                               {BooleanFunction::Value::ONE, "1"},
                                                                                               {BooleanFunction::Value::X, "X"},
                                                                                               {BooleanFunction::Value::Z, "Z"}};

    std::string BooleanFunction::to_string(Value v)
    {
        switch (v)
        {
            case ZERO:
                return std::string("0");
            case ONE:
                return std::string("1");
            case X:
                return std::string("X");
            case Z:
                return std::string("Z");
        }

        return std::string("X");
    }

    namespace
    {
        static std::vector<char> char_map = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
        // namespace

        static Result<std::string> to_bin(const std::vector<BooleanFunction::Value>& value)
        {
            if (value.size() == 0)
            {
                return ERR("could not convert bit-vector to binary string: bit-vector is empty");
            }

            std::string res = "";
            res.reserve(value.size());

            for (auto v : value)
            {
                res += enum_to_string<BooleanFunction::Value>(v);
            }

            return OK(res);
        }

        static Result<std::string> to_oct(const std::vector<BooleanFunction::Value>& value)
        {
            int bitsize = value.size();
            if (bitsize == 0)
            {
                return ERR("could not convert bit-vector to octal string: bit-vector is empty");
            }

            u8 first_bits = bitsize % 3;

            u8 index = 0;
            u8 mask  = 0;

            u8 v1, v2, v3;

            // result string prep
            std::string res = "";
            res.reserve((bitsize + 2) / 3);

            // deal with 0-3 leading bits
            for (u8 i = 0; i < first_bits; i++)
            {
                v1    = value.at(i);
                index = (index << 1) | v1;
                mask |= v1;
            }
            mask = -((mask >> 1) & 0x1);
            if (first_bits)
            {
                res += (char_map[index] & ~mask) | ('X' & mask);
            }

            // deal with 4-bit blocks (left to right)
            for (int i = bitsize % 3; i < bitsize; i += 3)
            {
                v1 = value[i];
                v2 = value[i + 1];
                v3 = value[i + 2];

                index = (v1 << 2) | (v2 << 1) | v3;    // cannot exceed char_map range as index always < 16, no further check required
                mask  = -(((v1 | v2 | v3) >> 1) & 0x1);

                res += (char_map[index] & ~mask) | ('X' & mask);
            }
            return OK(res);
        }

        static Result<std::string> to_dec(const std::vector<BooleanFunction::Value>& value)
        {
            int bitsize = value.size();
            if (bitsize == 0)
            {
                return ERR("could not convert bit-vector to decimal string: bit-vector is empty");
            }

            if (bitsize > 64)
            {
                return ERR("could not convert bit-vector to decimal string: bit-vector has length " + std::to_string(bitsize) + ", but only up to 64 bits are supported for decimal conversion");
            }

            u64 tmp   = 0;
            u8 x_flag = 0;
            for (auto v : value)
            {
                x_flag |= v >> 1;
                tmp = (tmp << 1) | v;
            }

            if (x_flag)
            {
                return OK(std::string("X"));
            }
            return OK(std::to_string(tmp));
        }

        static Result<std::string> to_hex(const std::vector<BooleanFunction::Value>& value)
        {
            int bitsize = value.size();
            if (bitsize == 0)
            {
                return ERR("could not convert bit-vector to hexadecimal string: bit-vector is empty");
            }

            u8 first_bits = bitsize & 0x3;

            u8 index = 0;
            u8 mask  = 0;

            u8 v1, v2, v3, v4;

            // result string prep
            std::string res = "";
            res.reserve((bitsize + 3) / 4);

            // deal with 0-3 leading bits
            for (u8 i = 0; i < first_bits; i++)
            {
                v1    = value.at(i);
                index = (index << 1) | v1;
                mask |= v1;
            }
            mask = -((mask >> 1) & 0x1);
            if (first_bits)
            {
                res += (char_map[index] & ~mask) | ('X' & mask);
            }

            // deal with 4-bit blocks (left to right)
            for (int i = bitsize & 0x3; i < bitsize; i += 4)
            {
                v1 = value[i];
                v2 = value[i + 1];
                v3 = value[i + 2];
                v4 = value[i + 3];

                index = ((v1 << 3) | (v2 << 2) | (v3 << 1) | v4) & 0xF;
                mask  = -(((v1 | v2 | v3 | v4) >> 1) & 0x1);

                res += (char_map[index] & ~mask) | ('X' & mask);
            }

            return OK(res);
        }
    }    // namespace

    Result<std::string> BooleanFunction::to_string(const std::vector<BooleanFunction::Value>& value, u8 base)
    {
        switch (base)
        {
            case 2:
                return to_bin(value);
            case 8:
                return to_oct(value);
            case 10:
                return to_dec(value);
            case 16:
                return to_hex(value);
            default:
                return ERR("could not convert bit-vector to string: invalid value '" + std::to_string(base) + "' given for base");
        }
    }

    std::ostream& operator<<(std::ostream& os, BooleanFunction::Value v)
    {
        return os << BooleanFunction::to_string(v);
    }

    BooleanFunction::BooleanFunction()
    {
    }

    Result<BooleanFunction> BooleanFunction::build(std::vector<BooleanFunction::Node>&& nodes)
    {
        if (auto res = BooleanFunction::validate(BooleanFunction(std::move(nodes))); res.is_error())
        {
            return ERR_APPEND(res.get_error(), "could not build Boolean function from vector of nodes: failed to validate Boolean function");
        }
        else
        {
            return res;
        }
    }

    BooleanFunction BooleanFunction::Var(const std::string& name, u16 size)
    {
        return BooleanFunction(Node::Variable(name, size));
    }

    BooleanFunction BooleanFunction::Const(const BooleanFunction::Value& value)
    {
        return BooleanFunction(Node::Constant({value}));
    }

    BooleanFunction BooleanFunction::Const(const std::vector<BooleanFunction::Value>& values)
    {
        return BooleanFunction(Node::Constant(values));
    }

    BooleanFunction BooleanFunction::Const(u64 value, u16 size)
    {
        auto values = std::vector<BooleanFunction::Value>();
        values.reserve(size);
        for (auto i = 0; i < size; i++)
        {
            values.emplace_back((value & (1 << i)) ? BooleanFunction::Value::ONE : BooleanFunction::Value::ZERO);
        }

        return BooleanFunction::Const(values);
    }

    BooleanFunction BooleanFunction::Index(u16 index, u16 size)
    {
        return BooleanFunction(Node::Index(index, size));
    }

    Result<BooleanFunction> BooleanFunction::And(BooleanFunction&& p0, BooleanFunction&& p1, u16 size)
    {
        if ((p0.size() != p1.size()) || (p0.size() != size))
        {
            return ERR("could not join Boolean functions using AND operation: bit-sizes do not match (p0 = " + std::to_string(p0.size()) + ", p1 = " + std::to_string(p1.size())
                       + ", size = " + std::to_string(size) + ")");
        }

        return OK(BooleanFunction(Node::Operation(NodeType::And, size), std::move(p0), std::move(p1)));
    }

    Result<BooleanFunction> BooleanFunction::Or(BooleanFunction&& p0, BooleanFunction&& p1, u16 size)
    {
        if ((p0.size() != p1.size()) || (p0.size() != size))
        {
            return ERR("could not join Boolean functions using OR operation: bit-sizes do not match (p0 = " + std::to_string(p0.size()) + ", p1 = " + std::to_string(p1.size())
                       + ", size = " + std::to_string(size) + ").");
        }

        return OK(BooleanFunction(Node::Operation(NodeType::Or, size), std::move(p0), std::move(p1)));
    }

    Result<BooleanFunction> BooleanFunction::Not(BooleanFunction&& p0, u16 size)
    {
        if (p0.size() != size)
        {
            return ERR("could not invert Boolean function using NOT operation: bit-sizes do not match (p0 = " + std::to_string(p0.size()) + ", size = " + std::to_string(size) + ").");
        }

        return OK(BooleanFunction(Node::Operation(NodeType::Not, size), std::move(p0)));
    }

    Result<BooleanFunction> BooleanFunction::Xor(BooleanFunction&& p0, BooleanFunction&& p1, u16 size)
    {
        if ((p0.size() != p1.size()) || (p0.size() != size))
        {
            return ERR("could not join Boolean functions using XOR operation: bit-sizes do not match (p0 = " + std::to_string(p0.size()) + ", p1 = " + std::to_string(p1.size())
                       + ", size = " + std::to_string(size) + ").");
        }

        return OK(BooleanFunction(Node::Operation(NodeType::Xor, size), std::move(p0), std::move(p1)));
    }

    Result<BooleanFunction> BooleanFunction::Add(BooleanFunction&& p0, BooleanFunction&& p1, u16 size)
    {
        if ((p0.size() != p1.size()) || (p0.size() != size))
        {
            return ERR("could not join Boolean functions using ADD operation: bit-sizes do not match (p0 = " + std::to_string(p0.size()) + ", p1 = " + std::to_string(p1.size())
                       + ", size = " + std::to_string(size) + ").");
        }

        return OK(BooleanFunction(Node::Operation(NodeType::Add, size), std::move(p0), std::move(p1)));
    }

    Result<BooleanFunction> BooleanFunction::Sub(BooleanFunction&& p0, BooleanFunction&& p1, u16 size)
    {
        if ((p0.size() != p1.size()) || (p0.size() != size))
        {
            return ERR("could not join Boolean functions using SUB operation: bit-sizes do not match (p0 = " + std::to_string(p0.size()) + ", p1 = " + std::to_string(p1.size())
                       + ", size = " + std::to_string(size) + ").");
        }

        return OK(BooleanFunction(Node::Operation(NodeType::Sub, size), std::move(p0), std::move(p1)));
    }

    Result<BooleanFunction> BooleanFunction::Slice(BooleanFunction&& p0, BooleanFunction&& p1, BooleanFunction&& p2, u16 size)
    {
        if (!p1.is_index() || !p2.is_index())
        {
            return ERR("could not apply slice operation: function types do not match (p1 and p2 must be of type 'BooleanFunction::Index')");
        }
        if ((p0.size() != p1.size()) || (p1.size() != p2.size()))
        {
            return ERR("could not apply slice operation: bit-sizes do not match (p0 = " + std::to_string(p0.size()) + ", p1 = " + std::to_string(p1.size()) + ", p2 = " + std::to_string(p2.size())
                       + " - sizes must be equal)");
        }

        auto start = p1.get_index_value().get(), end = p2.get_index_value().get();

        if ((start > end) || (start >= p0.size()) || (end >= p0.size()) || (end - start + 1) != size)
        {
            return ERR("could not apply SLICE operation: bit-sizes do not match (p0 = " + std::to_string(p0.size()) + ", p1 = " + std::to_string(start) + ", p2 = " + std::to_string(end) + ")");
        }

        return OK(BooleanFunction(Node::Operation(NodeType::Slice, size), std::move(p0), std::move(p1), std::move(p2)));
    }

    Result<BooleanFunction> BooleanFunction::Concat(BooleanFunction&& p0, BooleanFunction&& p1, u16 size)
    {
        if ((p0.size() + p1.size()) != size)
        {
            return ERR("could not apply CONCAT operation: function input widths do not match (p0 = " + std::to_string(p0.size()) + "-bit, p1 = " + std::to_string(p1.size())
                       + "-bit, size = " + std::to_string(size) + ").");
        }

        return OK(BooleanFunction(Node::Operation(NodeType::Concat, size), std::move(p0), std::move(p1)));
    }

    Result<BooleanFunction> BooleanFunction::Zext(BooleanFunction&& p0, BooleanFunction&& p1, u16 size)
    {
        if (p0.size() > size || p1.size() != size)
        {
            return ERR("could not apply ZEXT operation: function input width does not match (p0 = " + std::to_string(p0.size()) + "-bit, p1 = " + std::to_string(p1.size())
                       + "-bit, size = " + std::to_string(size) + ").");
        }

        if (!p1.has_index_value(size))
        {
            return ERR("could not apply ZEXT operation: p1 does not encode size (p1 = " + p1.to_string() + ", size = " + std::to_string(size) + ").");
        }

        return OK(BooleanFunction(Node::Operation(NodeType::Zext, size), std::move(p0), std::move(p1)));
    }

    Result<BooleanFunction> BooleanFunction::Sext(BooleanFunction&& p0, BooleanFunction&& p1, u16 size)
    {
        if (p0.size() > size || p1.size() != size)
        {
            return ERR("could not apply SEXT operation: function input width does not match (p0 = " + std::to_string(p0.size()) + "-bit, p1 = " + std::to_string(p1.size())
                       + "-bit, size = " + std::to_string(size) + ").");
        }

        if (!p1.has_index_value(size))
        {
            return ERR("could not apply SEXT operation: p1 does not encode size (p1 = " + p1.to_string() + ", size = " + std::to_string(size) + ").");
        }

        return OK(BooleanFunction(Node::Operation(NodeType::Sext, size), std::move(p0), std::move(p1)));
    }

    Result<BooleanFunction> BooleanFunction::Eq(BooleanFunction&& p0, BooleanFunction&& p1, u16 size)
    {
        if (p0.size() != p1.size() || size != 1)
        {
            return ERR("could not apply EQ operation: function input width does not match (p0 = " + std::to_string(p0.size()) + "-bit, p1 = " + std::to_string(p1.size())
                       + "-bit, size = " + std::to_string(size) + ").");
        }

        return OK(BooleanFunction(Node::Operation(NodeType::Eq, size), std::move(p0), std::move(p1)));
    }

    Result<BooleanFunction> BooleanFunction::Sle(BooleanFunction&& p0, BooleanFunction&& p1, u16 size)
    {
        if (p0.size() != p1.size() || size != 1)
        {
            return ERR("could not apply SLE operation: function input width does not match (p0 = " + std::to_string(p0.size()) + "-bit, p1 = " + std::to_string(p1.size())
                       + "-bit, size = " + std::to_string(size) + ").");
        }

        return OK(BooleanFunction(Node::Operation(NodeType::Sle, size), std::move(p0), std::move(p1)));
    }

    Result<BooleanFunction> BooleanFunction::Slt(BooleanFunction&& p0, BooleanFunction&& p1, u16 size)
    {
        if (p0.size() != p1.size() || size != 1)
        {
            return ERR("could not apply SLT operation: function input width does not match (p0 = " + std::to_string(p0.size()) + "-bit, p1 = " + std::to_string(p1.size())
                       + "-bit, size = " + std::to_string(size) + ").");
        }

        return OK(BooleanFunction(Node::Operation(NodeType::Slt, size), std::move(p0), std::move(p1)));
    }

    Result<BooleanFunction> BooleanFunction::Ule(BooleanFunction&& p0, BooleanFunction&& p1, u16 size)
    {
        if (p0.size() != p1.size() || size != 1)
        {
            return ERR("could not apply ULE operation: function input width does not match (p0 = " + std::to_string(p0.size()) + "-bit, p1 = " + std::to_string(p1.size())
                       + "-bit, size = " + std::to_string(size) + ").");
        }

        return OK(BooleanFunction(Node::Operation(NodeType::Ule, size), std::move(p0), std::move(p1)));
    }

    Result<BooleanFunction> BooleanFunction::Ult(BooleanFunction&& p0, BooleanFunction&& p1, u16 size)
    {
        if (p0.size() != p1.size() || size != 1)
        {
            return ERR("could not apply ULT operation: function input width does not match (p0 = " + std::to_string(p0.size()) + "-bit, p1 = " + std::to_string(p1.size())
                       + "-bit, size = " + std::to_string(size) + ").");
        }

        return OK(BooleanFunction(Node::Operation(NodeType::Ult, size), std::move(p0), std::move(p1)));
    }

    Result<BooleanFunction> BooleanFunction::Ite(BooleanFunction&& p0, BooleanFunction&& p1, BooleanFunction&& p2, u16 size)
    {
        if (p0.size() != 1 || p1.size() != size || p2.size() != size)
        {
            return ERR("could not apply ITE operation: function input width does not match (p0 = " + std::to_string(p0.size()) + "-bit, p1 = " + std::to_string(p1.size())
                       + "-bit, p2 = " + std::to_string(p2.size()) + "-bit, size = " + std::to_string(size) + ").");
        }

        return OK(BooleanFunction(Node::Operation(NodeType::Ite, size), std::move(p0), std::move(p1), std::move(p2)));
    }

    std::ostream& operator<<(std::ostream& os, const BooleanFunction& function)
    {
        return os << function.to_string();
    }

    BooleanFunction BooleanFunction::operator&(const BooleanFunction& other) const
    {
        return BooleanFunction::And(this->clone(), other.clone(), this->size()).get();
    }

    BooleanFunction& BooleanFunction::operator&=(const BooleanFunction& other)
    {
        *this = BooleanFunction::And(this->clone(), other.clone(), this->size()).get();
        return *this;
    }

    BooleanFunction BooleanFunction::operator~() const
    {
        return BooleanFunction::Not(this->clone(), this->size()).get();
    }

    BooleanFunction BooleanFunction::operator|(const BooleanFunction& other) const
    {
        return BooleanFunction::Or(this->clone(), other.clone(), this->size()).get();
    }

    BooleanFunction& BooleanFunction::operator|=(const BooleanFunction& other)
    {
        *this = BooleanFunction::Or(this->clone(), other.clone(), this->size()).get();
        return *this;
    }

    BooleanFunction BooleanFunction::operator^(const BooleanFunction& other) const
    {
        return BooleanFunction::Xor(this->clone(), other.clone(), this->size()).get();
    }

    BooleanFunction& BooleanFunction::operator^=(const BooleanFunction& other)
    {
        *this = BooleanFunction::Xor(this->clone(), other.clone(), this->size()).get();
        return *this;
    }

    BooleanFunction BooleanFunction::operator+(const BooleanFunction& other) const
    {
        return BooleanFunction::Add(this->clone(), other.clone(), this->size()).get();
    }

    BooleanFunction& BooleanFunction::operator+=(const BooleanFunction& other)
    {
        *this = BooleanFunction::Add(this->clone(), other.clone(), this->size()).get();
        return *this;
    }

    BooleanFunction BooleanFunction::operator-(const BooleanFunction& other) const
    {
        return BooleanFunction::Sub(this->clone(), other.clone(), this->size()).get();
    }

    BooleanFunction& BooleanFunction::operator-=(const BooleanFunction& other)
    {
        *this = BooleanFunction::Sub(this->clone(), other.clone(), this->size()).get();
        return *this;
    }

    bool BooleanFunction::operator==(const BooleanFunction& other) const
    {
        if (this->m_nodes.size() != other.m_nodes.size())
        {
            return false;
        }

        for (auto i = 0ul; i < this->m_nodes.size(); i++)
        {
            if (this->m_nodes[i] != other.m_nodes[i])
            {
                return false;
            }
        }
        return true;
    }

    bool BooleanFunction::operator!=(const BooleanFunction& other) const
    {
        return !(*this == other);
    }

    bool BooleanFunction::operator<(const BooleanFunction& other) const
    {
        if (this->m_nodes.size() < other.m_nodes.size())
        {
            return true;
        }
        if (this->m_nodes.size() > other.m_nodes.size())
        {
            return false;
        }

        return this->to_string_in_reverse_polish_notation() < other.to_string_in_reverse_polish_notation();
    }

    bool BooleanFunction::is_empty() const
    {
        return this->m_nodes.empty();
    }

    BooleanFunction BooleanFunction::clone() const
    {
        auto function = BooleanFunction();
        function.m_nodes.reserve(this->m_nodes.size());

        for (const auto& node : this->m_nodes)
        {
            function.m_nodes.emplace_back(node);
        }

        return function;
    }

    u16 BooleanFunction::size() const
    {
        return this->m_nodes.back().size;
    }

    bool BooleanFunction::is(u16 type) const
    {
        return (this->is_empty()) ? false : this->get_top_level_node().is(type);
    }

    bool BooleanFunction::is_variable() const
    {
        return (this->is_empty()) ? false : this->get_top_level_node().is_variable();
    }

    bool BooleanFunction::is_constant() const
    {
        return (this->is_empty()) ? false : this->get_top_level_node().is_constant();
    }

    bool BooleanFunction::has_constant_value(u64 value) const
    {
        return (this->is_empty()) ? false : this->get_top_level_node().has_constant_value(value);
    }

    Result<u64> BooleanFunction::get_constant_value() const
    {
        if (!this->is_constant())
        {
            return ERR("Boolean function is not a constant");
        }

        if (this->size() > 64)
        {
            return ERR("Boolean function constant has size > 64");
        }

        if (std::any_of(this->m_nodes[0].constant.begin(), this->m_nodes[0].constant.end(), [](auto v) { return v != BooleanFunction::Value::ONE && v != BooleanFunction::Value::ZERO; }))
        {
            return ERR("Boolean function constant is undefined or high-impedance");
        }

        u64 val = 0;
        for (auto it = this->m_nodes[0].constant.rbegin(); it != this->m_nodes[0].constant.rend(); it++)
        {
            val <<= 1;
            val |= *it;
        }

        return OK(val);
    }

    bool BooleanFunction::is_index() const
    {
        return (this->is_empty()) ? false : this->get_top_level_node().is_index();
    }

    bool BooleanFunction::has_index_value(u16 value) const
    {
        return (this->is_empty()) ? false : this->get_top_level_node().has_index_value(value);
    }

    Result<u16> BooleanFunction::get_index_value() const
    {
        if (!this->is_index())
        {
            return ERR("Boolean function is not an index");
        }

        return OK(this->m_nodes[0].index);
    }

    const BooleanFunction::Node& BooleanFunction::get_top_level_node() const
    {
        return this->m_nodes.back();
    }

    u32 BooleanFunction::length() const
    {
        return this->m_nodes.size();
    }

    const std::vector<BooleanFunction::Node>& BooleanFunction::get_nodes() const
    {
        return this->m_nodes;
    }

    std::vector<BooleanFunction> BooleanFunction::get_parameters() const
    {
        /// # Developer Note
        /// Instead of iterating the whole Boolean function and decomposing the
        /// abstract syntax tree, we simple iterate the Boolean function once
        /// and compute the coverage, i.e. how many nodes are covered below the
        /// node in the tree, and based on these indices assemble the Boolean
        /// function node vector.

        auto coverage = this->compute_node_coverage();
        switch (this->get_top_level_node().get_arity())
        {
            case 0: {
                return {};
            }
            case 1: {
                return {BooleanFunction(std::vector<Node>({this->m_nodes.begin(), this->m_nodes.end() - 1}))};
            }
            case 2: {
                auto index = this->length() - coverage[this->length() - 2] - 1;

                return {BooleanFunction(std::vector<Node>({this->m_nodes.begin(), this->m_nodes.begin() + index})),
                        BooleanFunction(std::vector<Node>({this->m_nodes.begin() + index, this->m_nodes.end() - 1}))};
            }
            case 3: {
                auto index0 = this->length() - coverage[this->length() - 3] - coverage[this->length() - 2] - 1;
                auto index1 = this->length() - coverage[this->length() - 2] - 1;

                return {BooleanFunction(std::vector<Node>({this->m_nodes.begin(), this->m_nodes.begin() + index0})),
                        BooleanFunction(std::vector<Node>({this->m_nodes.begin() + index0, this->m_nodes.begin() + index1})),
                        BooleanFunction(std::vector<Node>({this->m_nodes.begin() + index1, this->m_nodes.end() - 1}))};
            }

            default:
                assert(false && "not implemented reached.");
        }

        return {};
    }

    std::set<std::string> BooleanFunction::get_variable_names() const
    {
        auto variable_names = std::set<std::string>();
        for (const auto& node : this->m_nodes)
        {
            if (node.is_variable())
            {
                variable_names.insert(node.variable);
            }
        }
        return variable_names;
    }

    Result<std::string> BooleanFunction::default_printer(const BooleanFunction::Node& node, std::vector<std::string>&& operands)
    {
        if (node.get_arity() != operands.size())
        {
            return ERR("could not print Boolean function: node arity of " + std::to_string(node.get_arity()) + " does not match number of operands of " + std::to_string(operands.size()));
        }

        switch (node.type)
        {
            case BooleanFunction::NodeType::Constant:
            case BooleanFunction::NodeType::Index:
            case BooleanFunction::NodeType::Variable:
                return OK(node.to_string());

            case BooleanFunction::NodeType::And:
                return OK("(" + operands[0] + " & " + operands[1] + ")");
            case BooleanFunction::NodeType::Not:
                return OK("(! " + operands[0] + ")");
            case BooleanFunction::NodeType::Or:
                return OK("(" + operands[0] + " | " + operands[1] + ")");
            case BooleanFunction::NodeType::Xor:
                return OK("(" + operands[0] + " ^ " + operands[1] + ")");

            case BooleanFunction::NodeType::Add:
                return OK("(" + operands[0] + " + " + operands[1] + ")");
            case BooleanFunction::NodeType::Sub:
                return OK("(" + operands[0] + " - " + operands[1] + ")");

            case BooleanFunction::NodeType::Concat:
                return OK("(" + operands[0] + " ++ " + operands[1] + ")");
            case BooleanFunction::NodeType::Slice:
                return OK("Slice(" + operands[0] + ", " + operands[1] + ", " + operands[2] + ")");
            case BooleanFunction::NodeType::Zext:
                return OK("Zext(" + operands[0] + ", " + operands[1] + ")");
            case BooleanFunction::NodeType::Sext:
                return OK("Sext(" + operands[0] + ", " + operands[1] + ")");

            case BooleanFunction::NodeType::Eq:
                return OK("(" + operands[0] + " == " + operands[1] + ")");
            case BooleanFunction::NodeType::Slt:
                return OK("(" + operands[0] + " <s " + operands[1] + ")");
            case BooleanFunction::NodeType::Sle:
                return OK("(" + operands[0] + " <=s " + operands[1] + ")");
            case BooleanFunction::NodeType::Ult:
                return OK("(" + operands[0] + " < " + operands[1] + ")");
            case BooleanFunction::NodeType::Ule:
                return OK("(" + operands[0] + " <= " + operands[1] + ")");
            case BooleanFunction::NodeType::Ite:
                return OK("Ite(" + operands[0] + ", " + operands[1] + ", " + operands[2] + ")");

            default:
                return ERR("could not print Boolean function: unsupported node type '" + std::to_string(node.type) + "'");
        }
    }

    std::string BooleanFunction::to_string(std::function<Result<std::string>(const BooleanFunction::Node& node, std::vector<std::string>&& operands)>&& printer) const
    {
        // (1) early termination in case the Boolean function is empty
        if (this->m_nodes.empty())
        {
            return "<empty>";
        }

        // (2) iterate the list of nodes and setup string from leafs to root
        std::vector<std::string> stack;
        for (const auto& node : this->m_nodes)
        {
            std::vector<std::string> operands;

            if (stack.size() < node.get_arity())
            {
                // log_error("netlist", "Cannot fetch {} nodes from the stack (= imbalanced stack with {} parts - {}).", node->get_arity(), stack.size(), this->to_string_in_reverse_polish_notation());
                return "";
            }

            std::move(stack.end() - static_cast<u64>(node.get_arity()), stack.end(), std::back_inserter(operands));
            stack.erase(stack.end() - static_cast<u64>(node.get_arity()), stack.end());

            if (auto res = printer(node, std::move(operands)); res.is_ok())
            {
                stack.emplace_back(res.get());
            }
            else
            {
                log_error("netlist", "Cannot translate BooleanFunction::Node '{}' to a string: {}.", node.to_string(), res.get_error().get());
                return "";
            }
        }

        switch (stack.size())
        {
            case 1:
                return stack.back();
            default: {
                // log_error("netlist", "Cannot translate BooleanFunction (= imbalanced stack with {} remaining parts).", stack.size());
                return "";
            }
        }
    }

    Result<BooleanFunction> BooleanFunction::from_string(const std::string& expression)
    {
        using BooleanFunctionParser::ParserType;
        using BooleanFunctionParser::Token;

        static const std::vector<std::tuple<ParserType, std::function<Result<std::vector<Token>>(const std::string&)>>> parsers = {
            {ParserType::Standard, BooleanFunctionParser::parse_with_standard_grammar},
            {ParserType::Liberty, BooleanFunctionParser::parse_with_liberty_grammar},
        };

        for (const auto& [parser_type, parser] : parsers)
        {
            auto tokens = parser(expression);
            // (1) skip if parser cannot translate to tokens
            if (tokens.is_error())
            {
                continue;
            }

            // (2) skip if cannot translate to valid reverse-polish notation
            tokens = BooleanFunctionParser::reverse_polish_notation(tokens.get(), expression, parser_type);
            if (tokens.is_error())
            {
                continue;
            }
            // (3) skip if reverse-polish notation tokens are no valid Boolean function
            auto function = BooleanFunctionParser::translate(tokens.get(), expression);
            if (function.is_error())
            {
                continue;
            }
            return function;
        }
        return ERR("could not parse Boolean function from string: no parser available for '" + expression + "'");
    }

    BooleanFunction BooleanFunction::simplify() const
    {
        auto simplified = Simplification::local_simplification(*this).map<BooleanFunction>([](const auto& s) { return Simplification::abc_simplification(s); }).map<BooleanFunction>([](const auto& s) {
            return Simplification::local_simplification(s);
        });

        return (simplified.is_ok()) ? simplified.get() : this->clone();
    }

    BooleanFunction BooleanFunction::substitute(const std::string& old_variable_name, const std::string& new_variable_name) const
    {
        auto function = this->clone();
        for (auto i = 0u; i < this->m_nodes.size(); i++)
        {
            if (this->m_nodes[i].has_variable_name(old_variable_name))
            {
                function.m_nodes[i] = Node::Variable(new_variable_name, this->m_nodes[i].size);
            }
        }

        return function;
    }

    Result<BooleanFunction> BooleanFunction::substitute(const std::string& name, const BooleanFunction& replacement) const
    {
        /// Helper function to substitute a variable with a Boolean function.
        ///
        /// @param[in] node - Node.
        /// @param[in] operands - Operands of node.
        /// @param[in] var_name - Variable name to check for replacement.
        /// @param[in] repl - Replacement Boolean function.
        /// @returns AST replacement.
        auto substitute_variable = [](const auto& node, auto&& operands, auto var_name, auto repl) -> BooleanFunction {
            if (node.has_variable_name(var_name))
            {
                return repl.clone();
            }
            return BooleanFunction(node.clone(), std::move(operands));
        };

        std::vector<BooleanFunction> stack;
        for (const auto& node : this->m_nodes)
        {
            std::vector<BooleanFunction> operands;
            std::move(stack.end() - static_cast<i64>(node.get_arity()), stack.end(), std::back_inserter(operands));
            stack.erase(stack.end() - static_cast<i64>(node.get_arity()), stack.end());

            stack.emplace_back(substitute_variable(node, std::move(operands), name, replacement));
        }

        switch (stack.size())
        {
            case 1:
                return OK(stack.back());
            default:
                return ERR("could not replace variable '" + name + "' with Boolean function '" + replacement.to_string() + "': validation failed, the operations may be imbalanced");
        }
    }

    Result<BooleanFunction::Value> BooleanFunction::evaluate(const std::unordered_map<std::string, Value>& inputs) const
    {
        // (0) workaround to preserve the API functionality
        if (this->m_nodes.empty())
        {
            return OK(BooleanFunction::Value::X);
        }

        // (1) validate whether the input sizes match the boolean function
        if (this->size() != 1)
        {
            return ERR("could not evaluate Boolean function '" + this->to_string() + "': using single-bit evaluation on a Boolean function of size " + std::to_string(this->size()) + " is illegal");
        }

        // (2) translate the input to n-bit to use the generic function
        auto generic_inputs = std::unordered_map<std::string, std::vector<Value>>();
        for (const auto& [name, value] : inputs)
        {
            generic_inputs.emplace(name, std::vector<Value>({value}));
        }

        auto value = this->evaluate(generic_inputs);
        if (value.is_ok())
        {
            return OK(value.get()[0]);
        }

        return ERR(value.get_error());
    }

    Result<std::vector<BooleanFunction::Value>> BooleanFunction::evaluate(const std::unordered_map<std::string, std::vector<Value>>& inputs) const
    {
        // (0) workaround to preserve the API functionality
        if (this->m_nodes.empty())
        {
            return OK(std::vector<BooleanFunction::Value>({BooleanFunction::Value::X}));
        }

        // (1) validate whether the input sizes match the boolean function
        for (const auto& [name, value] : inputs)
        {
            for (const auto& node : this->m_nodes)
            {
                if (node.has_variable_name(name) && node.size != value.size())
                {
                    return ERR("could not evaluate Boolean function '" + this->to_string() + "': as the number of variables (" + std::to_string(node.size)
                               + ") does not match the number of provided inputs (" + std::to_string(value.size()) + ")");
                }
            }
        }

        // (2) initialize the symbolic state using the input variables
        auto symbolic_execution = SMT::SymbolicExecution();
        for (const auto& [name, value] : inputs)
        {
            symbolic_execution.state.set(BooleanFunction::Var(name, value.size()), BooleanFunction::Const(value));
        }

        // (3) analyze the evaluation result and check whether the result is a
        //     constant boolean function
        auto result = symbolic_execution.evaluate(*this);
        if (result.is_ok())
        {
            if (auto value = result.get(); value.is_constant())
            {
                return OK(value.get_top_level_node().constant);
            }
            return OK(std::vector<BooleanFunction::Value>(this->size(), BooleanFunction::Value::X));
        }
        return ERR(result.get_error());
    }

    Result<std::vector<std::vector<BooleanFunction::Value>>> BooleanFunction::compute_truth_table(const std::vector<std::string>& ordered_variables, bool remove_unknown_variables) const
    {
        auto variable_names_in_function = this->get_variable_names();

        // (1) check that each variable is just a single bit, otherwise we do
        //     not generate a truth-table
        for (const auto& node : this->m_nodes)
        {
            if (node.is_variable() && node.size != 1)
            {
                return ERR("could not compute truth table for Boolean function '" + this->to_string() + "': unable to generate a truth-table for Boolean function with variables of > 1-bit");
            }
        }

        // (2) select either parameter or the Boolean function variables
        auto variables = ordered_variables;
        if (variables.empty())
        {
            variables = std::vector<std::string>(variable_names_in_function.begin(), variable_names_in_function.end());
        }

        // (3) remove any unknown variables from the truth table
        if (remove_unknown_variables)
        {
            variables.erase(
                std::remove_if(variables.begin(), variables.end(), [&variable_names_in_function](const auto& s) { return variable_names_in_function.find(s) == variable_names_in_function.end(); }),
                variables.end());
        }

        // (4.1) check that the function is not empty, otherwise we return a
        //       Boolean function with a truth-table with 'X' values
        if (this->m_nodes.empty())
        {
            return OK(std::vector<std::vector<Value>>(1, std::vector<Value>(1 << variables.size(), Value::X)));
        }

        // (4.2) safety-check in case the number of variables is too large to process
        if (variables.size() > 10)
        {
            return ERR("could not compute truth table for Boolean function '" + this->to_string() + "': unable to generate truth-table with more than 10 variables");
        }

        std::vector<std::vector<Value>> truth_table(this->size(), std::vector<Value>(1 << variables.size(), Value::ZERO));

        // (5) iterate the truth-table rows and set each column accordingly
        for (auto value = 0u; value < ((u32)1 << variables.size()); value++)
        {
            std::unordered_map<std::string, std::vector<Value>> input;
            auto tmp = value;
            for (const auto& variable : variables)
            {
                input[variable] = ((tmp & 1) == 0) ? std::vector<Value>({Value::ZERO}) : std::vector<Value>({Value::ONE});
                tmp >>= 1;
            }
            auto result = this->evaluate(input);
            if (result.is_error())
            {
                return ERR(result.get_error());
            }
            auto output = result.get();
            for (auto index = 0u; index < truth_table.size(); index++)
            {
                truth_table[index][value] = output[index];
            }
        }

        return OK(truth_table);
    }

    z3::expr BooleanFunction::to_z3(z3::context& context, const std::map<std::string, z3::expr>& var2expr) const
    {
        /// Helper function to reduce a abstract syntax subtree to z3 expressions
        ///
        /// @param[in] node - Boolean function node.
        /// @param[in] p - Boolean function node parameters.
        /// @returns (1) status (true on success, false otherwise),
        ///          (2) SMT-LIB string representation of node and operands.
        auto reduce_to_z3 = [&context, &var2expr](const auto& node, auto&& p) -> std::tuple<bool, z3::expr> {
            if (node.get_arity() != p.size())
            {
                return {false, z3::expr(context)};
            }

            switch (node.type)
            {
                case BooleanFunction::NodeType::Index:
                    return {true, context.bv_val(node.index, node.size)};
                case BooleanFunction::NodeType::Constant: {
                    // since our constants are defined as arbitrary bit-vectors,
                    // we have to concat each bit just to be on the safe side
                    auto constant = context.bv_val(node.constant.front(), 1);
                    for (u32 i = 1; i < node.constant.size(); i++)
                    {
                        const auto bit = node.constant.at(i);
                        constant       = z3::concat(context.bv_val(bit, 1), constant);
                    }
                    return {true, constant};
                }
                case BooleanFunction::NodeType::Variable: {
                    if (auto it = var2expr.find(node.variable); it != var2expr.end())
                    {
                        return {true, it->second};
                    }
                    return {true, context.bv_const(node.variable.c_str(), node.size)};
                }

                case BooleanFunction::NodeType::And:
                    return {true, p[0] & p[1]};
                case BooleanFunction::NodeType::Or:
                    return {true, p[0] | p[1]};
                case BooleanFunction::NodeType::Not:
                    return {true, ~p[0]};
                case BooleanFunction::NodeType::Xor:
                    return {true, p[0] ^ p[1]};
                case BooleanFunction::NodeType::Slice:
                    return {true, p[0].extract( p[2].get_numeral_uint(), p[1].get_numeral_uint())};
                case BooleanFunction::NodeType::Concat:
                    return {true, z3::concat(p[0], p[1])};
                case BooleanFunction::NodeType::Sext:
                    return {true, z3::sext(p[0], p[1].get_numeral_uint())};

                default:
                    log_error("netlist", "Not implemented reached for nodetype {} in z3 conversion", node.type);
                    return {false, z3::expr(context)};
            }
        };

        std::vector<z3::expr> stack;
        for (const auto& node : this->m_nodes)
        {
            std::vector<z3::expr> operands;
            std::move(stack.end() - static_cast<i64>(node.get_arity()), stack.end(), std::back_inserter(operands));
            stack.erase(stack.end() - static_cast<i64>(node.get_arity()), stack.end());

            if (auto [ok, reduction] = reduce_to_z3(node, std::move(operands)); ok)
            {
                stack.emplace_back(reduction);
            }
            else
            {
                return z3::expr(context);
            }
        }

        switch (stack.size())
        {
            case 1:
                return stack.back();
            default:
                return z3::expr(context);
        }
    }

    BooleanFunction::BooleanFunction(std::vector<BooleanFunction::Node>&& nodes) : m_nodes(nodes)
    {
    }

    BooleanFunction::BooleanFunction(BooleanFunction::Node&& node, std::vector<BooleanFunction>&& p) : BooleanFunction()
    {
        auto size = 1;
        for (const auto& parameter : p)
        {
            size += parameter.size();
        }
        this->m_nodes.reserve(size);

        for (auto&& parameter : p)
        {
            this->m_nodes.insert(this->m_nodes.end(), parameter.m_nodes.begin(), parameter.m_nodes.end());
        }
        this->m_nodes.emplace_back(node);
    }

    std::string BooleanFunction::to_string_in_reverse_polish_notation() const
    {
        std::string s;
        for (const auto& node : this->m_nodes)
        {
            s += node.to_string() + " ";
        }
        return s;
    }

    Result<BooleanFunction> BooleanFunction::validate(BooleanFunction&& function)
    {
        /// # Note
        /// In order to validate correctness of a Boolean function, we analyze
        /// the arity of each node and whether its value matches the number of
        /// parameters and covered nodes in the abstract syntax tree.
        if (auto coverage = function.compute_node_coverage(); coverage.back() != function.length())
        {
            auto str = function.to_string_in_reverse_polish_notation();
            return ERR("could not validate '" + str + "': imbalanced function with coverage '" + std::to_string(coverage.back()) + " != " + std::to_string(function.length()));
        }

        return OK(std::move(function));
    }

    std::vector<u32> BooleanFunction::compute_node_coverage() const
    {
        auto coverage = std::vector<u32>(this->m_nodes.size(), (u32)-1);

        /// Short-hand function to safely access a coverage value from a vector
        ///
        /// @param[in] cov - Coverage vector.
        /// @param[in] index - Index into coverage vector.
        /// @returns Value at coverage[index] or -1 in case index is invalid.
        auto get = [](const auto& cov, size_t index) -> u32 { return (index < cov.size()) ? cov[index] : -1; };

        /// Short-hand function to safely set a coverage value at an index.
        ///
        /// @param[in,out] cov - Coverage vector to be modified.
        /// @param[in] index - Valid index into coverage vector.
        /// @param[in] x - 1st part of coverage value (= 0 on default)
        /// @param[in] y - 2nc part of coverage value (= 0 on default)
        /// @param[in] z - 3rd part of coverage value (= 0 on default)
        auto set = [](auto& cov, size_t index, u32 x = 0, u32 y = 0, u32 z = 0) { cov[index] = ((x != (u32)-1) && (y != (u32)-1) && (z != (u32)-1)) ? (x + y + z + 1) : (u32)-1; };

        for (auto i = 0ul; i < this->m_nodes.size(); i++)
        {
            auto arity = this->m_nodes[i].get_arity();

            switch (arity)
            {
                case 0: {
                    set(coverage, i);
                    break;
                }
                case 1: {
                    auto x = get(coverage, i - 1);
                    set(coverage, i, x);
                    break;
                }
                case 2: {
                    auto x = get(coverage, i - 1);
                    auto y = get(coverage, i - 1 - x);
                    set(coverage, i, x, y);
                    break;
                }
                case 3: {
                    auto x = get(coverage, i - 1);
                    auto y = get(coverage, i - 1 - x);
                    auto z = get(coverage, i - 1 - x - y);
                    set(coverage, i, x, y, z);
                    break;
                }
            }
        }

        return coverage;
    }

    BooleanFunction::Node BooleanFunction::Node::Operation(u16 _type, u16 _size)
    {
        return Node(_type, _size, {}, {}, {});
    }

    BooleanFunction::Node BooleanFunction::Node::Constant(const std::vector<BooleanFunction::Value> _constant)
    {
        return Node(NodeType::Constant, _constant.size(), _constant, {}, {});
    }

    BooleanFunction::Node BooleanFunction::Node::Index(u16 _index, u16 _size)
    {
        return Node(NodeType::Index, _size, {}, _index, {});
    }

    BooleanFunction::Node BooleanFunction::Node::Variable(const std::string _variable, u16 _size)
    {
        return Node(NodeType::Variable, _size, {}, {}, _variable);
    }

    bool BooleanFunction::Node::operator==(const Node& other) const
    {
        return std::tie(this->type, this->size, this->constant, this->index, this->variable) == std::tie(other.type, other.size, other.constant, other.index, other.variable);
    }

    bool BooleanFunction::Node::operator!=(const Node& other) const
    {
        return !(*this == other);
    }

    bool BooleanFunction::Node::operator<(const Node& other) const
    {
        return std::tie(this->type, this->size, this->constant, this->index, this->variable) < std::tie(other.type, other.size, other.constant, other.index, other.variable);
    }

    BooleanFunction::Node BooleanFunction::Node::clone() const
    {
        return Node(this->type, this->size, this->constant, this->index, this->variable);
    }

    std::string BooleanFunction::Node::to_string() const
    {
        switch (this->type)
        {
            case NodeType::Constant: {
                std::string str;
                for (const auto& value : this->constant)
                {
                    str = enum_to_string(value) + str;
                }
                return "0b" + str;
            }

            case NodeType::Index:
                return std::to_string(this->index);
            case NodeType::Variable:
                return this->variable;

            case NodeType::And:
                return "&";
            case NodeType::Or:
                return "|";
            case NodeType::Not:
                return "~";
            case NodeType::Xor:
                return "^";

            case NodeType::Add:
                return "+";
            case NodeType::Sub:
                return "-";

            case NodeType::Concat:
                return "++";
            case NodeType::Slice:
                return "Slice";
            case NodeType::Zext:
                return "Zext";
            case NodeType::Sext:
                return "Sext";

            case NodeType::Eq:
                return "==";
            case NodeType::Sle:
                return "<=s";
            case NodeType::Slt:
                return "<s";
            case NodeType::Ule:
                return "<=";
            case NodeType::Ult:
                return "<";
            case NodeType::Ite:
                return "Ite";

            default:
                return "unsupported node type '" + std::to_string(this->type) + "'.";
        }
    }

    u16 BooleanFunction::Node::get_arity() const
    {
        return BooleanFunction::Node::get_arity_of_type(this->type);
    }

    u16 BooleanFunction::Node::get_arity_of_type(u16 type)
    {
        static const std::map<u16, u16> type2arity = {
            {BooleanFunction::NodeType::And, 2},      {BooleanFunction::NodeType::Or, 2},    {BooleanFunction::NodeType::Not, 1},      {BooleanFunction::NodeType::Xor, 2},

            {BooleanFunction::NodeType::Add, 2},      {BooleanFunction::NodeType::Sub, 2},

            {BooleanFunction::NodeType::Concat, 2},   {BooleanFunction::NodeType::Slice, 3}, {BooleanFunction::NodeType::Zext, 2},     {BooleanFunction::NodeType::Sext, 2},

            {BooleanFunction::NodeType::Eq, 2},       {BooleanFunction::NodeType::Sle, 2},   {BooleanFunction::NodeType::Slt, 2},      {BooleanFunction::NodeType::Ule, 2},
            {BooleanFunction::NodeType::Ult, 2},      {BooleanFunction::NodeType::Ite, 3},

            {BooleanFunction::NodeType::Constant, 0}, {BooleanFunction::NodeType::Index, 0}, {BooleanFunction::NodeType::Variable, 0},
        };

        return type2arity.at(type);
    }

    bool BooleanFunction::Node::is(u16 _type) const
    {
        return this->type == _type;
    }

    bool BooleanFunction::Node::is_constant() const
    {
        return this->is(BooleanFunction::NodeType::Constant);
    }

    bool BooleanFunction::Node::has_constant_value(u64 value) const
    {
        if (!this->is_constant())
        {
            return false;
        }

        auto bv_value = std::vector<BooleanFunction::Value>({});
        bv_value.reserve(this->size);
        for (auto i = 0u; i < this->constant.size(); i++)
        {
            bv_value.emplace_back((value & (1 << i)) ? BooleanFunction::Value::ONE : BooleanFunction::Value::ZERO);
        }
        return this->constant == bv_value;
    }

    bool BooleanFunction::Node::is_index() const
    {
        return this->is(BooleanFunction::NodeType::Index);
    }

    bool BooleanFunction::Node::has_index_value(u16 value) const
    {
        return this->is_index() && (this->index == value);
    }

    bool BooleanFunction::Node::is_variable() const
    {
        return this->is(BooleanFunction::NodeType::Variable);
    }

    bool BooleanFunction::Node::has_variable_name(const std::string& value) const
    {
        return this->is_variable() && (this->variable == value);
    }

    bool BooleanFunction::Node::is_operation() const
    {
        return !this->is_operand();
    }

    bool BooleanFunction::Node::is_operand() const
    {
        return this->is_constant() || this->is_variable() || this->is_index();
    }

    bool BooleanFunction::Node::is_commutative() const
    {
        return (this->type == NodeType::And) || (this->type == NodeType::Or) || (this->type == NodeType::Xor) || (this->type == NodeType::Add) || (this->type == NodeType::Eq);
    }

    BooleanFunction::Node::Node(u16 _type, u16 _size, std::vector<BooleanFunction::Value> _constant, u16 _index, std::string _variable)
        : type(_type), size(_size), constant(_constant), index(_index), variable(_variable)
    {
    }

}    // namespace hal
