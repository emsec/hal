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

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/module.h"


#include <vector>
#include <tuple>

#include <QObject>

namespace hal
{
    /**
     * @ingroup gui
     * @brief Interface to interact with the gui itself.
     *
     * The GuiApi is a class that contains several functions to simulate various actions within the gui like selecting
     * a graphical item (e.g. a gate) or to get a list of all currently selected items.
     *
     * Its main purpose is NOT to create an easier interface for other gui classes. Instead it implements the API that
     * is used by the python context to provide access to the gui features in the python console and the python code
     * editor execution.
     */
    class GuiApi : public QObject
    {
        Q_OBJECT

    public:
        /**
         * Constructor.
         */
        GuiApi();

        /**
         * Gets a list that contains the ids of all currently selected gates. The ids are not sorted in any way.
         *
         * @returns the ids of all selected gates
         */
        std::vector<u32> getSelectedGateIds();

        /**
         * Gets a list that contains the ids of all currently selected nets. The ids are not sorted in any way.
         *
         * @returns the ids of all selected nets
         */
        std::vector<u32> getSelectedNetIds();

        /**
         * Gets a list that contains the ids of all currently selected modules.
         * The ids are not sorted in any meaningful way.
         *
         * @returns the ids of all selected modules
         */
        std::vector<u32> getSelectedModuleIds();

        /**
         * Gets a tuple [selectedGateIds, selectedNetIds, selectedModuleIds]. The element selectedGate/-Net/-ModuleIds
         * is a list that contains the ids of all currently selected gates/nets/modules.
         * The ids are not sorted in any meaningful way.
         *
         * @returns the tuple [selectedGateIds, selectedNetIds, selectedModuleIds]
         */
        std::tuple<std::vector<u32>, std::vector<u32>, std::vector<u32>> getSelectedItemIds();

        /**
         * Gets a list of all currently selected gates. The gates are not sorted in any meaningful way.
         *
         * @returns  all selected gates
         */
        std::vector<Gate*> getSelectedGates();

        /**
         * Gets a list of all currently selected nets. The nets are not sorted in any meaningful way.
         *
         * @returns  all selected nets
         */
        std::vector<Net*> getSelectedNets();

        /**
         * Gets a list of all currently selected modules. The modules are not sorted in any meaningful way.
         *
         * @returns  all selected modules
         */
        std::vector<Module*> getSelectedModules();

        /**
         * Gets a tuple [selectedGates, selectedNets, selectedModules]. The element selectedGate/-Net/-Modules
         * is a list of all currently selected gates/nets/modules. The items are not sorted in any meaningful way.
         *
         * @returns the tuple [selectedGates, selectedNets, selectedModules]
         */
        std::tuple<std::vector<Gate*>, std::vector<Net*>, std::vector<Module*>> getSelectedItems();

        /**
         * Selects a single gate with the specified id.
         * By default, the new selection will overwrite the old one and the view will move to the new selection.
         * This behaviour can be changed with the parameters clear_current_selection and navigate_to_selection.
         *
         * @param gate_id - The id of the gate to select
         * @param clear_current_selection - <b>true</b> (default): The old selection will be overwritten. <br>
         *                                  <b>false</b>: The new selection will be appended to the old one.
         * @param navigate_to_selection - <b>true</b> (default): The view will be moved to the new selection. <br>
         *                                <b>false</b>: The view won't be moved.
         */
        void selectGate(u32 gate_id, bool clear_current_selection = true, bool navigate_to_selection = true);

        /**
         * Selects multiple gates that are specified by their id.
         * By default, the new selection will overwrite the old one and the view will move to the new selection.
         * This behaviour can be changed with the parameters clear_current_selection and navigate_to_selection.
         *
         * @param gate_ids - A list of gate ids. These gates will be selected.
         * @param clear_current_selection - <b>true</b> (default): The old selection will be overwritten. <br>
         *                                  <b>false</b>: The new selection will be appended to the old one.
         * @param navigate_to_selection - <b>true</b> (default): The view will be moved to the new selection. <br>
         *                                <b>false</b>: The view won't be moved.
         */
        void selectGate(const std::vector<u32>& gate_ids, bool clear_current_selection = true, bool navigate_to_selection = true);

