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

#pragma once

#include "hal_core/defines.h"

#include <atomic>
#include <string>

namespace hal
{
    class UIPluginInterface;

    /**
     * Feedback that a long-running operation gives to the user interface, such as its progress or the fact that it is
     * about to change the netlist a lot.
     *
     * The user interface is looked up at runtime through `plugin_manager::get_ui_plugin`, so all of this does nothing
     * when running from a script or from the command line, and no plugin needs to depend on the user interface to use
     * it. In particular, a plugin does not need to provide a `GuiExtensionInterface` just to report progress.
     *
     * @ingroup plugins
     */
    namespace user_feedback
    {
        /**
         * Report the progress of a long-running operation to the user interface.
         *
         * A percentage of 100 dismisses the progress display again and must therefore be reported exactly once per
         * operation, which is what `ProgressScope` is for. Prefer that over calling this directly.
         *
         * @param[in] percent - The progress in percent, where 100 means done.
         * @param[in] message - The message to display alongside the progress.
         */
        void report_progress(int percent, const std::string& message);

        /**
         * Suppresses layout updates of the user interface for as long as the object exists.
         *
         * This keeps an operation that changes many gates or nets from triggering a re-layout of the graph views on
         * every single change, which would otherwise dominate its runtime. Lockers may be nested, the layout is
         * updated again once the last one is destroyed.
         */
        class LayoutLocker
        {
        public:
            LayoutLocker();
            ~LayoutLocker();

            LayoutLocker(const LayoutLocker&)            = delete;
            LayoutLocker& operator=(const LayoutLocker&) = delete;

        private:
            /** The user interface whose layout updates are suppressed, `nullptr` if none is loaded. */
            UIPluginInterface* m_ui_plugin;
        };

        /**
         * Brackets a long-running operation in the progress display of the user interface.
         *
         * Shows the display on construction and dismisses it on destruction, so that it is dismissed no matter how the
         * operation was invoked and no matter how it returned. Scopes may be nested, in which case only the outermost
         * one takes effect: an operation invoked from the user interface thereby keeps the display up while its caller
         * is still post-processing the results.
         */
        class ProgressScope
        {
        public:
            /**
             * Construct a progress scope and show the progress display at zero percent.
             *
             * @param[in] message - The message to display alongside the progress.
             */
            explicit ProgressScope(const std::string& message);

            ~ProgressScope();

            ProgressScope(const ProgressScope&)            = delete;
            ProgressScope& operator=(const ProgressScope&) = delete;

        private:
            static std::atomic<u32> s_depth;
        };
    }    // namespace user_feedback
}    // namespace hal
