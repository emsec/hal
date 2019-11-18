#include "netlist/boolean_function.h"

#include "core/utils.h"

std::string boolean_function::to_string(const operation& op)
{
    switch (op)
    {
        case operation::AND:
            return "&";
        case operation::OR:
            return "|";
        case operation::XOR:
            return "^";
    }
}

std::ostream& operator<<(std::ostream& os, const boolean_function::operation& op)
{
    return os << boolean_function::to_string(op);
}

std::string boolean_function::to_string(const value& v)
{
    switch (v)
    {
        case value::ONE:
            return "1";
        case value::ZERO:
            return "0";
        default:
            return "X";
    }
}

std::ostream& operator<<(std::ostream& os, const boolean_function::value& v)
{
    return os << boolean_function::to_string(v);
}

boolean_function::boolean_function()
{
    m_content = content_type::TERMS;
    m_invert  = false;
}

boolean_function::boolean_function(operation op, const std::vector<boolean_function>& operands, bool invert_result)
{
    if (operands.empty())
    {
        m_content = content_type::CONSTANT;
        m_invert  = false;

        m_constant = value::X;
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
        std::copy_if(operands.begin(), operands.end(), std::back_inserter(m_operands), [](auto op) { return !op.is_empty(); });
    }
}

boolean_function::boolean_function(const std::string& variable_name, bool invert_result)
{
    m_content = content_type::VARIABLE;
    m_invert  = invert_result;

    m_variable = core_utils::trim(variable_name);
    assert(!m_variable.empty());
}

boolean_function::boolean_function(value constant)
{
    m_content = content_type::CONSTANT;
    m_invert  = false;

    m_constant = constant;
}

boolean_function boolean_function::substitute(const std::string& variable_name, const boolean_function& function) const
{
    if (m_content == content_type::VARIABLE && m_variable == variable_name)
    {
        return function;
    }
    else if (m_content == content_type::TERMS)
    {
        auto result = *this;
        for (u32 i = 0; i < m_operands.size(); ++i)
        {
            result.m_operands[i] = result.m_operands[i].substitute(variable_name, function);
        }
        return result;
    }
    return *this;
}

boolean_function::value boolean_function::evaluate(const std::map<std::string, value>& inputs) const
{
    value result = X;
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
            if ((m_op == operation::AND && result == 0) || (m_op == operation::OR && result == 1) || (m_op == operation::XOR && result == X))
            {
                break;
            }

            auto next = m_operands[i].evaluate(inputs);
            if (m_op == operation::AND)
            {
                if (next == 0 || result == 0)
                {
                    result = (value)0;
                }
                else if (next == X && result == 1)
                {
                    result = X;
                }
            }
            else if (m_op == operation::OR)
            {
                if (next == 1 || result == 1)
                {
                    result = (value)1;
                }
                else if (next == X && result == 0)
                {
                    result = X;
                }
            }
            else if (m_op == operation::XOR)
            {
                if (next == 1)
                {
                    result = (value)(1 - result);
                }
                else if (next == X)
                {
                    result = X;
                }
            }
        }
    }

    if (m_invert)
    {
        if (result == 1)
        {
            return (value)0;
        }
        else if (result == 0)
        {
            return (value)1;
        }
    }
    return result;
}

boolean_function::value boolean_function::operator()(const std::map<std::string, boolean_function::value>& inputs) const
{
    return evaluate(inputs);
}

bool boolean_function::is_constant_one() const
{
    if (m_content == content_type::CONSTANT)
    {
        return m_constant == ONE;
    }
    auto tmp = optimize();
    return tmp.m_content == content_type::CONSTANT && tmp.m_constant == ONE;
}

bool boolean_function::is_constant_zero() const
{
    if (m_content == content_type::CONSTANT)
    {
        return m_constant == ZERO;
    }
    auto tmp = optimize();
    return tmp.m_content == content_type::CONSTANT && tmp.m_constant == ZERO;
}

bool boolean_function::is_empty() const
{
    return m_content == content_type::TERMS && m_operands.empty();
}

std::set<std::string> boolean_function::get_variables() const
{
    if (m_content == content_type::VARIABLE)
    {
        return {m_variable};
    }
    else if (m_content == content_type::TERMS)
    {
        std::set<std::string> result;
        for (const auto& f : m_operands)
        {
            auto tmp = f.get_variables();
            result.insert(tmp.begin(), tmp.end());
        }
        return result;
    }
    return {};
}

