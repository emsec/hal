#include "hdl_parser/token_stream.h"
#include "core/utils.h"

token::token(u32 n, const std::string s, bool cs) : number(n), string(s), case_sensitive(cs)
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
    if (!case_sensitive)
    {
        return core_utils::equals_ignore_case(string, s);
    }
    return string == s;
}

bool token::operator!=(const std::string& s) const
{
    return !(*this == s);
}

///////////////////////////////////////
///////////////////////////////////////

token_stream::token_stream()
{
    m_pos = 0;
}

token_stream::token_stream(const token_stream& other)
{
    m_pos  = other.m_pos;
    m_data = other.m_data;
}

token_stream::token_stream(const std::vector<token>& init) : token_stream()
{
    m_data = init;
}

void token_stream::append(const token& t)
{
    m_data.push_back(t);
}

token& token_stream::at(u32 position)
{
    return m_data.at(position);
}
const token& token_stream::at(u32 position) const
{
    return m_data.at(position);
}

token& token_stream::peek(i32 offset)
{
    return at(m_pos + offset);
}
const token& token_stream::peek(i32 offset) const
{
    return at(m_pos + offset);
}

token token_stream::consume(u32 num)
{
    m_pos += num;
    return at(m_pos - 1);
}

bool token_stream::consume(const std::string& expected)
{
    if (m_pos < size() && at(m_pos) == expected)
    {
        m_pos++;
        return true;
    }
    else
    {
        return false;
    }
}

token token_stream::consume_until(const std::string& match)
{
    m_pos = find_next(match);
    return at(m_pos - 1);
}

u32 token_stream::size() const
{
    return m_data.size();
}
u32 token_stream::consumed() const
{
    return m_pos;
}
u32 token_stream::remaining() const
{
    return size() - consumed();
}

u32 token_stream::position() const
{
    return m_pos;
}
void token_stream::set_position(u32 p)
{
    m_pos = p;
}

u32 token_stream::find_next(const std::string& match, u32 end) const
{
    u32 level = 0;
    for (u32 i = m_pos; i < size() && i < end; ++i)
    {
        const auto& token = at(i);
        if (level == 0 && core_utils::equals_ignore_case(token, match))
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

token token_stream::join_until(const std::string& match, const std::string& joiner)
{
    u32 start_line = at(m_pos).number;
    std::string result;
    auto end_pos = find_next(match);
    while (m_pos < end_pos && m_pos < size())
    {
        if (!result.empty())
        {
            result += joiner;
        }
        result += consume();
    }
    return {start_line, result};
}

token token_stream::join(const std::string& joiner)
{
    u32 start_line = at(m_pos).number;
    std::string result;
    while (remaining() > 0)
    {
        if (!result.empty())
        {
            result += joiner;
        }
        result += consume();
    }
    return {start_line, result};
}

token_stream token_stream::extract_until(const std::string& match)
{
    auto end_pos = std::min(size(), find_next(match));
    token_stream res;
    res.m_data.insert(res.m_data.begin(), m_data.begin() + m_pos, m_data.begin() + end_pos);
    m_pos = end_pos;
    return res;
}
