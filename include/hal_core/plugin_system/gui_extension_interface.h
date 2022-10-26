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
#include <vector>
#include <functional>
#include "hal_core/defines.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/plugin_system/plugin_parameter.h"

namespace hal {
    class GuiExtensionInterface;

    struct ContextMenuContribution
    {
        GuiExtensionInterface* mContributer;
        u32 mFunctionId;
        std::string mEntry;
    };

    class GuiExtensionInterface
    {
    public:
        GuiExtensionInterface() {;}
        virtual ~GuiExtensionInterface() {;}

        /**
         * Get list of configurable parameter
         * @return List of configurable parameter
         */
        virtual std::vector<PluginParameter> get_parameter() const;

        /**
         * Set values for configurable parameter
         * @param[in] nl The current netlist
         * @param[in] params List of configurable parameter with values
         */
        virtual void set_parameter(Netlist* nl, const std::vector<PluginParameter>& params);

        /**
         * Register function to indicate work progress when busy
         * @param pif Progress Indicator Function to register
         */
        virtual void register_progress_indicator(std::function<void(int, const std::string&)> pif);

        virtual std::vector<ContextMenuContribution> get_context_contribution(const Netlist* nl,
                                                                              const std::vector<u32>& mods,
                                                                              const std::vector<u32>& gats,
                                                                              const std::vector<u32>& nets);

        virtual void execute_context_action(u32 fid,
                                            Netlist* nl,
                                            const std::vector<u32>& mods,
                                            const std::vector<u32>& gats,
                                            const std::vector<u32>& nets);

    };
}
