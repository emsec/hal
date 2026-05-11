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

#include "hal_core/plugin_system/gui_extension_interface.h"

namespace hal
{
    /**
     * @class GuiExtensionMediaViewer
     * @brief GUI extension that registers the Media Viewer in the HAL plugin menu.
     */
    class GuiExtensionMediaViewer : public GuiExtensionInterface
    {
    public:
        /** @brief Default constructor. */
        GuiExtensionMediaViewer() : GuiExtensionInterface() {}

        /**
         * @brief Return the list of configurable parameters (none for v1).
         * @returns An empty parameter vector.
         */
        std::vector<PluginParameter> get_parameter() const override;

        /**
         * @brief Apply a set of parameters (no-op for v1).
         * @param params The parameter list (ignored).
         */
        void set_parameter(const std::vector<PluginParameter>& params) override;

        /**
         * @brief Called when a netlist is about to be closed (no-op for v1).
         * @param netlist The netlist being closed.
         */
        void netlist_about_to_close(Netlist* netlist) override;
    };
}    // namespace hal
