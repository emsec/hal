#include "hdl_parser/hdl_parser_verilog.h"

#include "core/log.h"
#include "core/utils.h"

#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/netlist.h"

#include "netlist/netlist_factory.h"

#include <iostream>

//#include <queue>

hdl_parser_verilog::hdl_parser_verilog(std::stringstream& stream) : hdl_parser(stream)
{
}

// ###########################################################################
// ###########          Parse HDL into intermediate format          ##########
// ###########################################################################

std::shared_ptr<netlist> hdl_parser_verilog::parse(const std::string& gate_library)
{
    m_netlist = netlist_factory::create_netlist(gate_library);
    if (m_netlist == nullptr)
    {
        log_error("hdl_parser", "netlist_factory returned nullptr");
        return nullptr;
    }

    // read the whole file into logical parts
    module current_module;

    std::string line, token;

    u32 line_number          = 0;
    i32 token_begin          = -1;
    bool multi_line_comment  = false;
    bool multi_line_property = false;

    while (std::getline(m_fs, line))
    {
        line_number++;

        // remove single-line and multi-line comments
        this->remove_comments(line, multi_line_comment, multi_line_property);

        line = core_utils::trim(line);

        // skip empty lines
        if (line.empty())
        {
            continue;
        }

        // build tokens
        token += line;
        if (token_begin == -1)
        {
            token_begin = line_number;
        }

        // tokens end with ";"
        if (core_utils::ends_with(token, ";"))
        {
            if (core_utils::starts_with(token, "module "))
            {
                // reset current_module
                current_module.name        = token.substr(0, token.find('(')).substr(token.find(' ') + 1);
                current_module.line_number = static_cast<u32>(token_begin);
                current_module.definition.ports.clear();
                current_module.definition.wires.clear();
                current_module.definition.instances.clear();
            }
            else if (core_utils::starts_with(token, "input ") || core_utils::starts_with(token, "output ") || core_utils::starts_with(token, "inout "))
            {
                // collect port declarations
                current_module.definition.ports.push_back(file_line{static_cast<u32>(token_begin), token});
            }
            else if (core_utils::starts_with(token, "wire "))
            {
                // collect wire declarations
                current_module.definition.wires.push_back(file_line{static_cast<u32>(token_begin), token});
            }
            else if (core_utils::starts_with(token, "assign "))
            {
                // collect assign statements
                current_module.definition.assigns.push_back(file_line{static_cast<u32>(token_begin), token});
            }
            else
            {
                // collect instance declarations
                current_module.definition.instances.push_back(file_line{static_cast<u32>(token_begin), token});
            }

            token_begin = -1;
            token.clear();
        }
        else if (core_utils::starts_with(token, "endmodule"))
        {
            // add module to set of modules
            m_modules[current_module.name] = current_module;

            token_begin = -1;
            token.clear();
        }
    }

    if (m_modules.empty())
    {
        log_error("hdl_parser", "file did not contain any modules.");
        return nullptr;
    }

    // parse intermediate format
    for (auto& it : m_modules)
    {
        if (!this->parse_module(it.second))
        {
            return nullptr;
        }
    }

    return nullptr;
}

void hdl_parser_verilog::remove_comments(std::string& line, bool& multi_line_comment, bool& multi_line_property)
{
    bool repeat = true;

    while (repeat)
    {
        repeat = false;

        // skip empty lines
        if (line.empty())
        {
            break;
        }

        auto single_line_comment_begin = line.find("//");
        auto multi_line_comment_begin  = line.find("/*");
        auto multi_line_comment_end    = line.find("*/");
        auto multi_line_property_begin = line.find("(*");
        auto multi_line_property_end   = line.find("*)");

        std::string begin = "";
        std::string end   = "";

        if (multi_line_comment == true)
        {
            if (multi_line_comment_end != std::string::npos)
            {
                // multi-line comment ends in current line
                multi_line_comment = false;
                line               = line.substr(multi_line_comment_end + 2);
                repeat             = true;
            }
            else
            {
                // current line entirely within multi-line comment
                line = "";
                break;
            }
        }
        else if (multi_line_property == true)
        {
            if (multi_line_property_end != std::string::npos)
            {
                // multi-line property ends in current line
                multi_line_property = false;
                line                = line.substr(multi_line_property_end + 2);
                repeat              = true;
            }
            else
            {
                // current line entirely in multi-line property
                line = "";
                break;
            }
        }
        else
        {
            if (single_line_comment_begin != std::string::npos)
            {
                if (multi_line_comment_begin == std::string::npos || (multi_line_comment_begin != std::string::npos && multi_line_comment_begin > single_line_comment_begin))
                {
                    // single-line comment
                    line   = line.substr(0, single_line_comment_begin);
                    repeat = true;
                }
            }
            else if (multi_line_comment_begin != std::string::npos)
            {
                if (multi_line_comment_end != std::string::npos)
                {
                    // multi-line comment entirely in current line
                    line   = line.substr(0, multi_line_comment_begin) + line.substr(multi_line_comment_end + 2);
                    repeat = true;
                }
                else
                {
                    // multi-line comment starts in current line
                    multi_line_comment = true;
                    line               = line.substr(0, multi_line_comment_begin);
                }
            }
            else if (multi_line_property_begin != std::string::npos)
            {
                if (multi_line_property_end != std::string::npos)
                {
                    // multi-line property entirely in current line
                    line   = line.substr(0, multi_line_property_begin) + line.substr(multi_line_property_end + 2);
                    repeat = true;
                }
                else
                {
                    // multi-line property starts in current line
                    multi_line_property = true;
                    line                = line.substr(0, multi_line_property_begin);
                }
            }
        }
    }
}

bool hdl_parser_verilog::parse_module(module& m)
{
    if (!parse_ports(m))
    {
        return false;
    }

    if (!parse_wires(m))
    {
        return false;
    }

    if (!parse_assigns(m))
    {
        return false;
    }

    if (!parse_instances(m))
    {
        return false;
    }

    return true;
}

bool hdl_parser_verilog::parse_ports(module& m)
{
    return false;
}

bool hdl_parser_verilog::parse_wires(module& m)
{
    return false;
}

bool hdl_parser_verilog::parse_assigns(module& m)
{
    return false;
}

bool hdl_parser_verilog::parse_instances(module& m)
{
    return false;
}
