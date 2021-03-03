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

#include "hal_core/netlist/event_system/gate_event_handler.h"
#include "hal_core/netlist/event_system/net_event_handler.h"
#include "hal_core/netlist/event_system/netlist_event_handler.h"
#include "hal_core/netlist/event_system/module_event_handler.h"
#include "hal_core/netlist/event_system/grouping_event_handler.h"

#include <QMap>
#include <QObject>

namespace hal
{
    class ModuleItem;
    class ModuleModel;
    class Module;

    /**
     * The NetlistRelay is used to capture events from the hal-core and to delegate their handling to various gui objects.
     * These objects may connect to one of the numerous Q_SIGNALS provided by this class to implement an individual
     * reaction to the registered events.
     *
     * Also the NetlistRelay stores the color for modules wich can accessed via NetlistRelay::getModuleColor
     */
    class NetlistRelay : public QObject
    {
        Q_OBJECT

    public:
        /**
         * Constructor. Already connects this relay with the event system of the hal-core.
         *
         * @param parent - The parent QObject
         */
        explicit NetlistRelay(QObject* parent = nullptr);

        /**
         * Destructor. Disconnects this relay from the event system of the hal-core.
         */
        ~NetlistRelay();

        /**
         * TODO: Why not private?
         * Adds the relay functions of the NetlistRelay as callbacks for hal-core events.
         */
        void registerCallbacks();

        /**
         * Gets the color that is assigned to a module.
         *
         * @param id - The id of the module
         * @returns the color of the specified module
         */
        QColor getModuleColor(const u32 id);

        /**
         * Accesses the module model.
         *
         * @returns the module model
         */
        ModuleModel* getModuleModel();

        // TODO: Why named "debug..."?
        /**
         * Changes the name of a specific module by asking the user for a new name in a 'Rename'-dialogue.
         *
         * @param id - The id of the module to rename
         */
        void debugChangeModuleName(const u32 id);

        /**
         * Changes the type of a specific module by asking the user for a new name in a 'New Type'-dialogue.
         *
         * @param id - The id of the module whose type is to be changed
         */
        void debugChangeModuleType(const u32 id);

        /**
         * Changes the type of a specific module by asking the user to select a new color in a color dialogue.
         *
         * @param id - The id of the module whose color is to be changed
         */
        void debugChangeModuleColor(const u32 id);

        /**
         * TODO: Only works for gates. Modules were ignored...
         * Adds the current selection to a specific module.
         *
         * @param id - The id of the module that should be appended
         */
        void debugAddSelectionToModule(const u32 id);

        /**
         * Adds an empty child module to the specified module.
         *
         * @param id - The id of the module that becomes the parent of the empty child module
         */
        void debugAddChildModule(const u32 id);

        /**
         * Deletes the specified module from the netlist.
         *
         * @param id - The id of the module to remove
         */
        void debugDeleteModule(const u32 id);

    Q_SIGNALS:
        // PROBABLY OBSOLETE
        /**
         * TODO: Unused (Never Emitted)
         */
        void netlistEvent(netlist_event_handler::event ev, Netlist* object, u32 associated_data) const;

        /**
         * TODO: Unused (Never Emitted)
         */
        void moduleEvent(module_event_handler::event ev, Module* object, u32 associated_data) const;

        /**
         * TODO: Unused (Never Emitted)
         */
        void gateEvent(gate_event_handler::event ev, Gate* object, u32 associated_data) const;

        /**
         * TODO: Unused (Never Emitted)
         */
        void netEvent(net_event_handler::event ev, Net* object, u32 associated_data) const;


        /**
         * Q_SIGNAL to notify that the netlists id has been changed. <br>
         * Relays the following hal-core event: <i>netlist_event_handler::event::id_changed</i>
         *
         * @param n - The affected netlist
         * @param associated_data - The old netlist id
         */
        void netlistIdChanged(Netlist* n, const u32 associated_data) const;

        /**
         * Q_SIGNAL to notify that the input filename of the netlist has been changed. <br>
         * Relays the following hal-core event: <i>netlist_event_handler::event::input_filename_changed</i>
         *
         * @param n - The affected netlist
         */
        void netlistInputFilenameChanged(Netlist* n) const;

        /**
         * Q_SIGNAL to notify that the design name of the netlist has been changed. <br>
         * Relays the following hal-core event: <i>netlist_event_handler::event::design_name_changed</i>
         *
         * @param n - The affected netlist
         */
        void netlistDesignNameChanged(Netlist* n) const;

        /**
         * Q_SIGNAL to notify that the device name of the netlist has been changed. <br>
         * Relays the following hal-core event: <i>netlist_event_handler::event::device_name_changed</i>
         *
         * @param n - The affected netlist
         */
        void netlistDeviceNameChanged(Netlist* n) const;

