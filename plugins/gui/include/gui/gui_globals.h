//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
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
#include "gui/plugin_relay/plugin_relay.h"
#include "gui/python/python_context.h"
#include "gui/selection_relay/selection_relay.h"
#include "gui/gui_api/gui_api.h"

#include <QSettings>

namespace hal
{
    class Netlist;

    /** @name GUI Globals
     * Interface to access the globals of the gui defined in src/plugin_gui.cpp
     */
    ///@{
    /**
     * Is used to persist the list of recent files and the previously opened file in the 'open file' dialog
     */
    extern QSettings* gGuiState;

    /**
     * The global ContentManager (see doc)
     */
    extern ContentManager* gContentManager;

    /**
     * This shared_ptr contains and owns the netlist after a file was opened. To access the Netlist data structure with
     * its functions, it is intended to use the raw pointer gNetlist instead.
     */
    extern std::shared_ptr<Netlist> gNetlistOwner;

    /**
     * The netlist that is currently loaded. It is a pointer to the netlist owned by gNetlistOwner.
     */
    extern Netlist* gNetlist;

    /**
     * The global NetlistRelay (see doc)
     */
    extern NetlistRelay* gNetlistRelay;

    /**
     * The global PluginRelay. Plugin management is not available in this version. Therefore it is currently unused.
     */
    extern PluginRelay* gPluginRelay;

    /**
     * The global SelectionRelay (see doc)
     */
    extern SelectionRelay* gSelectionRelay;

    /**
     * The global FileStatusManager (see doc)
     */
    extern FileStatusManager* gFileStatusManager;

    /**
     * The global GraphContextManager (see doc)
     */
    extern GraphContextManager* gGraphContextManager;

    /**
     * The global PythonContext (see doc)
     */
    extern std::unique_ptr<PythonContext> gPythonContext;

    /**
     * The global GuiApi (see doc)
     */
    extern GuiApi* gGuiApi;

    ///@}

    // Comment this out to not compile the debug code for the graph grid.
    // This will also hide the respective debug setting from the settings page.
    #define GUI_DEBUG_GRID
}