        /**
         * Selects the specified gate.
         * By default, the new selection will overwrite the old one and the view will move to the new selection.
         * This behaviour can be changed with the parameters clear_current_selection and navigate_to_selection.
         *
         * @param gate - The gate to select
         * @param clear_current_selection - <b>true</b> (default): The old selection will be overwritten. <br>
         *                                  <b>false</b>: The new selection will be appended to the old one.
         * @param navigate_to_selection - <b>true</b> (default): The view will be moved to the new selection. <br>
         *                                <b>false</b>: The view won't be moved.
         */
        void selectGate(Gate* gate, bool clear_current_selection = true, bool navigate_to_selection = true);

        /**
         * Selects multiple gates.
         * By default, the new selection will overwrite the old one and the view will move to the new selection.
         * This behaviour can be changed with the parameters clear_current_selection and navigate_to_selection.
         *
         * @param gates - A list of gates. These gates will be selected.
         * @param clear_current_selection - <b>true</b> (default): The old selection will be overwritten. <br>
         *                                  <b>false</b>: The new selection will be appended to the old one.
         * @param navigate_to_selection - <b>true</b> (default): The view will be moved to the new selection. <br>
         *                                <b>false</b>: The view won't be moved.
         */
        void selectGate(const std::vector<Gate*>& gates, bool clear_current_selection = true, bool navigate_to_selection = true);

        /**
         * Selects a single net with the specified id.
         * By default, the new selection will overwrite the old one and the view will move to the new selection.
         * This behaviour can be changed with the parameters clear_current_selection and navigate_to_selection.
         *
         * @param netId - The id of the net to select
         * @param clear_current_selection - <b>true</b> (default): The old selection will be overwritten. <br>
         *                                  <b>false</b>: The new selection will be appended to the old one.
         * @param navigate_to_selection - <b>true</b> (default): The view will be moved to the new selection. <br>
         *                                <b>false</b>: The view won't be moved.
         */
        void selectNet(u32 netId, bool clear_current_selection = true, bool navigate_to_selection = true);

        /**
         * Selects multiple nets that are specified by their id.
         * By default, the new selection will overwrite the old one and the view will move to the new selection.
         * This behaviour can be changed with the parameters clear_current_selection and navigate_to_selection.
         *
         * @param net_ids - A list of net ids. These nets will be selected.
         * @param clear_current_selection - <b>true</b> (default): The old selection will be overwritten. <br>
         *                                  <b>false</b>: The new selection will be appended to the old one.
         * @param navigate_to_selection - <b>true</b> (default): The view will be moved to the new selection. <br>
         *                                <b>false</b>: The view won't be moved.
         */
        void selectNet(const std::vector<u32>& net_ids, bool clear_current_selection = true, bool navigate_to_selection = true);

        /**
         * Selects the specified net.
         * By default, the new selection will overwrite the old one and the view will move to the new selection.
         * This behaviour can be changed with the parameters clear_current_selection and navigate_to_selection.
         *
         * @param net - The net to select
         * @param clear_current_selection - <b>true</b> (default): The old selection will be overwritten. <br>
         *                                  <b>false</b>: The new selection will be appended to the old one.
         * @param navigate_to_selection - <b>true</b> (default): The view will be moved to the new selection. <br>
         *                                <b>false</b>: The view won't be moved.
         */
        void selectNet(Net* net, bool clear_current_selection = true, bool navigate_to_selection = true);

        /**
         * Selects multiple nets.
         * By default, the new selection will overwrite the old one and the view will move to the new selection.
         * This behaviour can be changed with the parameters clear_current_selection and navigate_to_selection.
         *
         * @param nets - A list of nets. These nets will be selected.
         * @param clear_current_selection - <b>true</b> (default): The old selection will be overwritten. <br>
         *                                  <b>false</b>: The new selection will be appended to the old one.
         * @param navigate_to_selection - <b>true</b> (default): The view will be moved to the new selection. <br>
         *                                <b>false</b>: The view won't be moved.
         */
        void selectNet(const std::vector<Net*>& nets, bool clear_current_selection = true, bool navigate_to_selection = true);

