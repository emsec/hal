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

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/module.h"
#include "gui/include/gui/gui_def.h"


#include <QObject>
#include <QSet>
#include <tuple>
#include <vector>
#include <optional>

namespace hal
{
    namespace GuiApiClasses {

        class View{
        public:
            /**
             * Isolates given modules and gates into a new view.
             * If only one module and no gates are given and 
             * there already exists a view which is exclusively bound the given module, 
             * then that view is returned instead.
             * 
             * @param modules - List of modules to be added.
             * @param gates - List of gates to be added
             * 
             * @return The id of the requested view. Returns 0 if the view could not be created.
             */
            static int isolateInNew(const std::vector<Module*> modules, const std::vector<Gate*> gates);
            /**
             * Deletes a view.
             * 
             * @param id - The id of the view.
             * 
             * @return True on success, otherwise false.
             */
            static bool deleteView(int id);
            /**
             * Adds modules and gates to a view.
             * 
             * @param id - The id of the view.
             * @param modules - The list of modules to be added to the view.
             * @param gates - The list of gates to be added to the view.
             * 
             * @return True on success, otherwise false.
             */
            static bool addTo(int id, const std::vector<Module*> modules, const std::vector<Gate*> gates);
            /**
             * Removes modules and gates from a view.
             * 
             * @param id - The id of the view to remove the modules and gates from.
             * @param modules - The modules to be removed from the view.
             * @param gates - The gates to be removed from the view.
             * 
             * @return True on success, otherwise false.
             */
            static bool removeFrom(int id, const std::vector<Module*> modules, const std::vector<Gate*> gates);
            /**
             * Sets the name of a view.
             * 
             * @param id - The id of the view.
             * @param name - The new name of the view.
             * 
             * @return True on success, otherwise false.
             */
            static bool setName(int id, const std::string& name);
            /**
             * Returns the id of a view.
             * 
             * @param name - The name of the view to search for.
             * 
             * @return The id of the view. Returns 0, if the view was not found.
             */
            static int getId(const std::string& name);
            /**
             * Returns the name of a view.
             * 
             * @param id - The id of the view.
             * 
             * @return The name of the view. Returns an empty string, if the view was not found.
             */
            static std::string getName(int id);
            /**
             * Returns the modules contained in a view.
             * 
             * @param id - The id of the view.
             * 
             * @return A list of modules. Returns an empty list, if the view was not found.
             */
            static std::vector<Module*> getModules(int id);
            /**
             * Returns the gates contained in a view.
             * 
             * @param id - The id of the view.
             * 
             * @return A list of gates. Returns an empty list, if the view was not found.
             */
            static std::vector<Gate*> getGates(int id);
            /**
             * Returns the ids of views containing at least the given modules and gates.
             * 
             * @param modules - List of required modules.
             * @param gates - List of required gates.
             * 
             * @return A list of ids of views, which contain all given modules and gates.
             */
            static std::vector<u32> getIds(const std::vector<Module*> modules, const std::vector<Gate*> gates);
            /**
             * Fold a specific module. Hides the submodules and gates, shows the specific module.
             * 
             * @param view_id - The id of the view.
             * @param module - The Module to fold.
             *
             * @return True on success, otherwise False.
             */
            static bool foldModule(int view_id, Module* module);
            /**
             * Unfold a specific module. Hides the module, shows submodules and gates.
             * 
             * @param view_id - The id of the view.
             * @param module - The Module to unfold.
             *
             * @return True on success, otherwise False.
             */
            static bool unfoldModule(int view_id, Module* module);

            struct ModuleGateIdPair {
                QSet<u32> moduleIds;
                QSet<u32> gateIds;
            };

            static ModuleGateIdPair getValidObjects(int viewId, const std::vector<Module*>, const std::vector<Gate*>);
            static GridPlacement* getGridPlacement(int viewId);
            static bool setGridPlacement(int viewId, GridPlacement* gp);

            /**
             * Returns the id of the current directory.
             * 
             * Used for selection and similar.
             * 
             * @returns The id of the current directory.
             */
            static u32 getCurrentDirectory();
            /**
             * Sets the id of the current directory.
             * 
             * Used for selection and similar.
             * 
             * @param id - The id of the new current directory.
             */
            static void setCurrentDirectory(u32 id);
            /**
             * Creates a new directory with a given name.
             * 
             * @param name - The name of the new directory.
             * 
             * @return The id of the newly created directory.
             */
            static u32 createNewDirectory(const std::string& name);
            /**
             * Deleted a directory.
             * 
             * @param id - The id of the directory.
             */
            static void deleteDirectory(u32 id);
            /**
             * Moves a view to a directory.
             * 
             * @param viewId - The id of the view.
             * @param destinationDirectoryId - The id of the destination directory. 
             *                                 If not given, the current directory is used instead.
             * @param row - Optional. The row there the view is inserted.
             */
            static void moveView(u32 viewId, std::optional<u32> destinationDirectoryId, std::optional<int> row);
            /**
             * Moves a directory under another directory.
             * 
             * @param directoryId - The id of the directory to move.
             * @param destinationDirectoryId - The id of the destination directory.
             *                                 If not given, the current directory is used instead.
             * @param row - Optional. The row there the view is inserted.
             */
            static void moveDirectory(u32 directoryId, std::optional<u32> destinationDirectoryId, std::optional<int> row);

            /**
             * Returns a list of child directories for a given directory.
             * 
             * @param directoryId - The id of the parent directory.
             * 
             * @return List of ids of child directories. 
             *         If the parent directory does not exist std::nullopt is returned instead.
             */
            static std::optional<std::vector<u32>> getChildDirectories(u32 directoryId);

            /**
             * Returns a list of child views for a given directory.
             * 
             * @param directoryId - The id of the parent directory.
             * 
             * @return List of ids of child views. 
             *         If the parent directory does not exist std::nullopt is returned instead.
             */
            static std::optional<std::vector<u32>> getChildViews(u32 directoryId);

            /**
             * Isolates the shortest path from one module to another in a new view
             *
             * @param fromModId - The id of the module the path starts from
             * @param toModId - The id of the module the path ends
             *
             * @return The ID of the created view if successull, 0 otherwise
             */
            static int isolateModuleToModulePathInNewView(u32 fromModId, u32 toModId);
        };
    }


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
