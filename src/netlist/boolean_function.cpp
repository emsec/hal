#include "hal_core/netlist/boolean_function.h"

#include "hal_core/utilities/utils.h"

#include <algorithm>
#include <bitset>
#include <map>

namespace hal
{
    template<>
    std::vector<std::string> EnumStrings<BooleanFunction::Value>::data = {"0", "1", "Z", "X"};

    std::string BooleanFunction::to_string(const operation& op)
    {
        switch (op)
        {
            case operation::AND:
                return "&";
            case operation::OR:
                return "|";
            case operation::XOR:
                return "^";
            default:
                return "?";
        }
    }

    std::ostream& operator<<(std::ostream& os, const BooleanFunction::operation& op)
    {
        return os << BooleanFunction::to_string(op);
    }

    std::string BooleanFunction::to_string(Value v)
    {
        return enum_to_string<Value>(v);
    }

    std::ostream& operator<<(std::ostream& os, BooleanFunction::Value v)
    {
        return os << BooleanFunction::to_string(v);
    }

    BooleanFunction::BooleanFunction()
    {
        m_content = content_type::TERMS;
        m_invert  = false;
    }

    BooleanFunction::BooleanFunction(operation op, const std::vector<BooleanFunction>& operands, bool invert_result) : BooleanFunction()
    {
        if (operands.empty())
        {
            m_content  = content_type::CONSTANT;
            m_constant = Value::X;
        }
        else if (operands.size() == 1)
        {
            *this = operands[0];
        }
        else
        {
            m_content = content_type::TERMS;
            m_invert  = invert_result;

            m_op = op;
            std::copy_if(operands.begin(), operands.end(), std::back_inserter(m_operands), [](auto op2) { return !op2.is_empty(); });
        }
    }

    BooleanFunction::BooleanFunction(const std::string& variable_name) : BooleanFunction()
    {
        m_content  = content_type::VARIABLE;
        m_variable = utils::trim(variable_name);
        assert(!m_variable.empty());
    }

    BooleanFunction::BooleanFunction(Value constant) : BooleanFunction()
    {
        m_content  = content_type::CONSTANT;
        m_constant = constant;
    }

    BooleanFunction::content_type BooleanFunction::get_type() const {
        return m_content;
    }

    BooleanFunction::operation BooleanFunction::get_operation() const {
        return m_op;
    }

    bool BooleanFunction::is_neg() const {
        return m_invert;
    }

    const std::vector<BooleanFunction>& BooleanFunction::get_operands() const {
        return m_operands;
    }

    BooleanFunction BooleanFunction::substitute(const std::string& old_variable_name, const std::string& new_variable_name) const
    {
        return substitute(old_variable_name, BooleanFunction(new_variable_name));
    }

    void BooleanFunction::substitute_helper(BooleanFunction& f, const std::string& v, const BooleanFunction& s)
    {
        if (f.m_content == content_type::VARIABLE && f.m_variable == v)
        {
            if (f.m_invert)
            {
                f = ~s;
            }
            else
            {
                f = s;
            }
        }
        else if (f.m_content == content_type::TERMS)
        {
            for (u32 i = 0; i < f.m_operands.size(); ++i)
            {
                if (f.m_operands[i].m_content == content_type::VARIABLE && f.m_operands[i].m_variable == v)
                {
                    if (f.m_operands[i].m_invert)
                    {
                        f.m_operands[i] = ~s;
                    }
                    else
                    {
                        f.m_operands[i] = s;
                    }
                }
                else if (f.m_operands[i].m_content == content_type::TERMS)
                {
                    substitute_helper(f.m_operands[i], v, s);
                }
            }
        }
    }

    BooleanFunction BooleanFunction::substitute(const std::string& variable_name, const BooleanFunction& function) const
    {
        auto result = *this;
        substitute_helper(result, variable_name, function);
        return result;
    }

    BooleanFunction::Value BooleanFunction::evaluate(const std::unordered_map<std::string, Value>& inputs) const
    {
        Value result = X;
        if (m_content == content_type::VARIABLE)
        {
            auto it = inputs.find(m_variable);
            if (it != inputs.end())
            {
                result = it->second;
            }
        }
        else if (m_content == content_type::CONSTANT)
        {
            result = m_constant;
        }
        else if (!m_operands.empty())
        {
            result = m_operands[0].evaluate(inputs);

            for (u32 i = 1; i < m_operands.size(); ++i)
            {
                // early exit
                if ((m_op == operation::AND && result == ZERO) || (m_op == operation::OR && result == ONE) || (m_op == operation::XOR && result == X))
                {
                    break;
                }

                auto next = m_operands[i].evaluate(inputs);
                if (m_op == operation::AND)
                {
                    if (next == ZERO || result == ZERO)
                    {
                        result = ZERO;
                    }
                    else if ((next == X || next == Z) && result == ONE)
                    {
                        result = X;
                    }
                }
                else if (m_op == operation::OR)
                {
                    if (next == ONE || result == ONE)
                    {
                        result = ONE;
                    }
                    else if ((next == X || next == Z) && result == ZERO)
                    {
                        result = X;
                    }
                }
                else if (m_op == operation::XOR)
                {
                    if (next == ONE)
                    {
                        result = (Value)(1 - result);
                    }
                    else if (next == X || next == Z)
                    {
                        result = X;
                    }
                }
            }
        }

        if (m_invert)
        {
            if (result == ONE)
            {
                return ZERO;
            }
            else if (result == ZERO)
            {
                return ONE;
            }
            else if (result == Z)
            {
                result = X;
            }
        }
        return result;
    }

    BooleanFunction::Value BooleanFunction::operator()(const std::unordered_map<std::string, BooleanFunction::Value>& inputs) const
    {
        return evaluate(inputs);
    }

    bool BooleanFunction::is_constant_one() const
    {
        if (m_content == content_type::CONSTANT)
        {
            return m_constant == ONE;
        }
        auto tmp = optimize();
        return tmp.m_content == content_type::CONSTANT && tmp.m_constant == ONE;
    }

