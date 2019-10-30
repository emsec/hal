#include "netlist/boolean_function.h"

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

boolean_function boolean_function::operator&(const boolean_function& other) const
{
    if (m_op == operation::AND && other.m_op == operation::AND && m_invert == other.m_invert)
    {
        auto joint_operands = m_operands;
        joint_operands.insert(joint_operands.end(), other.m_operands.begin(), other.m_operands.end());
        boolean_function result(m_op, joint_operands, m_invert);
        return result;
    }
    else
    {
        return boolean_function(operation::AND, {*this, other});
    }
}

boolean_function boolean_function::operator|(const boolean_function& other) const
{
    if (m_op == operation::OR && other.m_op == operation::OR && m_invert == other.m_invert)
    {
        auto joint_operands = m_operands;
        joint_operands.insert(joint_operands.end(), other.m_operands.begin(), other.m_operands.end());
        boolean_function result(m_op, joint_operands, m_invert);
        return result;
    }
    else
    {
        return boolean_function(operation::OR, {*this, other});
    }
}

boolean_function boolean_function::operator^(const boolean_function& other) const
{
    if (m_op == operation::XOR && other.m_op == operation::XOR && m_invert == other.m_invert)
    {
        auto joint_operands = m_operands;
        joint_operands.insert(joint_operands.end(), other.m_operands.begin(), other.m_operands.end());
        boolean_function result(m_op, joint_operands, m_invert);
        return result;
    }
    else
    {
        return boolean_function(operation::XOR, {*this, other});
    }
}

boolean_function boolean_function::operator!() const
{
    auto result     = *this;
    result.m_invert = !result.m_invert;
    return result;
}
