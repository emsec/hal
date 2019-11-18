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

token_stream::token_stream(const std::vector<std::string>& increase_level_tokens, const std::vector<std::string>& decrease_level_tokens)
{
    m_pos                   = 0;
    m_increase_level_tokens = increase_level_tokens;
    m_decrease_level_tokens = decrease_level_tokens;
}

token_stream::token_stream(const token_stream& other)
{
    m_pos                   = other.m_pos;
    m_data                  = other.m_data;
    m_increase_level_tokens = other.m_increase_level_tokens;
    m_decrease_level_tokens = other.m_decrease_level_tokens;
}

token_stream::token_stream(const std::vector<token>& init, const std::vector<std::string>& increase_level_tokens, const std::vector<std::string>& decrease_level_tokens)
    : token_stream(increase_level_tokens, decrease_level_tokens)
{
    m_data = init;
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

bool token_stream::consume(const std::string& expected, bool throw_on_error)
{
    if (m_pos >= size())
    {
        if (throw_on_error)
        {
            throw std::invalid_argument("expected token '" + expected + "' but reached the end of the stream");
        }
        return false;
    }

    if (at(m_pos) != expected)
    {
        if (throw_on_error)
        {
            throw std::invalid_argument("expected token '" + expected + "' but got '" + at(m_pos).string + "'");
        }
        return false;
    }

    m_pos++;
    return true;
}

token token_stream::consume_until(const std::string& match, u32 end, bool level_aware)
{
    m_pos = std::min(size(), find_next(match, end, level_aware));
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

u32 token_stream::find_next(const std::string& match, u32 end, bool level_aware) const
{
    u32 level = 0;
    for (u32 i = m_pos; i < size() && i < end; ++i)
    {
        const auto& token = at(i);
        if ((!level_aware || level == 0) && core_utils::equals_ignore_case(token, match))
        {
            return i;
        }
        else if (level_aware && std::find_if(m_increase_level_tokens.begin(), m_increase_level_tokens.end(), [&token](const auto& x) { return token == x; }) != m_increase_level_tokens.end())
        {
            level++;
        }
        else if (level_aware && level > 0
                 && std::find_if(m_decrease_level_tokens.begin(), m_decrease_level_tokens.end(), [&token](const auto& x) { return token == x; }) != m_decrease_level_tokens.end())
        {
            level--;
        }
    }
    return end;
}

token token_stream::join_until(const std::string& match, const std::string& joiner, u32 end, bool level_aware)
{
    u32 start_line = at(m_pos).number;
    std::string result;
    auto end_pos = std::min(size(), find_next(match, end, level_aware));
    while (m_pos < end_pos)
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

token_stream token_stream::extract_until(const std::string& match, u32 end, bool level_aware)
{
    auto end_pos = std::min(size(), find_next(match, end, level_aware));
    token_stream res(m_increase_level_tokens, m_decrease_level_tokens);
    res.m_data.insert(res.m_data.begin(), m_data.begin() + m_pos, m_data.begin() + end_pos);
    m_pos = end_pos;
    return res;
}