    bool BooleanFunction::is_constant_zero() const
    {
        if (m_content == content_type::CONSTANT)
        {
            return m_constant == ZERO;
        }
        auto tmp = optimize();
        return tmp.m_content == content_type::CONSTANT && tmp.m_constant == ZERO;
    }

    bool BooleanFunction::is_empty() const
    {
        return m_content == content_type::TERMS && m_operands.empty();
    }

    std::vector<std::string> BooleanFunction::get_variables() const
    {
        if (m_content == content_type::VARIABLE)
        {
            return {m_variable};
        }
        else if (m_content == content_type::TERMS)
        {
            std::vector<std::string> result;
            for (const auto& f : m_operands)
            {
                auto tmp = f.get_variables();
                result.insert(result.end(), tmp.begin(), tmp.end());
            }
            std::sort(result.begin(), result.end());
            result.erase(std::unique(result.begin(), result.end()), result.end());
            return result;
        }
        return {};
    }

    BooleanFunction BooleanFunction::from_string(std::string expression, const std::vector<std::string>& variable_names)
    {
        auto sorted_variable_names = variable_names;
        std::sort(sorted_variable_names.begin(), sorted_variable_names.end(), [](const auto& a, const auto& b) { return a.size() > b.size(); });

        for (u32 i = 0; i < sorted_variable_names.size(); ++i)
        {
            auto pos = expression.find(sorted_variable_names[i]);
            while (pos != std::string::npos)
            {
                expression.replace(pos, sorted_variable_names[i].size(), "__v_" + std::to_string(i) + " ");
                pos = expression.find(sorted_variable_names[i], pos + sorted_variable_names[i].size());
            }
        }

        return from_string_internal(expression, sorted_variable_names);
    }

    BooleanFunction BooleanFunction::from_string_internal(std::string expression, const std::vector<std::string>& variable_names)
    {
        expression = utils::trim(expression);

        if (expression.empty())
        {
            return BooleanFunction();
        }

        const std::string delimiters = "!~^&|'+* ";

        // check for constants
        if (expression == "0")
        {
            return BooleanFunction(ZERO);
        }
        else if (expression == "1")
        {
            return BooleanFunction(ONE);
        }
        else if (expression == "X")
        {
            return BooleanFunction(X);
        }
        else if (expression == "Z")
        {
            return BooleanFunction(Z);
        }
        else
        {
            // check for variable
            bool is_term = false;
            for (const auto& d : delimiters + "()")
            {
                if (expression.find(d) != std::string::npos)
                {
                    is_term = true;
                    break;
                }
            }
            if (!is_term)
            {
                if (utils::starts_with(expression, std::string("__v_")))
                {
                    u32 idx    = std::stoul(expression.substr(4));
                    expression = variable_names[idx];
                }

                return BooleanFunction(expression);
            }
        }

        // simple bracket check
        i32 level = 0;
        for (const auto& c : expression)
        {
            if (c == '(')
            {
                level += 1;
            }
            else if (c == ')')
            {
                level -= 1;
                if (level < 0)
                {
                    return Value::X;
                }
            }
        }
        if (level != 0)
        {
            return Value::X;
        }

        // parse expression
        std::vector<std::string> terms;
        {
            std::string current_term;
            u32 bracket_level = 0;
            for (u32 i = 0; i < expression.size(); i++)
            {
                if (expression[i] == '(')
                {
                    // if this is a top-level term, store everything before the bracket
                    if (bracket_level == 0)
                    {
                        current_term = utils::trim(current_term);
                        if (!current_term.empty())
                        {
                            terms.push_back(current_term);
                            current_term.clear();
                        }
                    }

                    bracket_level++;
                }

                if (bracket_level == 0 && delimiters.find(expression[i]) != std::string::npos)
                {
                    // not in brackets and there was a delimiter -> save term and operation
                    current_term = utils::trim(current_term);
                    if (!current_term.empty())
                    {
                        terms.push_back(current_term);
                        current_term.clear();
                    }
                    if (expression[i] != ' ')
                    {
                        terms.push_back(std::string(1, expression[i]));
                    }
                }
                else
                {
                    // no special conditions -> char is term of current term
                    current_term += expression[i];
                }

                if (expression[i] == ')')
                {
                    bracket_level--;

                    if (bracket_level == 0)
                    {
                        // if we are back at top-level, store everything that was in the brackets
                        current_term = utils::trim(current_term);
                        if (!current_term.empty())
                        {
                            terms.push_back(current_term);
                            current_term.clear();
                        }
                    }
                }
            }
            current_term = utils::trim(current_term);
            if (!current_term.empty())
            {
                terms.push_back(current_term);
            }
        }

        // process terms
        if (terms.size() == 1)
        {
            // only a single term but not filtered before?
            // -> was of the form "(...)" so remove the outer brackets and repeat
            return from_string_internal(terms[0].substr(1, terms[0].size() - 2), variable_names);
        }

        // small mutable datastructure for parsing
        struct op_term
        {
            operation op;
            BooleanFunction term;
        };
        std::vector<op_term> parsed_terms;

        bool negate_next  = false;
        operation next_op = operation::AND;

        {
            // multiple terms available -> initialize return Value with first term
            u32 i = 0;
            while (terms[i] == "!" || terms[i] == "~")
            {
                negate_next = !negate_next;
                ++i;
            }
            BooleanFunction first_term = from_string_internal(terms[i], variable_names);
            while (i + 1 < terms.size() && terms[i + 1] == "'")
            {
                negate_next = !negate_next;
                ++i;
            }
            if (negate_next)
            {
                first_term  = ~first_term;
                negate_next = false;
            }

            parsed_terms.push_back({operation::AND, first_term});

            // process the remaining terms
            while (++i < terms.size())
            {
                if (terms[i] == "!" || terms[i] == "~")
                {
                    negate_next = !negate_next;
                }
                else if (terms[i] == "&" || terms[i] == "*")
                {
                    next_op = operation::AND;
                }
                else if (terms[i] == "|" || terms[i] == "+")
                {
                    next_op = operation::OR;
                }
                else if (terms[i] == "^")
                {
                    next_op = operation::XOR;
                }
                else
                {
                    auto next_term = from_string_internal(terms[i], variable_names);
                    while (i + 1 < terms.size() && terms[i + 1] == "'")
                    {
                        negate_next = !negate_next;
                        ++i;
                    }
                    if (negate_next)
                    {
                        next_term = ~next_term;
                    }

                    parsed_terms.push_back({next_op, next_term});

                    negate_next = false;
                    next_op     = operation::AND;
                }
            }
        }

        // assemble terms in order of operator precedence

        for (u32 i = 1; i < parsed_terms.size(); ++i)
        {
            if (parsed_terms[i].op == operation::AND)
            {
                parsed_terms[i - 1].term = parsed_terms[i - 1].term & parsed_terms[i].term;
                parsed_terms.erase(parsed_terms.begin() + i);
                --i;
            }
        }

        for (u32 i = 1; i < parsed_terms.size(); ++i)
        {
            if (parsed_terms[i].op == operation::XOR)
            {
                parsed_terms[i - 1].term = parsed_terms[i - 1].term ^ parsed_terms[i].term;
                parsed_terms.erase(parsed_terms.begin() + i);
                --i;
            }
        }

        for (u32 i = 1; i < parsed_terms.size(); ++i)
        {
            if (parsed_terms[i].op == operation::OR)
            {
                parsed_terms[i - 1].term = parsed_terms[i - 1].term | parsed_terms[i].term;
                parsed_terms.erase(parsed_terms.begin() + i);
                --i;
            }
        }

        return parsed_terms[0].term;
    }

