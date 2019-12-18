#include "netlist/hdl_parser/hdl_parser.h"

hdl_parser::hdl_parser(std::stringstream& stream) : m_fs(stream)
{
    m_netlist = nullptr;
}
