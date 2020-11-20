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

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/module.h"


#include <vector>
#include <tuple>

#include <QObject>

namespace hal
{
    class GuiApi : public QObject
    {
        Q_OBJECT

    public:
        GuiApi();

        std::vector<u32> getSelectedGateIds();
        std::vector<u32> getSelectedNetIds();
        std::vector<u32> getSelectedModuleIds();
        std::tuple<std::vector<u32>, std::vector<u32>, std::vector<u32>> getSelectedItemIds();

        std::vector<Gate*> getSelectedGates();
        std::vector<Net*> getSelectedNets();
        std::vector<Module*> getSelectedModules();
        std::tuple<std::vector<Gate*>, std::vector<Net*>, std::vector<Module*>> getSelectedItems();

        void selectGate(u32 gate_id, bool clear_current_selection = true, bool navigate_to_selection = true);
        void selectGate(const std::vector<u32>& gate_ids, bool clear_current_selection = true, bool navigate_to_selection = true);
        void selectGate(Gate* gate, bool clear_current_selection = true, bool navigate_to_selection = true);
        void selectGate(const std::vector<Gate*>& gates, bool clear_current_selection = true, bool navigate_to_selection = true);

        void selectNet(u32 mNetId, bool clear_current_selection = true, bool navigate_to_selection = true);
        void selectNet(const std::vector<u32>& net_ids, bool clear_current_selection = true, bool navigate_to_selection = true);
        void selectNet(Net* net, bool clear_current_selection = true, bool navigate_to_selection = true);
        void selectNet(const std::vector<Net*>& nets, bool clear_current_selection = true, bool navigate_to_selection = true);

        void selectModule(u32 module_id, bool clear_current_selection = true, bool navigate_to_selection = true);
        void selectModule(const std::vector<u32>& module_ids, bool clear_current_selection = true, bool navigate_to_selection = true);
        void selectModule(Module* module, bool clear_current_selection = true, bool navigate_to_selection = true);
        void selectModule(const std::vector<Module*>& modules, bool clear_current_selection = true, bool navigate_to_selection = true);

        void select(Gate* gate, bool clear_current_selection = true, bool navigate_to_selection = true);
        void select(Net* net, bool clear_current_selection = true, bool navigate_to_selection = true);
        void select(Module* module, bool clear_current_selection = true, bool navigate_to_selection = true);
        void select(const std::vector<Gate*>& gates, bool clear_current_selection = true, bool navigate_to_selection = true);
        void select(const std::vector<Net*>& nets, bool clear_current_selection = true, bool navigate_to_selection = true);
        void select(const std::vector<Module*>& modules, bool clear_current_selection = true, bool navigate_to_selection = true);
        void select(const std::vector<u32>& gate_ids, const std::vector<u32>& net_ids, const std::vector<u32>& module_ids, bool clear_current_selection = true, bool navigate_to_selection = true);
        void select(const std::vector<Gate*>& gates, const std::vector<Net*>& nets, const std::vector<Module*>& modules, bool clear_current_selection = true, bool navigate_to_selection = true);

        void deselectGate(u32 gate_id);
        void deselectGate(const std::vector<u32>& gate_ids);
        void deselectGate(Gate* gate);
        void deselectGate(const std::vector<Gate*>& gates);

        void deselectNet(u32 mNetId);
        void deselectNet(const std::vector<u32>& net_ids);
        void deselectNet(Net* net);
        void deselectNet(const std::vector<Net*>& nets);

        void deselectModule(u32 module_id);
        void deselectModule(const std::vector<u32>& module_ids);
        void deselectModule(Module* module);
        void deselectModule(const std::vector<Module*>& modules);

        void deselectAllItems();
        void deselect(Gate* gate);
        void deselect(Net* net);
        void deselect(Module* module);
        void deselect(const std::vector<Gate*>& gates);
        void deselect(const std::vector<Net*>& nets);
        void deselect(const std::vector<Module*>& modules);
        void deselect(const std::vector<u32>& gate_ids, const std::vector<u32>& net_ids, const std::vector<u32>& module_ids);
        void deselect(const std::vector<Gate*>& gates, const std::vector<Net*>& nets, const std::vector<Module*>& modules);

    Q_SIGNALS:
        void navigationRequested();
    };
}