    std::string BooleanFunction::to_string() const
    {
        if (is_empty())
        {
            return "<empty>";
        }
        auto s = to_string_internal();

        // remove outer parantheses
        if (s[0] == '(')
        {
            u32 lvl = 0;
            u32 i   = 0;
            for (; i < s.size(); ++i)
            {
                if (s[i] == '(')
                {
                    lvl++;
                }
                else if (s[i] == ')')
                {
                    lvl--;
                    if (lvl == 0)
                    {
                        break;
                    }
                }
            }
            if (i >= s.size() - 1)
            {
                s = s.substr(1, s.size() - 2);
            }
        }

        return s;
    }

    std::string BooleanFunction::to_string_internal() const
    {
        std::string result = to_string(Value::X);
        if (m_content == content_type::VARIABLE)
        {
            result = m_variable;
        }
        else if (m_content == content_type::CONSTANT)
        {
            result = to_string(m_constant);
        }
        else if (!m_operands.empty())
        {
            std::string op_str = " " + to_string(m_op) + " ";

            std::vector<std::string> terms;
            for (const auto& f : m_operands)
            {
                terms.push_back(f.to_string_internal());
            }

            result = "(" + utils::join(op_str, terms) + ")";
        }

        if (m_invert)
        {
            result = "!" + result;
        }
        return result;
    }

    std::ostream& operator<<(std::ostream& os, const BooleanFunction& f)
    {
        return os << f.to_string();
    }

    BooleanFunction BooleanFunction::combine(operation op, const BooleanFunction& other) const
    {
        if (is_empty())
        {
            return other;
        }
        else if (other.is_empty())
        {
            return *this;
        }
        else if (m_content == content_type::TERMS && other.m_content == content_type::TERMS && m_op == op && m_op == other.m_op && !m_invert && !other.m_invert)
        {
            auto joint_operands = m_operands;
            joint_operands.insert(joint_operands.end(), other.m_operands.begin(), other.m_operands.end());
            BooleanFunction result(op, joint_operands);
            return result;
        }
        else if (m_content == content_type::TERMS && m_op == op && !m_invert)
        {
            BooleanFunction result = *this;
            result.m_operands.push_back(other);
            return result;
        }
        else if (other.m_content == content_type::TERMS && other.m_op == op && !other.m_invert)
        {
            BooleanFunction result = other;
            result.m_operands.insert(result.m_operands.begin(), *this);
            return result;
        }
        return BooleanFunction(op, {*this, other});
    }

    BooleanFunction BooleanFunction::operator&(const BooleanFunction& other) const
    {
        return combine(operation::AND, other);
    }

    BooleanFunction BooleanFunction::operator|(const BooleanFunction& other) const
    {
        return combine(operation::OR, other);
    }

    BooleanFunction BooleanFunction::operator^(const BooleanFunction& other) const
    {
        return combine(operation::XOR, other);
    }

    BooleanFunction& BooleanFunction::operator&=(const BooleanFunction& other)
    {
        *this = combine(operation::AND, other);
        return *this;
    }
    BooleanFunction& BooleanFunction::operator|=(const BooleanFunction& other)
    {
        *this = combine(operation::OR, other);
        return *this;
    }
    BooleanFunction& BooleanFunction::operator^=(const BooleanFunction& other)
    {
        *this = combine(operation::XOR, other);
        return *this;
    }

    BooleanFunction BooleanFunction::operator!() const
    {
        return this->operator~();
    }

    BooleanFunction BooleanFunction::operator~() const
    {
        auto result = *this;
        if ((m_content == content_type::TERMS && !m_operands.empty()) || m_content == content_type::VARIABLE)
        {
            result.m_invert = !result.m_invert;
        }
        else if (m_content == content_type::CONSTANT)
        {
            if (m_constant == ZERO)
                result.m_constant = ONE;
            else if (m_constant == ONE)
                result.m_constant = ZERO;
        }
        return result;
    }

