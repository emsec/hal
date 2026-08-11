#pragma once

#include "hal_core/defines.h"

namespace hal
{
    class UIPluginInterface;

    namespace module_identification
    {
        /**
         * Suppresses layout updates of the GUI for as long as the object exists.
         * This keeps a long-running analysis from triggering a re-layout on every single netlist change.
         */
        class GuiLayoutLocker
        {
            UIPluginInterface* mGuiPlugin;

        public:
            GuiLayoutLocker();
            ~GuiLayoutLocker();
        };
    }    // namespace module_identification
}    // namespace hal