#include "hal_core/plugin_system/user_feedback.h"

#include "hal_core/plugin_system/plugin_interface_ui.h"
#include "hal_core/plugin_system/plugin_manager.h"

namespace hal
{
    namespace user_feedback
    {
        void report_progress(int percent, const std::string& message)
        {
            if (auto* ui_plugin = plugin_manager::get_ui_plugin(); ui_plugin != nullptr)
            {
                ui_plugin->set_progress(percent, message);
            }
        }

        LayoutLocker::LayoutLocker() : m_ui_plugin(plugin_manager::get_ui_plugin())
        {
            if (m_ui_plugin != nullptr)
            {
                m_ui_plugin->set_layout_locker(true);
            }
        }

        LayoutLocker::~LayoutLocker()
        {
            if (m_ui_plugin != nullptr)
            {
                m_ui_plugin->set_layout_locker(false);
            }
        }

        std::atomic<u32> ProgressScope::s_depth(0);

        ProgressScope::ProgressScope(const std::string& message)
        {
            if (s_depth++ == 0)
            {
                report_progress(0, message);
            }
        }

        ProgressScope::~ProgressScope()
        {
            // the progress display treats 100 percent as "done" and dismisses itself, so it must be reported exactly
            // once per operation
            if (--s_depth == 0)
            {
                report_progress(100, "done");
            }
        }
    }    // namespace user_feedback
}    // namespace hal