    bool BooleanFunction::operator==(const BooleanFunction& other) const
    {
        if (is_empty() && other.is_empty())
        {
            return true;
        }
        if (m_content != other.m_content)
            return false;
        if (m_invert != other.m_invert)
            return false;
        return (m_content == content_type::VARIABLE && m_variable == other.m_variable) || (m_content == content_type::CONSTANT && m_constant == other.m_constant)
               || (m_content == content_type::TERMS && m_op == other.m_op && m_operands == other.m_operands);
    }
    bool BooleanFunction::operator!=(const BooleanFunction& other) const
    {
        return !(*this == other);
    }

    BooleanFunction BooleanFunction::replace_xors() const
    {
        if (m_content != content_type::TERMS)
        {
            return *this;
        }
        std::vector<BooleanFunction> terms;
        for (const auto& operand : m_operands)
        {
            terms.push_back(operand.replace_xors());
        }
        if (m_op != operation::XOR)
        {
            return BooleanFunction(m_op, terms, m_invert);
        }

        // actually replace the current xors
        auto result = (terms[0] & (~terms[1])) | ((~terms[0]) & terms[1]);
        for (u32 i = 2; i < terms.size(); ++i)
        {
            result = (result & (~terms[i])) | ((~result) & terms[i]);
        }

        if (m_invert)
        {
            result = ~result;
        }
        return result;
    }

    std::vector<BooleanFunction> BooleanFunction::expand_AND_of_functions(const std::vector<std::vector<BooleanFunction>>& AND_terms_to_expand) const
    {
        /*
        This helper function expands a list of AND terms
        Example: we started with ((e | f) & cd & (g | h), so this function is called with [[e, f], [cd], [g, h]]
        Strategy:
            1. initialize the output list with first set of AND terms --> [e, f]
            2. iterate over the remaining AND term sets --> [cd], [g, h]
                a. for each member of the current set, AND it to all terms in the current output and add each result to a new output list
                   reduce the output to only these new functions
                b. replace the old output list with the new output list

        Rundown:
            1: init output = [e,f]
            2: process [cd] --> output = [ecd, fcd]
            2: process [g, h] --> output = [ecdg, ecdh, fcdg, fcdh]
        */

        std::vector<BooleanFunction> result = AND_terms_to_expand[0];

        auto simple_hash = [](const BooleanFunction& f) -> std::string {
            std::string hash = f.m_invert ? "!#" : "#";
            for (const auto& var : f.m_operands)
            {
                if (var.m_invert)
                {
                    hash += "!";
                }
                if (var.m_content == content_type::CONSTANT)
                {
                    hash += "c" + std::to_string(static_cast<int>(var.m_constant) + 1);
                }
                else
                {
                    hash += "v" + var.m_variable;
                }
                hash += " ";
            }
            return hash;
        };

        for (u32 i = 1; i < AND_terms_to_expand.size(); ++i)
        {
            std::unordered_set<std::string> seen;
            std::vector<BooleanFunction> tmp;
            tmp.reserve(AND_terms_to_expand[i].size() * result.size());
            for (const auto& bf : AND_terms_to_expand[i])
            {
                for (const auto& bf2 : result)
                {
                    auto combined = (bf2 & bf).optimize_constants();
                    if (!(combined.m_content == content_type::CONSTANT && combined.m_constant == Value::ZERO))
                    {
                        if (combined.m_content == content_type::TERMS)
                        {
                            std::sort(combined.m_operands.begin(), combined.m_operands.end(), [](const auto& f1, const auto& f2) { return f1.m_variable < f2.m_variable; });
                        }
                        auto s = simple_hash(combined);
                        if (seen.find(s) == seen.end())
                        {
                            seen.insert(s);
                            tmp.push_back(combined);
                        }
                    }
                }
            }
            result = tmp;
        }

        return result;
    }

    std::vector<BooleanFunction> BooleanFunction::get_AND_terms() const
    {
        /*
        This helper function transforms the BF into a vector of terms that solely consist of the AND op
        Example: ab | (cd & (e | f)) | g --> [ad, cde, cdf, g]
        */

        if (m_content != content_type::TERMS)
        {
            return {*this};
        }

        if (m_op == operation::OR)
        {
            std::vector<BooleanFunction> AND_terms;
            for (const auto& operand : m_operands)
            {
                auto tmp = operand.get_AND_terms();
                AND_terms.insert(AND_terms.end(), tmp.begin(), tmp.end());
            }
            return AND_terms;
        }
        else    // m_op == AND
        {
            // at this point we potentially face a nested computation like "cd & (e | f)"
            // we have to expand the terms, i.e., AND every outer term with every inner term
            // for the example, we would output  (cd & (e | f)) --> [cde, cdf]
            std::vector<std::vector<BooleanFunction>> ANDed_functions;
            for (const auto& operand : m_operands)
            {
                ANDed_functions.push_back(operand.get_AND_terms());
            }
            return expand_AND_of_functions(ANDed_functions);
        }
    }

    BooleanFunction BooleanFunction::expand_ands() const
    {
        /*
        Strategy: expand all AND-multiplied terms
        Example: (a | b | c) & d --> ad | bd | cd
        Output: a flat OR-chain of AND terms
        */

        std::vector<BooleanFunction> primitives = get_AND_terms();
        if (primitives.empty())
        {
            return Value::ZERO;
        }
        return BooleanFunction(operation::OR, primitives);
    }

