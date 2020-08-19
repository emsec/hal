#pragma once

#include "common/grouping.h"
#include "def.h"

#include <functional>

namespace hal
{
    namespace processing
    {
        using pass_function = std::function<std::shared_ptr<Grouping>(const std::shared_ptr<Grouping>&)>;
        using pass_id       = u16;

        struct PassConfiguration
        {
            PassConfiguration(const pass_function& func);
            PassConfiguration() = default;

            pass_function function;
            pass_id id;
        };

        namespace pass_collection
        {
            std::vector<PassConfiguration> get_passes(const std::vector<std::vector<pass_id>>& previous_passes);
        }    // namespace pass_collection

    }    // namespace processing
}    // namespace hal