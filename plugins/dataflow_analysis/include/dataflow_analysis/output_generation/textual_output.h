#pragma once

#include "hal_core/defines.h"

namespace hal
{
    /* forward declaration */
    struct Grouping;

    namespace textual_output
    {
        void write_register_output(const std::shared_ptr<Grouping>& state, const std::string m_path, const std::string file_name);
    }    // namespace textual_output
}    // namespace hal