    BooleanFunction BooleanFunction::optimize_constants() const
    {
        if (is_empty() || m_content == content_type::VARIABLE || m_content == content_type::CONSTANT)
        {
            return *this;
        }

        std::vector<BooleanFunction> terms;
        for (const auto& operand : m_operands)
        {
            auto term = operand.optimize_constants();
            if (m_op == operation::OR)
            {
                if (term.is_constant_one())
                {
                    return BooleanFunction::ONE;
                }
                else if (term.is_constant_zero())
                {
                    continue;
                }
            }
            else if (m_op == operation::AND)
            {
                if (term.is_constant_one())
                {
                    continue;
                }
                else if (term.is_constant_zero())
                {
                    return BooleanFunction::ZERO;
                }
            }
            terms.push_back(term);
        }

        if (terms.empty())
        {
            if (m_op == operation::OR)
            {
                return BooleanFunction::ZERO;
            }
            else if (m_op == operation::AND)
            {
                return BooleanFunction::ONE;
            }
        }

        // remove contradictions etc
        for (u32 i = 0; i < terms.size(); ++i)
        {
            for (u32 j = i + 1; j < terms.size(); ++j)
            {
                if (terms[i].m_content == content_type::VARIABLE && terms[j].m_content == content_type::VARIABLE && terms[i].m_variable == terms[j].m_variable)
                {
                    if (terms[i].m_invert != terms[j].m_invert)
                    {
                        if (m_op == operation::AND)
                        {
                            return BooleanFunction::ZERO;
                        }
                        else if (m_op == operation::OR)
                        {
                            return BooleanFunction::ONE;
                        }
                    }
                    else
                    {
                        if (m_op == operation::AND || m_op == operation::OR)
                        {
                            terms.erase(terms.begin() + j);
                            j--;
                            continue;
                        }
                    }
                }
            }
        }

        return BooleanFunction(m_op, terms);
    }

    BooleanFunction BooleanFunction::propagate_negations(bool negate_term) const
    {
        if (m_content != content_type::TERMS)
        {
            if (negate_term)
            {
                return ~(*this);
            }
            return *this;
        }

        bool use_de_morgan = m_invert ^ negate_term;

        if (!use_de_morgan)
        {
            std::vector<BooleanFunction> terms;
            for (const auto& operand : m_operands)
            {
                terms.push_back(operand.propagate_negations(false));
            }
            return BooleanFunction(m_op, terms);
        }
        else
        {
            std::vector<BooleanFunction> terms;
            for (const auto& operand : m_operands)
            {
                terms.push_back(operand.propagate_negations(true));
            }
            if (m_op == operation::AND)
            {
                return BooleanFunction(operation::OR, terms);
            }
            else
            {
                return BooleanFunction(operation::AND, terms);
            }
        }
    }

    bool BooleanFunction::is_dnf() const
    {
        if (m_content != content_type::TERMS)
        {
            return true;
        }
        if (m_op == operation::AND)
        {
            for (const auto& subterm : m_operands)
            {
                if (subterm.m_content == content_type::TERMS || subterm.m_content == content_type::CONSTANT)
                {
                    return false;
                }
            }
        }
        else if (m_op != operation::OR)
        {
            return false;
        }
        for (const auto& term : m_operands)
        {
            if (term.m_content == content_type::TERMS)
            {
                if (term.m_op == operation::AND)
                {
                    for (const auto& subterm : term.m_operands)
                    {
                        if (subterm.m_content == content_type::TERMS || subterm.m_content == content_type::CONSTANT)
                        {
                            return false;
                        }
                    }
                }
                else
                {
                    return false;
                }
            }
        }
        return true;
    }

    BooleanFunction BooleanFunction::to_dnf() const
    {
        if (is_dnf())
        {
            return *this;
        }

        // debug progress prints
        // auto tmp_vars = get_variables();
        // std::vector<std::string> init_vars(tmp_vars.begin(), tmp_vars.end());
        // auto init_tt = get_truth_table(init_vars);
        // std::cout << "transforming " << *this << std::endl;
        // auto x = replace_xors();
        // std::cout << "  replace_xors " << x << std::endl;
        // if (x.get_truth_table(init_vars) != init_tt)
        //     std::cout << "FUNCTIONS DONT MATCH" << std::endl;
        // x = x.propagate_negations();
        // std::cout << "  propagate_negations " << x << std::endl;
        // if (x.get_truth_table(init_vars) != init_tt)
        //     std::cout << "FUNCTIONS DONT MATCH" << std::endl;
        // x = x.expand_ands();
        // std::cout << "  expand_ands " << x << std::endl;
        // if (x.get_truth_table(init_vars) != init_tt)
        //     std::cout << "FUNCTIONS DONT MATCH" << std::endl;
        // x = x.optimize_constants();
        // std::cout << "  optimize_constants " << x << std::endl;
        // if (x.get_truth_table(init_vars) != init_tt)
        //     std::cout << "FUNCTIONS DONT MATCH" << std::endl;
        // return x;

        // the order of the passes is important!
        // every pass after replace_xors expects that there are no more xor operations
        return replace_xors().propagate_negations().expand_ands().optimize_constants();
    }

    std::vector<std::vector<std::pair<std::string, bool>>> BooleanFunction::get_dnf_clauses() const
    {
        std::vector<std::vector<std::pair<std::string, bool>>> result;
        if (is_empty())
        {
            return result;
        }

        auto dnf = to_dnf();

        if (dnf.m_content == content_type::VARIABLE)
        {
            result.push_back({std::make_pair(dnf.m_variable, !dnf.m_invert)});
            return result;
        }
        else if (dnf.m_content == content_type::CONSTANT)
        {
            result.push_back({std::make_pair(to_string(dnf.m_constant), true)});
            return result;
        }
        if (dnf.m_op == operation::OR)
        {
            for (const auto& term : dnf.m_operands)
            {
                std::vector<std::pair<std::string, bool>> clause;
                if (term.m_content == content_type::TERMS)
                {
                    for (const auto& v : term.m_operands)
                    {
                        clause.push_back(std::make_pair(v.m_variable, !v.m_invert));
                    }
                }
                else
                {
                    clause.push_back(std::make_pair(term.m_variable, !term.m_invert));
                }
                result.push_back(clause);
            }
        }
        else
        {
            std::vector<std::pair<std::string, bool>> clause;
            for (const auto& v : dnf.m_operands)
            {
                clause.push_back(std::make_pair(v.m_variable, !v.m_invert));
            }
            result.push_back(clause);
        }
        return result;
    }

