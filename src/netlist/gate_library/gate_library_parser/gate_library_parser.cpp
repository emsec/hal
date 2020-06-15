#include "netlist/gate_library/gate_library_parser/gate_library_parser.h"

namespace hal
{
    gate_library_parser::gate_library_parser(const std::filesystem::path& file_path, std::stringstream& file_content) : m_fs(file_content), m_path(file_path)
    {
        m_gate_lib = nullptr;
    }
}    // namespace hal
