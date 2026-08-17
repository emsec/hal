#include "dataflow_analysis/utils/progress_scope.h"

#include "dataflow_analysis/plugin_dataflow.h"

namespace hal
{
    namespace dataflow
    {
        std::atomic<u32> ProgressScope::s_depth(0);

        ProgressScope::ProgressScope(const std::string& message)
        {
            if (s_depth++ == 0 && GuiExtensionDataflow::s_progress_indicator_function)
            {
                GuiExtensionDataflow::s_progress_indicator_function(0, message);
            }
        }

        ProgressScope::~ProgressScope()
        {
            // the indicator treats 100 percent as "done" and dismisses itself, so it must be reported exactly once
            if (--s_depth == 0 && GuiExtensionDataflow::s_progress_indicator_function)
            {
                GuiExtensionDataflow::s_progress_indicator_function(100, "dataflow analysis finished");
            }
        }
    }    // namespace dataflow
}    // namespace hal