    std::vector<BooleanFunction::Value> BooleanFunction::get_truth_table(std::vector<std::string> variables, bool remove_unknown_variables) const
    {
        std::vector<Value> result;

        {
            auto unique_vars = get_variables();
            if (variables.empty())
            {
                variables = std::move(unique_vars);
            }
            else if (remove_unknown_variables)
            {
                variables.erase(std::remove_if(variables.begin(), variables.end(), [&unique_vars](auto& s) { return std::find(unique_vars.begin(), unique_vars.end(), s) == unique_vars.end(); }),
                                variables.end());
            }
        }

        for (u32 Values = 0; Values < (u32)(1 << variables.size()); ++Values)
        {
            std::unordered_map<std::string, Value> inputs;
            u32 tmp = Values;
            for (const auto& var : variables)
            {
                inputs[var] = (Value)(tmp & 1);
                tmp >>= 1;
            }
            result.push_back(evaluate(inputs));
        }
        return result;
    }

    BooleanFunction BooleanFunction::optimize() const
    {
        if (m_content != content_type::TERMS)
        {
            return *this;
        }

        BooleanFunction result = to_dnf().propagate_negations().optimize_constants();

        if (result.m_content != content_type::TERMS || result.m_op == operation::AND)
        {
            return result;
        }

        // result is an OR-chain of *multiple* AND-chains of *only variables*
        std::vector<std::vector<Value>> terms;
        std::vector<std::string> vars = get_variables();

        for (const auto& or_term : result.m_operands)
        {
            std::vector<Value> term(vars.size(), Value::X);
            if (or_term.m_content == content_type::TERMS)
            {
                for (const auto& and_term : or_term.m_operands)
                {
                    int index   = std::distance(vars.begin(), std::find(vars.begin(), vars.end(), and_term.m_variable));
                    term[index] = and_term.m_invert ? Value::ZERO : Value::ONE;
                }
            }
            else
            {
                int index   = std::distance(vars.begin(), std::find(vars.begin(), vars.end(), or_term.m_variable));
                term[index] = or_term.m_invert ? Value::ZERO : Value::ONE;
            }
            terms.emplace_back(term);
        }

        terms = qmc(terms);

        result = BooleanFunction();

        for (auto it = terms.begin(); it != terms.end(); ++it)
        {
            auto& term = (*it);
            BooleanFunction tmp;
            for (u32 i = 0; i < term.size(); ++i)
            {
                if (term[i] == Value::ONE)
                {
                    tmp &= BooleanFunction(vars[i]);
                }
                else if (term[i] == Value::ZERO)
                {
                    tmp &= ~BooleanFunction(vars[i]);
                }
            }
            if (tmp.is_empty())    // all variables are "dont care"
            {
                tmp = Value::ONE;
            }
            result |= tmp;
        }

        return result;
    }

    std::vector<std::vector<BooleanFunction::Value>> BooleanFunction::qmc(std::vector<std::vector<Value>> terms)
    {
        if (terms.empty())
        {
            return {};
        }

        u32 num_variables = terms[0].size();

        // repeatedly merge all groups that only differ in a single value
        while (true)
        {
            bool any_changes = false;
            std::vector<std::vector<Value>> new_merged_terms;
            std::vector<bool> removed_by_merge(terms.size(), false);
            // pairwise compare all terms...
            for (u32 i = 0; i < terms.size(); ++i)
            {
                for (u32 j = i + 1; j < terms.size(); ++j)
                {
                    // ...merge their values and count differences...
                    std::vector<Value> merged(num_variables, Value::X);
                    u32 cnt = 0;
                    for (u32 k = 0; k < num_variables; ++k)
                    {
                        if (terms[i][k] == terms[j][k])
                        {
                            merged[k] = terms[i][k];
                        }
                        else
                        {
                            ++cnt;
                        }
                    }
                    // ...and if they differ only in a single value, replace them with the merged term
                    if (cnt == 1)
                    {
                        removed_by_merge[i] = removed_by_merge[j] = true;
                        new_merged_terms.push_back(merged);
                        any_changes = true;
                    }
                }
            }
            if (!any_changes)
            {
                break;
            }
            for (u32 i = 0; i < terms.size(); ++i)
            {
                if (!removed_by_merge[i])
                {
                    new_merged_terms.push_back(terms[i]);
                }
            }
            std::sort(new_merged_terms.begin(), new_merged_terms.end());
            new_merged_terms.erase(std::unique(new_merged_terms.begin(), new_merged_terms.end()), new_merged_terms.end());
            terms = new_merged_terms;
        }

        std::vector<std::vector<Value>> output;

        // build ON-set minterms to later identify essential implicants
        std::map<u32, std::vector<u32>> table;
        for (u32 i = 0; i < terms.size(); ++i)
        {
            // find all inputs that are covered by term i
            // Example: term=01-1 --> all inputs covered are 0101 and 0111
            std::vector<u32> covered_inputs;
            for (u32 j = 0; j < num_variables; ++j)
            {
                if (terms[i][j] != Value::X)
                {
                    if (covered_inputs.empty())
                    {
                        covered_inputs.push_back(terms[i][j]);
                    }
                    else
                    {
                        for (auto& v : covered_inputs)
                        {
                            v = (v << 1) | terms[i][j];
                        }
                    }
                }
                else
                {
                    if (covered_inputs.empty())
                    {
                        covered_inputs.push_back(0);
                        covered_inputs.push_back(1);
                    }
                    else
                    {
                        std::vector<u32> tmp;
                        for (auto v : covered_inputs)
                        {
                            tmp.push_back((v << 1) | 0);
                            tmp.push_back((v << 1) | 1);
                        }
                        covered_inputs = tmp;
                    }
                }
            }

            std::sort(covered_inputs.begin(), covered_inputs.end());
            covered_inputs.erase(std::unique(covered_inputs.begin(), covered_inputs.end()), covered_inputs.end());

            // now memorize that all these inputs are covered by term i
            for (auto v : covered_inputs)
            {
                table[v].push_back(i);
            }
        }

        // helper function to add a term to the output and remove it from the table
        auto add_to_output = [&](u32 term_index) {
            output.push_back(terms[term_index]);
            for (auto it2 = table.cbegin(); it2 != table.cend();)
            {
                if (std::find(it2->second.begin(), it2->second.end(), term_index) != it2->second.end())
                {
                    it2 = table.erase(it2);
                }
                else
                {
                    ++it2;
                }
            }
        };

        // finally, identify essential implicants and add them to the output
        while (!table.empty())
        {
            bool no_change = true;

            for (auto& it : table)
            {
                if (it.second.size() == 1)
                {
                    no_change = false;
                    add_to_output(it.second[0]);
                    break;    // 'add_to_output' invalidates table iterator, so break and restart in next iteration
                }
            }

            if (no_change)
            {
                // none of the remaining terms is essential, just pick the one that covers most input values
                std::unordered_map<u32, u32> counter;
                for (auto it : table)
                {
                    for (auto term_index : it.second)
                    {
                        counter[term_index]++;
                    }
                }

                u32 index = std::max_element(counter.begin(), counter.end(), [](const auto& p1, const auto& p2) { return p1.second < p2.second; })->first;
                add_to_output(index);
            }
        }

        // sort output terms for deterministic output order (no impact on functionality)
        std::sort(output.begin(), output.end());

        return output;
    }

