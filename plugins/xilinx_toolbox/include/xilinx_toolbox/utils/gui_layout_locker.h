// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/**
 * @file gui_layout_locker.h
 * @brief This file contains a helper that suppresses layout updates of the GUI.
 */

#pragma once

#include "hal_core/defines.h"

namespace hal
{
    class UIPluginInterface;

    namespace xilinx_toolbox
    {
        /**
         * @class GuiLayoutLocker
         * @brief Suppresses layout updates of the GUI for as long as the object exists.
         *
         * A netlist modification that touches many gates makes the GUI re-layout its graph views once per change,
         * which can dominate the runtime of the modification itself. Holding a locker defers those updates until it
         * goes out of scope, at which point every affected view is updated once.
         *
         * Locks nest, so it is safe to hold more than one at a time. Does nothing if no GUI is running, which makes
         * it safe to use from code that also runs headless.
         */
        class GuiLayoutLocker
        {
        public:
            /**
             * @brief Suppress layout updates of the GUI until the locker is destroyed.
             */
            GuiLayoutLocker();

            /**
             * @brief Release the lock and let the GUI update the views that changed in the meantime.
             */
            ~GuiLayoutLocker();

            GuiLayoutLocker(const GuiLayoutLocker&)            = delete;
            GuiLayoutLocker& operator=(const GuiLayoutLocker&) = delete;

        private:
            UIPluginInterface* m_gui_plugin;
        };
    }    // namespace xilinx_toolbox
}    // namespace hal
