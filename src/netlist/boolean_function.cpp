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
        default:
            return "?";
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

boolean_function::boolean_function(operation op, const std::vector<boolean_function>& operands, bool invert_result) : boolean_function()
{
    if (operands.empty())
    {
        m_content  = content_type::CONSTANT;
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
        std::copy_if(operands.begin(), operands.end(), std::back_inserter(m_operands), [](auto op2) { return !op2.is_empty(); });
    }
}

boolean_function::boolean_function(const std::string& variable_name) : boolean_function()
{
    m_content  = content_type::VARIABLE;
    m_variable = core_utils::trim(variable_name);
    assert(!m_variable.empty());
}

boolean_function::boolean_function(value constant) : boolean_function()
{
    m_content  = content_type::CONSTANT;
    m_constant = constant;
}

boolean_function boolean_function::substitute(const std::string& old_variable_name, const std::string& new_variable_name) const
{
    return substitute(old_variable_name, boolean_function(new_variable_name));
}

boolean_function boolean_function::substitute(const std::string& variable_name, const boolean_function& function) const
{
    if (m_content == content_type::VARIABLE && m_variable == variable_name)
    {
        if (m_invert)
        {
            return !function;
        }
        else
        {
            return function;
        }
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

boolean_function boolean_function::from_string(std::string expression, const std::vector<std::string>& variable_names)
{
    auto sorted_variable_names = variable_names;
    std::sort(sorted_variable_names.begin(), sorted_variable_names.end(), [](const auto& a, const auto& b) { return a.size() > b.size(); });

    for (u32 i = 0; i < sorted_variable_names.size(); ++i)
    {
        auto pos = expression.find(sorted_variable_names[i]);
        if (pos != std::string::npos)
        {
            expression.replace(pos, sorted_variable_names[i].size(), "__v_" + std::to_string(i));
        }
    }

    return from_string_internal(expression, sorted_variable_names);
}

boolean_function boolean_function::from_string_internal(std::string expression, const std::vector<std::string>& variable_names)
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
                break;
            }
        }
        if (!is_term)
        {
            if (core_utils::starts_with(expression, "__v_"))
            {
                u32 idx    = std::stoul(expression.substr(4));
                expression = variable_names[idx];
            }

            return boolean_function(expression);
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
                return value::X;
            }
        }
    }
    if (level != 0)
    {
        return value::X;
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
        return from_string_internal(terms[0].substr(1, terms[0].size() - 2), variable_names);
    }

    // small mutable datastructure for parsing
    struct op_term
    {
        operation op;
        boolean_function term;
    };
    std::vector<op_term> parsed_terms;

    bool negate_next  = false;
    operation next_op = operation::AND;

    {
        // multiple terms available -> initialize return value with first term
        u32 i = 0;
        while (terms[i] == "!")
        {
            negate_next = !negate_next;
            ++i;
        }
        boolean_function first_term = from_string_internal(terms[i], variable_names);
        while (i + 1 < terms.size() && terms[i + 1] == "'")
        {
            negate_next = !negate_next;
            ++i;
        }
        if (negate_next)
        {
            first_term  = !first_term;
            negate_next = false;
        }

        parsed_terms.push_back({operation::AND, first_term});

        // process the remaining terms
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
                auto next_term = from_string_internal(terms[i], variable_names);
                while (i + 1 < terms.size() && terms[i + 1] == "'")
                {
                    negate_next = !negate_next;
                    ++i;
                }
                if (negate_next)
                {
                    next_term = !next_term;
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
    else if (m_content == content_type::TERMS && m_op == op && !m_invert)
    {
        boolean_function result = *this;
        result.m_operands.push_back(other);
        return result;
    }
    else if (other.m_content == content_type::TERMS && other.m_op == op && !other.m_invert)
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

bool boolean_function::operator==(const boolean_function& other) const
{
    if (m_content != other.m_content)
        return false;
    if (m_invert != other.m_invert)
        return false;
    return (m_content == content_type::VARIABLE && m_variable == other.m_variable) || (m_content == content_type::CONSTANT && m_constant == other.m_constant)
           || (m_content == content_type::TERMS && m_op == other.m_op && m_operands == other.m_operands);
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
    auto result = (terms[0] & (!terms[1])) | ((!terms[0]) & terms[1]);
    for (u32 i = 2; i < terms.size(); ++i)
    {
        result = (result & (!terms[i])) | ((!result) & terms[i]);
    }

    if (m_invert)
    {
        result = !result;
    }
    return result;
}

std::vector<boolean_function> boolean_function::expand_ands_internal(const std::vector<std::vector<boolean_function>>& sub_primitives) const
{
    std::vector<boolean_function> result = sub_primitives[0];

    auto set_identifier = [](const boolean_function& f) -> std::string {
        std::string id;
        for (const auto& var : f.m_operands)
        {
            if (var.m_invert)
                id += "!";
            id += var.m_variable;
            id += " ";
        }
        return id;
    };

    for (u32 i = 1; i < sub_primitives.size(); ++i)
    {
        std::set<std::string> seen;
        std::vector<boolean_function> tmp;
        tmp.reserve(sub_primitives[i].size() * result.size());
        for (const auto& bf : sub_primitives[i])
        {
            for (const auto& bf2 : result)
            {
                auto combined = (bf2 & bf).optimize_constants();
                if (!(combined.m_content == content_type::CONSTANT && combined.m_constant == value::ZERO))
                {
                    if (combined.m_content == content_type::TERMS)
                    {
                        std::sort(combined.m_operands.begin(), combined.m_operands.end(), [](const auto& f1, const auto& f2) { return f1.m_variable < f2.m_variable; });
                    }
                    auto s = set_identifier(combined);
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
    else    // m_op == AND
    {
        std::vector<std::vector<boolean_function>> sub_primitives;
        for (const auto& operand : m_operands)
        {
            sub_primitives.push_back(operand.get_primitives());
        }
        return expand_ands_internal(sub_primitives);
    }
}

boolean_function boolean_function::expand_ands() const
{
    auto primitives = get_primitives();
    if (primitives.empty())
    {
        return value::ZERO;
    }
    return boolean_function(operation::OR, primitives);
}

boolean_function boolean_function::optimize_constants() const
{
    if (is_empty() || m_content == content_type::VARIABLE || m_content == content_type::CONSTANT)
    {
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

    if (terms.empty())
    {
        if (m_op == operation::OR)
        {
            return boolean_function::ZERO;
        }
        else if (m_op == operation::AND)
        {
            return boolean_function::ONE;
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
                        return boolean_function::ZERO;
                    }
                    else if (m_op == operation::OR)
                    {
                        return boolean_function::ONE;
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

    return boolean_function(m_op, terms);
}

boolean_function boolean_function::propagate_negations(bool negate_term) const
{
    if (m_content != content_type::TERMS)
    {
        if (negate_term)
        {
            return !(*this);
        }
        return *this;
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
    // x = x.flatten();
    // std::cout << "  flatten " << x << std::endl;
    // if (x.get_truth_table(init_vars) != init_tt)
    //     std::cout << "FUNCTIONS DONT MATCH" << std::endl;
    // x = x.optimize_constants();
    // std::cout << "  optimize_constants " << x << std::endl;
    // if (x.get_truth_table(init_vars) != init_tt)
    //     std::cout << "FUNCTIONS DONT MATCH" << std::endl;
    // return x;

    // the order of the passes is important!
    // every pass after replace_xors expects that there are no more xor operations
    return replace_xors().propagate_negations().expand_ands() /*.flatten()*/.optimize_constants();
}

std::vector<std::vector<std::pair<std::string, bool>>> boolean_function::get_dnf_clauses() const
{
    auto dnf = to_dnf();

    std::vector<std::vector<std::pair<std::string, bool>>> result;

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

std::vector<boolean_function::value> boolean_function::get_truth_table(std::vector<std::string> variables) const
{
    std::vector<value> result;

    auto unique_vars = get_variables();
    if (variables.empty())
    {
        variables.insert(variables.end(), unique_vars.begin(), unique_vars.end());
    }
    else
    {
        variables.erase(std::remove_if(variables.begin(), variables.end(), [&unique_vars](auto& s) { return unique_vars.find(s) == unique_vars.end(); }), variables.end());
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

    boolean_function result = to_dnf().propagate_negations().optimize_constants();

    if (result.m_content != content_type::TERMS || result.m_op == operation::AND)
    {
        return result;
    }

    // result is a OR-chain of *multiple* AND-chains of *only variables*
    std::vector<std::vector<value>> terms;
    auto vars_set = get_variables();
    std::vector<std::string> vars(vars_set.begin(), vars_set.end());
    for (const auto& or_term : result.m_operands)
    {
        std::vector<value> term(vars.size(), value::X);
        if (or_term.m_content == content_type::TERMS)
        {
            for (const auto& and_term : or_term.m_operands)
            {
                int index   = std::distance(vars.begin(), std::find(vars.begin(), vars.end(), and_term.m_variable));
                term[index] = and_term.m_invert ? value::ZERO : value::ONE;
            }
        }
        else
        {
            int index   = std::distance(vars.begin(), std::find(vars.begin(), vars.end(), or_term.m_variable));
            term[index] = or_term.m_invert ? value::ZERO : value::ONE;
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