    z3::expr BooleanFunction::to_z3(z3::context& context, const std::map<std::string, z3::expr>& var_to_expr) const
    {
        // convert bf variables to z3::expr
        std::unordered_map<std::string, z3::expr> input2expr;

        for (const std::string& var : get_variables())
        {
            if (var_to_expr.find(var) == var_to_expr.end()) {
                input2expr.emplace(var, context.bv_const(var.c_str(), 1));
            }
            else {
                input2expr.insert({var, var_to_expr.at(var)});
            }
        }

        z3::expr expr = to_z3_internal(context, input2expr);

        return expr;
    }

    z3::expr BooleanFunction::to_z3_internal(z3::context& context, const std::unordered_map<std::string, z3::expr>& input2expr) const
    {
        z3::expr result(context);

        if (is_empty())
        {
            return result;
        }

        if (m_content == content_type::VARIABLE)
        {
            result = input2expr.at(m_variable);
        }
        else if (m_content == content_type::CONSTANT)
        {
            if (m_constant == Value::ZERO)
            {
                result = context.bv_val(0, 1);
            }
            else if (m_constant == Value::ONE)
            {
                result = context.bv_val(1, 1);
            }
            else
            {
                // TODO log error
            }
        }
        else
        {
            std::vector<z3::expr> terms;

            for (const BooleanFunction& x : m_operands)
            {
                terms.push_back(x.to_z3_internal(context, input2expr));
            }

            result = terms[0];

            for (u32 i = 1; i < terms.size(); ++i)
            {
                if (m_op == operation::OR)
                {
                    result = result | terms[i];
                }
                else if (m_op == operation::XOR)
                {
                    result = result ^ terms[i];
                }
                else if (m_op == operation::AND)
                {
                    result = result & terms[i];
                }
            }
        }

        if (m_invert)
        {
            result = ~result;
        }

        return result;
    }

    std::vector<std::unique_ptr<BooleanFunction::Node>> BooleanFunction::get_reverse_polish_notation() const {
        switch (this->m_content)
        {
            case BooleanFunction::content_type::CONSTANT:
            {
                std::vector<std::unique_ptr<BooleanFunction::Node>> nodes = {};
                nodes.emplace_back(BooleanFunction::OperandNode::make(m_constant, 1));
                if (this->m_invert) 
                {
                    nodes.emplace_back(BooleanFunction::OperationNode::make(BooleanFunction::NodeType::Not, 1));
                }
                return nodes;
            }
            case BooleanFunction::content_type::VARIABLE:
            {
                std::vector<std::unique_ptr<BooleanFunction::Node>> nodes = {};
                nodes.emplace_back(BooleanFunction::OperandNode::make(m_variable, 1));
                if (this->m_invert) 
                {
                    nodes.emplace_back(BooleanFunction::OperationNode::make(BooleanFunction::NodeType::Not, 1));
                }
                return nodes;
            } 
            case BooleanFunction::content_type::TERMS: 
            {
                std::vector<std::unique_ptr<BooleanFunction::Node>> nodes = {};
                for (const auto& operand : this->m_operands) {
                    for (auto&& node : operand.get_reverse_polish_notation()) {
                        nodes.emplace_back(std::move(node));
                    }
                }

                switch (this->m_op) 
                {
                    case BooleanFunction::operation::AND: nodes.emplace_back(BooleanFunction::OperationNode::make(BooleanFunction::NodeType::And, 1)); break;
                    case BooleanFunction::operation::OR:  nodes.emplace_back(BooleanFunction::OperationNode::make(BooleanFunction::NodeType::Or, 1)); break;
                    case BooleanFunction::operation::XOR: nodes.emplace_back(BooleanFunction::OperationNode::make(BooleanFunction::NodeType::Xor, 1)); break;
                }

                if (this->m_invert) 
                {
                    nodes.emplace_back(BooleanFunction::OperationNode::make(BooleanFunction::NodeType::Not, 1));
                }

                return nodes;
            }

            // this statement should never be reached
            default: return {};
        }
    }

    BooleanFunction::Node::Node(u16 _type, u16 _size) : 
        type(_type), size(_size) {}

    bool BooleanFunction::Node::operator==(const Node& other) const
    {
        if (this->is_operation() && other.is_operation()) {
        return static_cast<const OperationNode&>(*this) == static_cast<const OperationNode&>(other);
        }
        if (this->is_operand() && other.is_operand()) {
            return static_cast<const OperandNode&>(*this) == static_cast<const OperandNode&>(other);
        }

        return false;
    }
    
    bool BooleanFunction::Node::operator!=(const Node& other) const
    {
        return !(*this == other);
    }
    
