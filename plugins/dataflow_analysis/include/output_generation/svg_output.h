#pragma once

#include "def.h"
#include "evaluation/result.h"
#include "output_generation/json.hpp"
#include "processing/result.h"

namespace hal
{
    namespace svg_output
    {
        void save_state_to_svg(const std::shared_ptr<Grouping>& state, const std::string m_path, const std::string file_namey);
    }    // namespace svg_output
}    // namespace hal