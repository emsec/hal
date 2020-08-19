#pragma once

#include "def.h"
#include "processing/pass_collection.h"
#include "processing/result.h"

#include <map>
#include <shared_mutex>

namespace hal
{
    /* forward declaration */
    struct PipelineConfiguration;
    struct NetlistAbstraction;

    namespace processing
    {
        struct Context
        {
            /* pass process */
            bool done;
            bool end_reached;
            u32 phase;

            u32 pass_counter;
            u32 finished_passes;
            u32 num_iterations;

            // std::shared_ptr<grouping> main_state;
            // std::shared_ptr<grouping> control_state;

            /* helpers */
            std::vector<std::pair<std::shared_ptr<Grouping>, PassConfiguration>> current_passes;
            std::map<std::pair<std::shared_ptr<Grouping>, pass_id>, std::shared_ptr<Grouping>> pass_outcome;
            std::vector<std::tuple<std::shared_ptr<Grouping>, pass_id, std::shared_ptr<Grouping>>> new_unique_groupings;
            std::vector<std::tuple<std::shared_ptr<Grouping>, pass_id, std::shared_ptr<Grouping>>> new_recurring_results;

            /* progress printing */
            std::mutex progress_mutex;

            /* result */
            std::mutex result_mutex;

            processing::Result result;
        };

    }    // namespace processing
}    // namespace hal