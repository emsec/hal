#pragma once

#include "hal_core/defines.h"

#include <string>
#include <vector>

namespace hal
{
    struct Grouping;

    namespace dot_graph
    {
        bool create_graph(const std::shared_ptr<Grouping>& state, const std::string m_path, const std::vector<std::string>& file_types);
    }    // namespace dot_graph
}    // namespace hal