        /**
         * Selects a single module with the specified id.
         * By default, the new selection will overwrite the old one and the view will move to the new selection.
         * This behaviour can be changed with the parameters clear_current_selection and navigate_to_selection.
         *
         * @param module_id - The id of the module to select
         * @param clear_current_selection - <b>true</b> (default): The old selection will be overwritten. <br>
         *                                  <b>false</b>: The new selection will be appended to the old one.
         * @param navigate_to_selection - <b>true</b> (default): The view will be moved to the new selection. <br>
         *                                <b>false</b>: The view won't be moved.
         */
        void selectModule(u32 module_id, bool clear_current_selection = true, bool navigate_to_selection = true);

        /**
         * Selects multiple modules that are specified by their id.
         * By default, the new selection will overwrite the old one and the view will move to the new selection.
         * This behaviour can be changed with the parameters clear_current_selection and navigate_to_selection.
         *
         * @param module_ids - A list of module ids. These modules will be selected.
         * @param clear_current_selection - <b>true</b> (default): The old selection will be overwritten. <br>
         *                                  <b>false</b>: The new selection will be appended to the old one.
         * @param navigate_to_selection - <b>true</b> (default): The view will be moved to the new selection. <br>
         *                                <b>false</b>: The view won't be moved.
         */
        void selectModule(const std::vector<u32>& module_ids, bool clear_current_selection = true, bool navigate_to_selection = true);

        /**
         * Selects the specified module.
         * By default, the new selection will overwrite the old one and the view will move to the new selection.
         * This behaviour can be changed with the parameters clear_current_selection and navigate_to_selection.
         *
         * @param module - The module to select
         * @param clear_current_selection - <b>true</b> (default): The old selection will be overwritten. <br>
         *                                  <b>false</b>: The new selection will be appended to the old one.
         * @param navigate_to_selection - <b>true</b> (default): The view will be moved to the new selection. <br>
         *                                <b>false</b>: The view won't be moved.
         */
        void selectModule(Module* module, bool clear_current_selection = true, bool navigate_to_selection = true);

        /**
         * Selects multiple modules.
         * By default, the new selection will overwrite the old one and the view will move to the new selection.
         * This behaviour can be changed with the parameters clear_current_selection and navigate_to_selection.
         *
         * @param modules - A list of modules. These modules will be selected.
         * @param clear_current_selection - <b>true</b> (default): The old selection will be overwritten. <br>
         *                                  <b>false</b>: The new selection will be appended to the old one.
         * @param navigate_to_selection - <b>true</b> (default): The view will be moved to the new selection. <br>
         *                                <b>false</b>: The view won't be moved.
         */
        void selectModule(const std::vector<Module*>& modules, bool clear_current_selection = true, bool navigate_to_selection = true);

        /**
         * Selects the specified gate.
         * By default, the new selection will overwrite the old one and the view will move to the new selection.
         * This behaviour can be changed with the parameters clear_current_selection and navigate_to_selection.
         *
         * @param gate - The gate to select
         * @param clear_current_selection - <b>true</b> (default): The old selection will be overwritten. <br>
         *                                  <b>false</b>: The new selection will be appended to the old one.
         * @param navigate_to_selection - <b>true</b> (default): The view will be moved to the new selection. <br>
         *                                <b>false</b>: The view won't be moved.
         */
        void select(Gate* gate, bool clear_current_selection = true, bool navigate_to_selection = true);

        /**
         * Selects the specified net.
         * By default, the new selection will overwrite the old one and the view will move to the new selection.
         * This behaviour can be changed with the parameters clear_current_selection and navigate_to_selection.
         *
         * @param net - The net to select
         * @param clear_current_selection - <b>true</b> (default): The old selection will be overwritten. <br>
         *                                  <b>false</b>: The new selection will be appended to the old one.
         * @param navigate_to_selection - <b>true</b> (default): The view will be moved to the new selection. <br>
         *                                <b>false</b>: The view won't be moved.
         */
        void select(Net* net, bool clear_current_selection = true, bool navigate_to_selection = true);

        /**
         * Selects the specified module.
         * By default, the new selection will overwrite the old one and the view will move to the new selection.
         * This behaviour can be changed with the parameters clear_current_selection and navigate_to_selection.
         *
         * @param module - The module to select
         * @param clear_current_selection - <b>true</b> (default): The old selection will be overwritten. <br>
         *                                  <b>false</b>: The new selection will be appended to the old one.
         * @param navigate_to_selection - <b>true</b> (default): The view will be moved to the new selection. <br>
         *                                <b>false</b>: The view won't be moved.
         */
        void select(Module* module, bool clear_current_selection = true, bool navigate_to_selection = true);

