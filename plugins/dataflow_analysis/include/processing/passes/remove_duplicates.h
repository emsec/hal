#pragma once

#include "def.h"

namespace hal
{
    /* forward declaration */
    struct Grouping;

    namespace remove_duplicates
    {
        std::shared_ptr<Grouping> process(const std::shared_ptr<Grouping>& state, bool delete_from_smaller);

    }    // namespace remove_duplicates
}    // namespace hal