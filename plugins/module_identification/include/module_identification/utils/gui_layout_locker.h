#pragma once

#include "hal_core/defines.h"

namespace hal
{
    class UIPluginInterface;

    namespace module_identification
    {
        class GuiLayoutLocker
        {
            UIPluginInterface* mGuiPlugin;

        public:
            GuiLayoutLocker();
            ~GuiLayoutLocker();
        };
    }    // namespace module_identification
}    // namespace hal