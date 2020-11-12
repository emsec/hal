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

    class NetlistRelay : public QObject
    {
        Q_OBJECT

    public:
        explicit NetlistRelay(QObject* parent = nullptr);
        ~NetlistRelay();

        void registerCallbacks();

        QColor getModuleColor(const u32 id);

        ModuleModel* getModuleModel();

        void debugChangeModuleName(const u32 id);
        void debugChangeModuleType(const u32 id);
        void debugChangeModuleColor(const u32 id);
        void debugAddSelectionToModule(const u32 id);
        void debugAddChildModule(const u32 id);
        void debugDeleteModule(const u32 id);

    Q_SIGNALS:
        // PROBABLY OBSOLETE
        void netlistEvent(netlist_event_handler::event ev, Netlist* object, u32 associated_data) const;
        void moduleEvent(module_event_handler::event ev, Module* object, u32 associated_data) const;
        void gateEvent(gate_event_handler::event ev, Gate* object, u32 associated_data) const;
        void netEvent(net_event_handler::event ev, Net* object, u32 associated_data) const;

        void netlistIdChanged(Netlist* n, const u32 associated_data) const;
        void netlistInputFilenameChanged(Netlist* n) const;
        void netlistDesignNameChanged(Netlist* n) const;
        void netlistDeviceNameChanged(Netlist* n) const;
        void netlistMarkedGlobalVcc(Netlist* n, const u32 associated_data) const;
        void netlistMarkedGlobalGnd(Netlist* n, const u32 associated_data) const;
        void netlistUnmarkedGlobalVcc(Netlist* n, const u32 associated_data) const;
        void netlistUnmarkedGlobalGnd(Netlist* n, const u32 associated_data) const;
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

    private Q_SLOTS:
        void debugHandleFileOpened();
        void debugHandleFileClosed();

    private:
        void relayNetlistEvent(netlist_event_handler::event ev, Netlist* object, u32 associated_data);
        void relayModuleEvent(module_event_handler::event ev, Module* object, u32 associated_data);
        void relayGateEvent(gate_event_handler::event ev, Gate* object, u32 associated_data);
        void relayNetEvent(net_event_handler::event ev, Net* object, u32 associated_data);
        void relayGroupingEvent(grouping_event_handler::event ev, Grouping* object, u32 associated_data);

        QMap<u32, QColor> mModuleColors;

        QMap<u32, QString> mGateAliases;
        QMap<u32, QString> mNetAliases;

        ModuleModel* mModuleModel;
    };
}
