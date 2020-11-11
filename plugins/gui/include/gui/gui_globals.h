//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include "hal_core/netlist/netlist.h"

#include "gui/file_status_manager/file_status_manager.h"
#include "gui/graph_widget/graph_context_manager.h"
#include "gui/content_manager/content_manager.h"
#include "gui/netlist_relay/netlist_relay.h"
#include "gui/notifications/notification_manager.h"
#include "gui/plugin_management/plugin_relay.h"
#include "gui/python/python_context.h"
#include "gui/selection_relay/selection_relay.h"
#include "gui/settings/keybind_manager.h"
#include "gui/settings/settings_relay.h"
#include "gui/settings/settings_manager.h"
#include "gui/thread_pool/thread_pool.h"
#include "gui/window_manager/window_manager.h"
#include "gui/gui_api/gui_api.h"

#include <QSettings>

namespace hal
{
    class Netlist;



    extern QSettings* gGuiState;
    extern WindowManager* gWindowManager;
    extern NotificationManager* gNotificationManager;
    extern ContentManager* gContentManager;
    extern std::unique_ptr<Netlist> gNetlistOwner;
    extern Netlist* gNetlist;
    extern NetlistRelay* gNetlistRelay;
    extern PluginRelay* gPluginRelay;
    extern SelectionRelay* gSelectionRelay;
    extern SettingsRelay* gSettingsRelay;
    extern SettingsManager* gSettingsManager;
    extern KeybindManager* gKeybindManager;
    extern FileStatusManager* gFileStatusManager;
    extern GraphContextManager* gGraphContextManager;
    extern ThreadPool* gThreadPool;
    extern std::unique_ptr<PythonContext> gPythonContext;
    extern GuiApi* gGuiApi;

    // Comment this out to not compile the debug code for the graph grid.
    // This will also hide the respective debug setting from the settings page.
    #define GUI_DEBUG_GRID
}