        /**
         * Selects multiple gates.
         * By default, the new selection will overwrite the old one and the view will move to the new selection.
         * This behaviour can be changed with the parameters clear_current_selection and navigate_to_selection.
         *
         * @param gates - A list of gates. These gates will be selected.
         * @param clear_current_selection - <b>true</b> (default): The old selection will be overwritten. <br>
         *                                  <b>false</b>: The new selection will be appended to the old one.
         * @param navigate_to_selection - <b>true</b> (default): The view will be moved to the new selection. <br>
         *                                <b>false</b>: The view won't be moved.
         */
        void select(const std::vector<Gate*>& gates, bool clear_current_selection = true, bool navigate_to_selection = true);

        /**
         * Selects multiple nets.
         * By default, the new selection will overwrite the old one and the view will move to the new selection.
         * This behaviour can be changed with the parameters clear_current_selection and navigate_to_selection.
         *
         * @param nets - A list of nets. These nets will be selected.
         * @param clear_current_selection - <b>true</b> (default): The old selection will be overwritten. <br>
         *                                  <b>false</b>: The new selection will be appended to the old one.
         * @param navigate_to_selection - <b>true</b> (default): The view will be moved to the new selection. <br>
         *                                <b>false</b>: The view won't be moved.
         */
        void select(const std::vector<Net*>& nets, bool clear_current_selection = true, bool navigate_to_selection = true);

        /**
         * Selects multiple modules.
         * By default, the new selection will overwrite the old one and the view will move to the new selection.
         * This behaviour can be changed with the parameters clear_current_selection and navigate_to_selection.
         *
         * @param modules - A list of modules. These modules will be selected.
         * @param clear_current_selection - <b>true</b> (default): The old selection will be overwritten. <br>
         *                                  <b>false</b>: The new selection will be appended to the old one.
         * @param navigate_to_selection - <b>true</b> (default): The view will be moved to the new selection. <br>
         *                                <b>false</b>: The view won't be moved.
         */
        void select(const std::vector<Module*>& modules, bool clear_current_selection = true, bool navigate_to_selection = true);

        /**
         * Selects multiple gates, nets and modules that are specified by their id.
         * By default, the new selection will overwrite the old one and the view will move to the new selection.
         * This behaviour can be changed with the parameters clear_current_selection and navigate_to_selection.
         *
         * @param gate_ids - A list of gate ids. These gates will bse selected.
         * @param net_ids - A list of net ids. These nets will be selected.
         * @param module_ids - A list of module ids. These modules will be selected.
         * @param clear_current_selection - <b>true</b> (default): The old selection will be overwritten. <br>
         *                                  <b>false</b>: The new selection will be appended to the old one.
         * @param navigate_to_selection - <b>true</b> (default): The view will be moved to the new selection. <br>
         *                                <b>false</b>: The view won't be moved.
         */
        void select(const std::vector<u32>& gate_ids, const std::vector<u32>& net_ids, const std::vector<u32>& module_ids, bool clear_current_selection = true, bool navigate_to_selection = true);

        /**
         * Selects multiple gates, nets and modules.
         * By default, the new selection will overwrite the old one and the view will move to the new selection.
         * This behaviour can be changed with the parameters clear_current_selection and navigate_to_selection.
         *
         * @param gates - A list of gates. These gates will be selected.
         * @param nets - A list of nets. These nets will be selected.
         * @param modules - A list of modules. These modules will be selected.
         * @param clear_current_selection - <b>true</b> (default): The old selection will be overwritten. <br>
         *                                  <b>false</b>: The new selection will be appended to the old one.
         * @param navigate_to_selection - <b>true</b> (default): The view will be moved to the new selection. <br>
         *                                <b>false</b>: The view won't be moved.
         */
        void select(const std::vector<Gate*>& gates, const std::vector<Net*>& nets, const std::vector<Module*>& modules, bool clear_current_selection = true, bool navigate_to_selection = true);