    bool BooleanFunction::Node::operator <(const Node& other) const
    {
        if (this->is_operation() && other.is_operation()) {
            return static_cast<const OperationNode&>(*this) < static_cast<const OperationNode&>(other);
        }
        if (this->is_operand() && other.is_operand()) {
            return static_cast<const OperandNode&>(*this) < static_cast<const OperandNode&>(other);
        }
        return std::tie(type, size) < std::tie(other.type, other.size);
    }

    u16 BooleanFunction::Node::get_arity() const {
        return BooleanFunction::Node::get_arity(this->type);
    }

    u16 BooleanFunction::Node::get_arity(u16 type) {
        static const std::map<u16, u16> type2arity = {
            {BooleanFunction::NodeType::And, 2},
            {BooleanFunction::NodeType::Or,  2},
            {BooleanFunction::NodeType::Not, 1},
            {BooleanFunction::NodeType::Xor, 2},

            {BooleanFunction::NodeType::Add, 2},

            {BooleanFunction::NodeType::Concat, 2},
            {BooleanFunction::NodeType::Zext, 2},
            {BooleanFunction::NodeType::Slice, 3},

            {BooleanFunction::NodeType::Constant, 0},
            {BooleanFunction::NodeType::Index, 0},
            {BooleanFunction::NodeType::Variable, 0},
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

    bool BooleanFunction::Node::is_index() const 
    {
        return this->is(BooleanFunction::NodeType::Index);
    }

    bool BooleanFunction::Node::is_variable() const 
    { 
        return this->is(BooleanFunction::NodeType::Variable); 
    }

    bool BooleanFunction::Node::is_operation() const 
    {
        return !this->is_constant() && !this->is_variable(); 
    }
    
    bool BooleanFunction::Node::is_operand() const 
    {
        return !this->is_operation(); 
    }

    std::unique_ptr<BooleanFunction::Node> BooleanFunction::OperationNode::make(u16 type, u16 size)
    {
        return std::unique_ptr<BooleanFunction::Node>(new OperationNode(type, size));
    }

    bool BooleanFunction::OperationNode::operator==(const BooleanFunction::OperationNode& other) const 
    {
        return std::tie(this->type, this->size) == std::tie(other.type, other.size);
    }

    bool BooleanFunction::OperationNode::operator!=(const BooleanFunction::OperationNode& other) const 
    {
        return !(*this == other);
    }

    bool BooleanFunction::OperationNode::operator <(const BooleanFunction::OperationNode& other) const 
    {
        return std::tie(this->type, this->size) < std::tie(other.type, other.size);
    }

    std::unique_ptr<BooleanFunction::Node> BooleanFunction::OperationNode::clone() const 
    {
        return std::unique_ptr<BooleanFunction::Node>(new OperationNode(*this));
    }

    std::string BooleanFunction::OperationNode::to_string() const 
    {
        static const std::map<u16, std::string>& operation2name = {
            {BooleanFunction::NodeType::And, "&"},
            {BooleanFunction::NodeType::Or, "|"},
            {BooleanFunction::NodeType::Not, "~"},
            {BooleanFunction::NodeType::Xor, "^"},

            {BooleanFunction::NodeType::Add, "+"},

            {BooleanFunction::NodeType::Concat, "++"},
            {BooleanFunction::NodeType::Slice, "Slice"},
            {BooleanFunction::NodeType::Zext, "Zext"},
        };

        return operation2name.at(this->type);
    }

    BooleanFunction::OperationNode::OperationNode(u16 _type, u16 _size) :
        BooleanFunction::Node(_type, _size) {}

    std::unique_ptr<BooleanFunction::Node> BooleanFunction::OperandNode::make(BooleanFunction::Value _constant, u16 _size)
    {
        return std::unique_ptr<BooleanFunction::Node>(new OperandNode(NodeType::Constant, _size, _constant, 0, ""));
    }
 
    std::unique_ptr<BooleanFunction::Node> BooleanFunction::OperandNode::make(u16 _index, u16 _size) 
    {
        return std::unique_ptr<BooleanFunction::Node>(new OperandNode(NodeType::Index, _size, BooleanFunction::Value::X, _index, ""));
    }

    std::unique_ptr<BooleanFunction::Node> BooleanFunction::OperandNode::make(const std::string& _name, u16 _size) 
    {
        return std::unique_ptr<BooleanFunction::Node>(new OperandNode(NodeType::Variable, _size, BooleanFunction::Value::X, 0, _name));
    }

    bool BooleanFunction::OperandNode::operator==(const BooleanFunction::OperandNode& other) const 
    {
        return std::tie(this->type, this->size, this->constant, this->index, this->variable) == std::tie(other.type, other.size, other.constant, other.index, other.variable);
    }

    bool BooleanFunction::OperandNode::operator!=(const BooleanFunction::OperandNode& other) const 
    {
        return !(*this == other);
    }

    bool BooleanFunction::OperandNode::operator <(const BooleanFunction::OperandNode& other) const 
    {
        return std::tie(this->type, this->size, this->constant, this->index, this->variable) < std::tie(other.type, other.size, other.constant, other.index, other.variable);
    }

    std::unique_ptr<BooleanFunction::Node> BooleanFunction::OperandNode::clone() const 
    {
        return std::unique_ptr<BooleanFunction::Node>(new OperandNode(*this));
    }

    std::string BooleanFunction::OperandNode::to_string() const 
    {
        static const std::map<u16, std::function<std::string(const OperandNode*)>> operand2name = 
        {
            {BooleanFunction::NodeType::Constant, [] (const OperandNode* node) { return enum_to_string(node->constant); }},
            {BooleanFunction::NodeType::Index, [] (const OperandNode* node) { return std::to_string(node->index); }},
            {BooleanFunction::NodeType::Variable, [] (const OperandNode* node) { return node->variable; }},
        };

        return operand2name.at(this->type)(this);
    }

    BooleanFunction::OperandNode::OperandNode(u16 _type, u16 _size, BooleanFunction::Value _constant, u16 _index, const std::string& _variable) :
        BooleanFunction::Node(_type, _size), constant(_constant), index(_index), variable(_variable) {}

}    // namespace hal
