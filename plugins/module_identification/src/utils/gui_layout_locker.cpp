#include "module_identification/utils/gui_layout_locker.h"

#include "hal_core/plugin_system/plugin_interface_ui.h"
#include "hal_core/plugin_system/plugin_manager.h"

namespace hal
{
    namespace module_identification
    {
        GuiLayoutLocker::GuiLayoutLocker() : mGuiPlugin(nullptr)
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
    }    // namespace module_identification
}    // namespace hal