        /**
         * Q_SIGNAL to notify that a gate has been marked as a global vcc gate. <br>
         * Relays the following hal-core event: <i>netlist_event_handler::event::marked_global_vcc</i>
         *
         * @param n - The affected netlist
         * @param associated_data - The id of the gate that has been marked as a global vcc gate
         */
        void netlistMarkedGlobalVcc(Netlist* n, const u32 associated_data) const;

        /**
         * Q_SIGNAL to notify that a gate has been marked as a global gnd gate. <br>
         * Relays the following hal-core event: <i>netlist_event_handler::event::marked_global_gnd</i>
         *
         * @param n - The affected netlist
         * @param associated_data - The id of the gate that has been marked as a global gnd gate
         */
        void netlistMarkedGlobalGnd(Netlist* n, const u32 associated_data) const;

        /**
         * Q_SIGNAL to notify that a gate has been unmarked from beeing a global vcc gate. <br>
         * Relays the following hal-core event: <i>netlist_event_handler::event::unmarked_global_vcc</i>
         *
         * @param n - The affected netlist
         * @param associated_data - The id of the gate that is not a global vcc gate anymore
         */
        void netlistUnmarkedGlobalVcc(Netlist* n, const u32 associated_data) const;

        /**
         * Q_SIGNAL to notify that a gate has been unmarked from beeing a global gnd gate. <br>
         * Relays the following hal-core event: <i>netlist_event_handler::event::unmarked_global_gnd</i>
         *
         * @param n - The affected netlist
         * @param associated_data - The id of the gate that is not a global gnd gate anymore
         */
        void netlistUnmarkedGlobalGnd(Netlist* n, const u32 associated_data) const;

        /**
         * IN_PROGRESS: Further Documentation
         * @param n
         * @param associated_data
         */
        void netlistMarkedGlobalInput(Netlist* n, const u32 associated_data) const;
        void netlistMarkedGlobalOutput(Netlist* n, const u32 associated_data) const;
        void netlistMarkedGlobalInout(Netlist* n, const u32 associated_data) const;
        void netlistUnmarkedGlobalInput(Netlist* n, const u32 associated_data) const;
        void netlistUnmarkedGlobalOutput(Netlist* n, const u32 associated_data) const;
        void netlistUnmarkedGlobalInout(Netlist* n, const u32 associated_data) const;

        void moduleCreated(Module* m) const;
        void module_removed(Module* m) const;
        void moduleNameChanged(Module* m) const;
        void moduleParentChanged(Module* m) const;
        void moduleSubmoduleAdded(Module* m, const u32 added_module) const;
        void moduleSubmoduleRemoved(Module* m, const u32 removed_module) const;
        void moduleGateAssigned(Module* m, const u32 assigned_gate) const;
        void moduleGateRemoved(Module* m, const u32 removed_gate) const;
        void moduleInputPortNameChanged(Module* m, const u32 respective_net) const;
        void moduleOutputPortNameChanged(Module* m, const u32 respective_net) const;
        void moduleTypeChanged(Module* m) const;

        void gateCreated(Gate* g) const;
        void gate_removed(Gate* g) const;
        void gateNameChanged(Gate* g) const;

        void netCreated(Net* n) const;
        void net_removed(Net* n) const;
        void netNameChanged(Net* n) const;
        void netSourceAdded(Net* n, const u32 dst_gate_id) const;
        void netSourceRemoved(Net* n, const u32 dst_gate_id) const;
        void netDestinationAdded(Net* n, const u32 dst_gate_id) const;
        void netDestinationRemoved(Net* n, const u32 dst_gate_id) const;

        void groupingCreated(Grouping* grp) const;
        void groupingRemoved(Grouping* grp) const;
        void groupingNameChanged(Grouping* grp) const;
        void groupingGateAssigned(Grouping* grp, u32 id) const;
        void groupingGateRemoved(Grouping* grp, u32 id) const;
        void groupingNetAssigned(Grouping* grp, u32 id) const;
        void groupingNetRemoved(Grouping* grp, u32 id) const;
        void groupingModuleAssigned(Grouping* grp, u32 id) const;
        void groupingModuleRemoved(Grouping* grp, u32 id) const;

        // GUI
        void moduleColorChanged(Module* m) const;

    public Q_SLOTS:
        void debugHandleFileOpened();
        void debugHandleFileClosed();

    private:
        void relayNetlistEvent(netlist_event_handler::event ev, Netlist* object, u32 associated_data);
        void relayModuleEvent(module_event_handler::event ev, Module* object, u32 associated_data);
        void relayGateEvent(gate_event_handler::event ev, Gate* object, u32 associated_data);
        void relayNetEvent(net_event_handler::event ev, Net* object, u32 associated_data);
        void relayGroupingEvent(grouping_event_handler::event ev, Grouping* object, u32 associated_data);

        QMap<u32, QColor> mModuleColors;

        // TODO: Unused. Can be removed?
        QMap<u32, QString> mGateAliases;
        QMap<u32, QString> mNetAliases;

        ModuleModel* mModuleModel;
    };
}
