#include "netlist/gate_library/gate_library_parser/gate_library_parser.h"

gate_library_parser::gate_library_parser(std::stringstream& stream) : m_fs(stream)
{
    m_gate_lib = nullptr;
}