        /**
         * Removes the gate with the specified id from the selection.
         *
         * @param gate_id - The id of the gate that will be removed from the selection
         */
        void deselectGate(u32 gate_id);

        /**
         * Removes multiple gates that are specified by their id from the selection.
         *
         * @param gate_ids - A list of gate ids. These gates will be removed from the selection.
         */
        void deselectGate(const std::vector<u32>& gate_ids);

        /**
         * Removes the specified gate from the selection.
         *
         * @param gate - The gate that will be removed from the selection
         */
        void deselectGate(Gate* gate);

         /**
          * Removes multiple gates from the selection.
          *
          * @param gates - A list of gates. These gates will be removed from the selection.
          */
        void deselectGate(const std::vector<Gate*>& gates);

        /**
         * Removes the net with the specified id from the selection.
         *
         * @param netId - The id of the net that will be removed from the selection
         */
        void deselectNet(u32 netId);

        /**
         * Removes multiple nets that are specified by their id from the selection.
         *
         * @param net_ids - A list of net ids. These nets will be removed from the selection.
         */
        void deselectNet(const std::vector<u32>& net_ids);

        /**
         * Removes the specified net from the selection.
         *
         * @param net - The net that will be removed from the selection
         */
        void deselectNet(Net* net);

        /**
          * Removes multiple nets from the selection.
          *
          * @param nets - A list of nets. These nets will be removed from the selection.
          */
        void deselectNet(const std::vector<Net*>& nets);

        /**
         * Removes the module with the specified id from the selection.
         *
         * @param module_id - The id of the module that will be removed from the selection
         */
        void deselectModule(u32 module_id);

        /**
         * Removes multiple modules that are specified by their id from the selection.
         *
         * @param module_ids - A list of module ids. These modules will be removed from the selection.
         */
        void deselectModule(const std::vector<u32>& module_ids);

        /**
         * Removes the specified module from the selection.
         *
         * @param module - The module that will be removed from the selection
         */
        void deselectModule(Module* module);

        /**
          * Removes multiple modules from the selection.
          *
          * @param modules - A list of modules. These modules will be removed from the selection.
          */
        void deselectModule(const std::vector<Module*>& modules);

        /**
         * Clears the current selection.
         */
        void deselectAllItems();

        /**
         * Removes the specified gate from the selection.
         *
         * @param gate - The gate that will be removed from the selection
         */
        void deselect(Gate* gate);

        /**
         * Removes the specified net from the selection.
         *
         * @param net - The net that will be removed from the selection
         */
        void deselect(Net* net);

        /**
         * Removes the specified module from the selection.
         *
         * @param module - The module that will be removed from the selection
         */
        void deselect(Module* module);

        /**
          * Removes multiple gates from the selection.
          *
          * @param gates - A list of gates. These gates will be removed from the selection.
          */
        void deselect(const std::vector<Gate*>& gates);

        /**
          * Removes multiple nets from the selection.
          *
          * @param nets - A list of nets. These nets will be removed from the selection.
          */
        void deselect(const std::vector<Net*>& nets);

        /**
          * Removes multiple modules from the selection.
          *
          * @param modules - A list of modules. These modules will be removed from the selection.
          */
        void deselect(const std::vector<Module*>& modules);

        /**
         * Removes multiple gates, nets and modules that are specified by their id from the selection.
         *
         * @param gate_ids - A list of gate ids. These gates will be removed from the selection.
         * @param net_ids - A list of net ids. These nets will be removed from the selection.
         * @param module_ids - A list of module ids. These modules will be removed from the selection.
         */
        void deselect(const std::vector<u32>& gate_ids, const std::vector<u32>& net_ids, const std::vector<u32>& module_ids);

        /**
         * Removes multiple gates, nets and modules from the selection.
         *
         * @param gates - A list of gates. These gates will be removed from the selection.
         * @param nets - A list of nets. These nets will be removed from the selection.
         * @param modules - A list of modules. These modules will be removed from the selection.
         */
        void deselect(const std::vector<Gate*>& gates, const std::vector<Net*>& nets, const std::vector<Module*>& modules);

    Q_SIGNALS:
        /**
         * Q_SIGNAL that is emitted whenever the view should be moved to a new selection.
         */
        void navigationRequested();
    };
}
