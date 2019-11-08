#include "hdl_parser/token_stream.h"
#include "core/utils.h"

token::token(u32 n, const std::string s) : number(n), string(s)
{
}

token::operator std::string() const
{
    return string;
}

token& token::operator=(const std::string& s)
{
    this->string = s;
    return *this;
}

token& token::operator+=(const std::string& s)
{
    this->string += s;
    return *this;
}

bool token::operator==(const std::string& s) const
{
    return core_utils::equals_ignore_case(string, s);
}

bool token::operator!=(const std::string& s) const
{
    return !(*this == s);
}

token_stream::token_stream(bool case_sensitive)
{
    m_case_sensitive = case_sensitive;
    m_pos            = 0;
}

token& token_stream::peek(u32 offset = 0)
{
    return at(m_pos + offset);
}
const token& token_stream::peek(u32 offset = 0) const
{
    return at(m_pos + offset);
}

token token_stream::consume(u32 num = 1)
{
    m_pos += num;
    return at(m_pos - 1);
}

bool token_stream::consume(const std::string& expected)
{
    if (at(m_pos) == expected)
    {
        m_pos++;
        return true;
    }
    else
    {
        return false;
    }
}

token token_stream::consume_until(const std::string& end)
{
    return consume_until(find_next(end));
}

token token_stream::consume_until(u32 end)
{
    m_pos = end;
    return at(m_pos - 1);
}

bool token_stream::consumed() const
{
    return m_pos >= size();
}

u32 token_stream::position() const
{
    return m_pos;
}
void token_stream::set_position(u32 p)
{
    m_pos = p;
}

u32 token_stream::find_next(const std::string& end_marker) const
{
    u32 level = 0;
    for (u32 i = m_pos; i < size(); ++i)
    {
        const auto& token = at(i);
        if (level == 0 && core_utils::equals_ignore_case(token, end_marker))
        {
            return i;
        }
        else if (token == "(" || token == "[")
        {
            level++;
        }
        else if (token == ")" || token == "]")
        {
            level--;
        }
    }
    return not_found;
}

token token_stream::join_until(const std::string& end, const std::string& joiner)
{
    return join_until(find_next(end), joiner);
}

token token_stream::join_until(u32 end, const std::string& joiner)
{
    u32 start_line = at(m_pos).number;
    std::string result;
    while (m_pos < end)
    {
        if (!result.empty())
        {
            result += " ";
        }
        result += consume();
    }
    return {start_line, result};
}

token token_stream::join(const std::string& joiner)
{
    std::string result;
    for (const auto& x : *this)
    {
        if (!result.empty())
        {
            result += joiner;
        }
        result += x;
    }
    return {at(0).number, result};
}

token_stream token_stream::extract_until(const std::string& end)
{
    return extract_until(find_next(end));
}

token_stream token_stream::extract_until(u32 end_pos)
{
    if (end_pos > size())
    {
        token_stream res(m_case_sensitive);
        res.insert(res.begin(), begin() + m_pos, end());
        m_pos = size();
        return res;
    }
    else
    {
        token_stream res(m_case_sensitive);
        res.insert(res.begin(), begin() + m_pos, begin() + end_pos);
        m_pos = end_pos;
        return res;
    }
}