boolean_function boolean_function::from_string(std::string expression)
{
    expression = core_utils::trim(expression);

    if (expression.empty())
    {
        return boolean_function();
    }

    const std::string delimiters = "!^&|'+* ";

    // check for constants
    if (expression == "0")
    {
        return boolean_function(ZERO);
    }
    else if (expression == "1")
    {
        return boolean_function(ONE);
    }
    else if (expression == "X")
    {
        return boolean_function(X);
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
            }
        }
        if (!is_term)
        {
            return boolean_function(expression);
        }
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
                    current_term = core_utils::trim(current_term);
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
                current_term = core_utils::trim(current_term);
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
                    current_term = core_utils::trim(current_term);
                    if (!current_term.empty())
                    {
                        terms.push_back(current_term);
                        current_term.clear();
                    }
                }
            }
        }
        current_term = core_utils::trim(current_term);
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
        return from_string(terms[0].substr(1, terms[0].size() - 2));
    }

    bool negate_next  = false;
    operation next_op = operation::AND;

    // multiple terms available -> initialize return value with first term
    u32 i = 0;
    while (terms[i] == "!")
    {
        negate_next = !negate_next;
        ++i;
    }
    boolean_function result = from_string(terms[i]);
    while (i + 1 < terms.size() && terms[i + 1] == "'")
    {
        negate_next = !negate_next;
        ++i;
    }
    if (negate_next)
    {
        result      = !result;
        negate_next = false;
    }

    // process the remaining terms and update the result function
    while (++i < terms.size())
    {
        if (terms[i] == "!")
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
            auto next_term = from_string(terms[i]);
            while (i + 1 < terms.size() && terms[i + 1] == "'")
            {
                negate_next = !negate_next;
                ++i;
            }
            if (negate_next)
            {
                next_term = !next_term;
            }
            result = result.combine(next_op, next_term);

            negate_next = false;
            next_op     = operation::AND;
        }
    }

    return result;
}

std::string boolean_function::to_string() const
{
    if (is_empty())
    {
        return "<empty>";
    }
    auto s = to_string_internal();
    if (s.front() == '(' && s.back() == ')')
    {
        return s.substr(1, s.size() - 2);
    }
    return s;
}

std::string boolean_function::to_string_internal() const
{
    std::string result = to_string(value::X);
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

        result = "(" + core_utils::join(op_str, terms) + ")";
    }

    if (m_invert)
    {
        result = "!" + result;
    }
    return result;
}

std::ostream& operator<<(std::ostream& os, const boolean_function& f)
{
    return os << f.to_string();
}

boolean_function boolean_function::combine(operation op, const boolean_function& other) const
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
        boolean_function result(op, joint_operands);
        return result;
    }
    else if (m_content == content_type::TERMS && m_op == op)
    {
        boolean_function result = *this;
        result.m_operands.push_back(other);
        return result;
    }
    else if (other.m_content == content_type::TERMS && other.m_op == op)
    {
        boolean_function result = other;
        result.m_operands.insert(result.m_operands.begin(), *this);
        return result;
    }
    return boolean_function(op, {*this, other});
}

boolean_function boolean_function::operator&(const boolean_function& other) const
{
    return combine(operation::AND, other);
}

boolean_function boolean_function::operator|(const boolean_function& other) const
{
    return combine(operation::OR, other);
}

boolean_function boolean_function::operator^(const boolean_function& other) const
{
    return combine(operation::XOR, other);
}

boolean_function& boolean_function::operator&=(const boolean_function& other)
{
    *this = combine(operation::AND, other);
    return *this;
}
boolean_function& boolean_function::operator|=(const boolean_function& other)
{
    *this = combine(operation::OR, other);
    return *this;
}
boolean_function& boolean_function::operator^=(const boolean_function& other)
{
    *this = combine(operation::XOR, other);
    return *this;
}

boolean_function boolean_function::operator!() const
{
    auto result = *this;
    if (!(m_content == content_type::TERMS && m_operands.empty()))
    {
        result.m_invert = !result.m_invert;
    }
    return result;
}

bool boolean_function::operator==(const boolean_function& other) const
{
    return m_invert == other.m_invert && m_content == other.m_content && m_variable == other.m_variable && m_constant == other.m_constant && m_op == other.m_op && m_operands == other.m_operands;
}
bool boolean_function::operator!=(const boolean_function& other) const
{
    return !(*this == other);
}


