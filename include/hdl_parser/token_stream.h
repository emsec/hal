//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#include "pragma_once.h"
#ifndef __HAL_TOKE_STREAM_H__
#define __HAL_TOKE_STREAM_H__

#include "def.h"

#include <map>

/**
 * @ingroup hdl_parsers
 */

struct token
{
    token(u32 n, const std::string s);

    u32 number;
    std::string string;

    operator std::string() const;

    token& operator=(const std::string& s);
    token& operator+=(const std::string& s);

    bool operator==(const std::string& s) const;
    bool operator!=(const std::string& s) const;
};

class HDL_PARSER_API token_stream : public std::vector<token>
{
public:
    static const u32 not_found = -1;

    token_stream(bool case_sensitive = false);

    token& peek(u32 offset = 0);
    const token& peek(u32 offset = 0) const;

    token consume(u32 num = 1);
    bool consume(const std::string& expected);
    token consume_until(const std::string& end);
    token consume_until(u32 end);

    bool consumed() const;

    u32 position() const;
    void set_position(u32 p);

    u32 find_next(const std::string& end) const;

    token join_until(const std::string& end,const std::string& joiner = " ");
    token join_until(u32 end, const std::string& joiner = " ");
    token join(const std::string& joiner = " ");

    token_stream extract_until(const std::string& end);
    token_stream extract_until(u32 end);

private:
    u32 m_pos;
    bool m_case_sensitive;
};

#endif /* __HAL_HDL_PARSER_VHDL_H__ */
