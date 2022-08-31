#include "dataflow_analysis/utils/gui_layout_locker.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_core/plugin_system/plugin_interface_ui.h"

namespace hal
{
    namespace dataflow
    {
        GuiLayoutLocker::GuiLayoutLocker()
            : mGuiPlugin(nullptr)
        {
            mGuiPlugin = plugin_manager::get_plugin_instance<UIPluginInterface>("hal_gui");
            if (mGuiPlugin)
                mGuiPlugin->set_layout_locker(true);
        }

        GuiLayoutLocker::~GuiLayoutLocker()
        {
            if (mGuiPlugin)
                mGuiPlugin->set_layout_locker(false);
        }
    }
}