boolean_function boolean_function::replace_xors() const
{
    if (m_content != content_type::TERMS)
    {
        return *this;
    }
    std::vector<boolean_function> terms;
    for (const auto& operand : m_operands)
    {
        terms.push_back(operand.replace_xors());
    }
    if (m_op != operation::XOR)
    {
        return boolean_function(m_op, terms, m_invert);
    }

    // actually replace the current xors
    auto result = (terms[0] & !terms[1]) | ((!terms[0]) & terms[1]);
    for (u32 i = 2; i < terms.size(); ++i)
    {
        result = (result & !terms[i]) | ((!result) & terms[i]);
    }
    if (m_invert)
    {
        result = !result;
    }
    return result;
}

std::vector<boolean_function> boolean_function::expand_ands_internal(const std::vector<std::vector<boolean_function>>& sub_primitives, u32 i) const
{
    if (i >= sub_primitives.size())
    {
        return {boolean_function::ONE};
    }
    std::vector<boolean_function> result;
    for (const auto& x : sub_primitives[i])
    {
        for (const auto& y : expand_ands_internal(sub_primitives, i + 1))
        {
            result.push_back(x & y);
        }
    }
    return result;
}

std::vector<boolean_function> boolean_function::get_primitives() const
{
    if (m_content != content_type::TERMS)
    {
        return {*this};
    }

    if (m_op == operation::OR)
    {
        std::vector<boolean_function> primitives;
        for (const auto& operand : m_operands)
        {
            auto tmp = operand.get_primitives();
            primitives.insert(primitives.end(), tmp.begin(), tmp.end());
        }
        return primitives;
    }
    else
    {
        std::vector<std::vector<boolean_function>> sub_primitives;
        for (const auto& operand : m_operands)
        {
            sub_primitives.push_back(operand.get_primitives());
        }
        return expand_ands_internal(sub_primitives, 0);
    }
}

boolean_function boolean_function::expand_ands() const
{
    return boolean_function(operation::OR, get_primitives());
}

boolean_function boolean_function::optimize_constants() const
{
    if (m_content == content_type::VARIABLE)
    {
        return *this;
    }
    if (m_content == content_type::CONSTANT)
    {
        if (m_invert)
        {
            if (m_constant == 0)
            {
                return boolean_function::ONE;
            }
            else if (m_constant == 1)
            {
                return boolean_function::ZERO;
            }
        }
        return *this;
    }

    std::vector<boolean_function> terms;
    for (const auto& operand : m_operands)
    {
        auto term = operand.optimize_constants();
        if (m_op == operation::OR)
        {
            if (term.is_constant_one())
            {
                return boolean_function::ONE;
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
                return boolean_function::ZERO;
            }
        }
        terms.push_back(term);
    }

    // remove contradictions etc
    for (auto it = terms.begin(); it != terms.end(); ++it)
    {
        for (auto it2 = it + 1; it2 != terms.end();)
        {
            if (it->m_content == content_type::VARIABLE && it2->m_content == content_type::VARIABLE && it->m_variable == it2->m_variable)
            {
                if (it->m_invert != it2->m_invert)
                {
                    if (m_op == operation::AND)
                    {
                        return boolean_function::ZERO;
                    }
                    else if (m_op == operation::OR)
                    {
                        return boolean_function::ONE;
                    }
                }
                else
                {
                    if (m_op == operation::AND)
                    {
                        it2 = terms.erase(it2);
                        continue;
                    }
                    else if (m_op == operation::OR)
                    {
                        it2 = terms.erase(it2);
                        continue;
                    }
                }
            }
            ++it2;
        }
    }

    return boolean_function(m_op, terms);
}

boolean_function boolean_function::propagate_negations(bool negate_term) const
{
    if (m_content != content_type::TERMS)
    {
        boolean_function result = *this;
        result.m_invert         = result.m_invert ^ negate_term;
        return result;
    }

    bool use_de_morgan = m_invert ^ negate_term;

    if (!use_de_morgan)
    {
        std::vector<boolean_function> terms;
        for (const auto& operand : m_operands)
        {
            terms.push_back(operand.propagate_negations(false));
        }
        return boolean_function(m_op, terms);
    }
    else
    {
        std::vector<boolean_function> terms;
        for (const auto& operand : m_operands)
        {
            terms.push_back(operand.propagate_negations(true));
        }
        if (m_op == operation::AND)
        {
            return boolean_function(operation::OR, terms);
        }
        else
        {
            return boolean_function(operation::AND, terms);
        }
    }
}

