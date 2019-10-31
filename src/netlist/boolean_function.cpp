#include "netlist/boolean_function.h"

#include "core/utils.h"

boolean_function::boolean_function(operation op, const std::vector<boolean_function>& operands, bool invert_result)
{
    m_holds_variable = false;
    m_holds_constant = false;
    m_invert         = invert_result;

    m_op       = op;
    m_operands = operands;
}

boolean_function::boolean_function(const std::string& variable, bool invert_result)
{
    m_holds_variable = true;
    m_holds_constant = false;
    m_invert         = invert_result;

    m_variable = variable;
}

boolean_function::boolean_function(value constant)
{
    m_holds_variable = false;
    m_holds_constant = true;
    m_invert         = false;

    m_constant = constant;
}

boolean_function::value boolean_function::evaluate(const std::map<std::string, value>& inputs) const
{
    value result = X;
    if (m_holds_variable)
    {
        auto it = inputs.find(m_variable);
        if (it != inputs.end())
        {
            result = it->second;
        }
    }
    else if (m_holds_constant)
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

bool boolean_function::is_constant_one() const
{
    return m_holds_constant && m_constant == 1;
}

bool boolean_function::is_constant_zero() const
{
    return m_holds_constant && m_constant == 0;
}

boolean_function boolean_function::from_string(std::string expression)
{
    expression = core_utils::trim(expression);

    // check for constants
    if (expression == "0")
    {
        return boolean_function(ZERO);
    }
    if (expression == "1")
    {
        return boolean_function(ONE);
    }
    if (expression == "X")
    {
        return boolean_function(X);
    }

    // parse equation
    const std::string delimiters = "!^&|";
    std::vector<std::string> parts;
    std::string current_part;
    u32 bracket_level = 0;
    for (u32 i = 0; i < expression.size(); i++)
    {
        if (expression[i] == '(')
        {
            if (!current_part.empty())
            {
                parts.push_back(core_utils::trim(current_part));
                current_part.clear();
            }

            bracket_level++;
        }

        if (bracket_level == 0 && delimiters.find(expression[i]) != std::string::npos)
        {
            if (!current_part.empty())
            {
                parts.push_back(core_utils::trim(current_part));
                parts.push_back(std::string(1, expression[i]));
                current_part.clear();
            }
        }
        else
        {
            current_part += expression[i];
        }

        if (expression[i] == ')')
        {
            bracket_level--;

            if (!current_part.empty())
            {
                parts.push_back(core_utils::trim(current_part));
                current_part.clear();
            }
        }
    }
    if (!current_part.empty())
    {
        parts.push_back(core_utils::trim(current_part));
    }

    // process parts
    if (parts.size() == 1)
    {
        return from_string(parts[0].substr(1, parts[0].size() - 2));
    }

    bool negate_next = false;
    operation next_op;
    u32 i = 0;
    while (parts[i] == "!")
    {
        negate_next = !negate_next;
        ++i;
    }

    boolean_function result = from_string(parts[i]);

    while (++i < parts.size())
    {
        if (parts[i] == "!")
        {
            negate_next = !negate_next;
        }
        else if (parts[i] == "&")
        {
            next_op = operation::AND;
        }
        else if (parts[i] == "|")
        {
            next_op = operation::OR;
        }
        else if (parts[i] == "^")
        {
            next_op = operation::XOR;
        }
        else
        {
            result = result.combine(next_op, from_string(parts[i]));
            if (negate_next)
            {
                result = !result;
            }

            negate_next = false;
        }
    }

    return result;
}

std::string boolean_function::to_string() const
{
    std::string result = "X";
    if (m_holds_variable)
    {
        result = m_variable;
    }
    else if (m_holds_constant)
    {
        if (m_constant == X)
        {
            result = "X";
        }
        else if (m_constant == ONE)
        {
            result = "1";
        }
        else
        {
            result = "0";
        }
    }
    else if (!m_operands.empty())
    {
        std::string op_str;
        if (m_op == operation::AND)
        {
            op_str = " & ";
        }
        else if (m_op == operation::OR)
        {
            op_str = " | ";
        }
        else
        {
            op_str = " ^ ";
        }

        std::vector<std::string> terms;
        for (const auto& f : m_operands)
        {
            terms.push_back(f.to_string());
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
    bool holds_expression       = !(m_holds_variable || m_holds_constant);
    bool other_holds_expression = !(other.m_holds_variable || other.m_holds_constant);
    if (holds_expression && m_op == op && !other_holds_expression && !other.m_invert)
    {
        boolean_function result = *this;
        result.m_operands.push_back(other);
        return result;
    }
    else if (other_holds_expression && other.m_op == op && !holds_expression && !m_invert)
    {
        boolean_function result = other;
        result.m_operands.push_back(*this);
        return result;
    }
    else if (holds_expression && other_holds_expression && m_op == op && m_op == other.m_op && !m_invert && !other.m_invert)
    {
        auto joint_operands = m_operands;
        joint_operands.insert(joint_operands.end(), other.m_operands.begin(), other.m_operands.end());
        boolean_function result(op, joint_operands);
        return result;
    }
    else
    {
        return boolean_function(op, {*this, other});
    }
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

boolean_function boolean_function::operator!() const
{
    auto result     = *this;
    result.m_invert = !result.m_invert;
    return result;
}