boolean_function boolean_function::flatten() const
{
    if (m_content != content_type::TERMS)
    {
        return *this;
    }

    std::vector<boolean_function> terms;
    for (const auto& operand : m_operands)
    {
        auto term = operand.flatten();
        if (term.m_content == content_type::TERMS && m_op == term.m_op)
        {
            for (const auto& x : term.m_operands)
            {
                terms.push_back(x);
            }
        }
        else
        {
            terms.push_back(term);
        }
    }
    return boolean_function(m_op, terms);
}

bool boolean_function::is_dnf() const
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

boolean_function boolean_function::to_dnf() const
{
    if (is_dnf())
    {
        return *this;
    }
    // the order of the passes is important!
    // every pass after replace_xors expects that there are no more xor operations
    return replace_xors().propagate_negations().expand_ands().flatten().optimize_constants();
}

std::vector<boolean_function::value> boolean_function::get_truth_table(const std::vector<std::string>& order) const
{
    std::vector<value> result;

    std::vector<std::string> variables = order;
    if (variables.empty())
    {
        auto unique_vars = get_variables();
        variables.insert(variables.end(), unique_vars.begin(), unique_vars.end());
    }

    for (u32 values = 0; values < (u32)(1 << variables.size()); ++values)
    {
        std::map<std::string, boolean_function::value> inputs;
        u32 tmp = values;
        for (const auto& var : variables)
        {
            inputs[var] = (boolean_function::value)(tmp & 1);
            tmp >>= 1;
        }
        result.push_back(evaluate(inputs));
    }
    return result;
}

boolean_function boolean_function::optimize() const
{
    if (m_content != content_type::TERMS)
    {
        return *this;
    }

    boolean_function result = to_dnf();

    if (result.m_content != content_type::TERMS || result.m_op == operation::AND)
    {
        return result.optimize_constants();
    }

    // result is a OR-chain of *multiple* AND-chains of *only variables*
    std::vector<std::vector<value>> terms;
    auto vars_set = get_variables();
    std::vector<std::string> vars(vars_set.begin(), vars_set.end());
    for (const auto& or_term : result.m_operands)
    {
        std::vector<value> term(vars.size(), value::X);
        for (const auto& and_term : or_term.m_operands)
        {
            int index   = std::distance(vars.begin(), std::find(vars.begin(), vars.end(), and_term.m_variable));
            term[index] = and_term.m_invert ? value::ZERO : value::ONE;
        }
        terms.emplace_back(term);
    }

    terms = qmc(terms);

    result = boolean_function();
    for (const auto& term : terms)
    {
        boolean_function tmp;
        for (u32 i = 0; i < term.size(); ++i)
        {
            if (term[i] == value::ONE)
            {
                tmp &= boolean_function(vars[i]);
            }
            else if (term[i] == value::ZERO)
            {
                tmp &= !boolean_function(vars[i]);
            }
        }
        result |= tmp;
    }

    return result;
}

std::vector<std::vector<boolean_function::value>> boolean_function::qmc(const std::vector<std::vector<value>>& terms)
{
    std::vector<std::vector<value>> result;

    if (terms.empty())
    {
        return result;
    }

    std::vector<std::vector<value>> im;
    std::vector<bool> mark(terms.size(), false);
    u32 term_size = terms[0].size();

    for (u32 i = 0; i < terms.size(); ++i)
    {
        for (u32 j = i + 1; j < terms.size(); ++j)
        {
            std::vector<value> c(term_size, value::X);
            u32 cnt = 0;
            for (u32 k = 0; k < term_size; ++k)
            {
                if (terms[i][k] == terms[j][k])
                {
                    c[k] = terms[i][k];
                }
                else
                {
                    ++cnt;
                }
            }
            if (cnt > 1)
            {
                continue;
            }
            im.push_back(c);
            mark[i] = mark[j] = true;
        }
    }

    for (u32 i = 0; i < terms.size(); ++i)
    {
        if (!mark[i])
        {
            result.push_back(terms[i]);
        }
    }

    if (result.size() == terms.size() || terms.size() == 1)
    {
        return result;
    }

    std::vector<bool> mark2(im.size(), false);
    for (u32 i = 0; i < im.size(); ++i)
    {
        for (u32 j = i + 1; j < im.size(); ++j)
        {
            if (!mark2[j] && im[i] == im[j])
            {
                mark2[j] = true;
            }
        }
    }

    u32 cnt = 0;
    im.erase(std::remove_if(im.begin(), im.end(), [&](auto&) { return mark2[cnt++]; }), im.end());

    im = qmc(im);
    result.insert(result.end(), im.begin(), im.end());
    return result;
}
