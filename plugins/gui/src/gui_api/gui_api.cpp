#include "gui/gui_api/gui_api.h"


#include "gui/gui_globals.h"
#include "gui/user_action/user_action_compound.h"
#include "gui/user_action/user_action_object.h"
#include "gui/user_action/user_action_manager.h"
#include "gui/user_action/action_create_object.h"
#include "gui/user_action/action_delete_object.h"
#include "gui/user_action/action_add_items_to_object.h"
#include "gui/user_action/action_remove_items_from_object.h"
#include "gui/user_action/action_rename_object.h"
#include "gui/user_action/action_fold_module.h"
#include "gui/user_action/action_unfold_module.h"
#include "gui/graph_widget/graph_context_manager.h"
#include "gui/context_manager_widget/models/context_table_model.h"

#include <algorithm>

#include <QSet>

namespace hal
{
    GuiApi::GuiApi()
    {
        gSelectionRelay->registerSender(this, "GUI API");
    }

    std::vector<u32> GuiApi::getSelectedGateIds()
    {
        return gSelectionRelay->selectedGatesVector();
    }

    std::vector<u32> GuiApi::getSelectedNetIds()
    {
        return gSelectionRelay->selectedNetsVector();
    }

    std::vector<u32> GuiApi::getSelectedModuleIds()
    {
        return gSelectionRelay->selectedModulesVector();
    }

    std::tuple<std::vector<u32>, std::vector<u32>, std::vector<u32>> GuiApi::getSelectedItemIds()
    {
        return std::make_tuple(getSelectedGateIds(), getSelectedNetIds(), getSelectedModuleIds());
    }

    std::vector<Gate*> GuiApi::getSelectedGates()
    {
        std::vector<Gate*> gates;
        for (u32 id : gSelectionRelay->selectedGatesList())
            gates.push_back(gNetlist->get_gate_by_id(id));
        return gates;
    }

    std::vector<Net*> GuiApi::getSelectedNets()
    {
        std::vector<Net*> nets;
        for (u32 id : gSelectionRelay->selectedNetsList())
            nets.push_back(gNetlist->get_net_by_id(id));
        return nets;
    }

    std::vector<Module*> GuiApi::getSelectedModules()
    {
        std::vector<Module*> modules;
        for (u32 id : gSelectionRelay->selectedModulesList())
            modules.push_back(gNetlist->get_module_by_id(id));
        return modules;
    }

    std::tuple<std::vector<Gate*>, std::vector<Net*>, std::vector<Module*>> GuiApi::getSelectedItems()
    {
        return std::make_tuple(getSelectedGates(), getSelectedNets(), getSelectedModules());
    }

    void GuiApi::selectGate(Gate* gate, bool clear_current_selection, bool navigate_to_selection)
    {
        if(!gNetlist->is_gate_in_netlist(gate))
            return;

        if(clear_current_selection)
            gSelectionRelay->clear();

        gSelectionRelay->addGate(gate->get_id());
        gSelectionRelay->setFocus(SelectionRelay::ItemType::Gate,gate->get_id());
        gSelectionRelay->selectionChanged(this);

        if(navigate_to_selection)
            Q_EMIT navigationRequested();
    }

    void GuiApi::selectGate(u32 gate_id, bool clear_current_selection, bool navigate_to_selection)
    {
        selectGate(gNetlist->get_gate_by_id(gate_id), clear_current_selection, navigate_to_selection);
    }

    void GuiApi::selectGate(const std::vector<Gate*>& gates, bool clear_current_selection, bool navigate_to_selection)
    {
        QSet<u32> gate_ids;

        for(auto gate : gates)
        {
            if(!gNetlist->is_gate_in_netlist(gate))
                return;

            gate_ids.insert(gate->get_id());
        }

        if(clear_current_selection)
            gSelectionRelay->clear();

        gate_ids.unite(gSelectionRelay->selectedGates());
        gSelectionRelay->setSelectedGates(gate_ids);
        gSelectionRelay->selectionChanged(this);

        if(navigate_to_selection)
            Q_EMIT navigationRequested();
    }

    void GuiApi::selectGate(const std::vector<u32>& gate_ids, bool clear_current_selection, bool navigate_to_selection)
    {
        std::vector<Gate*> gates(gate_ids.size());
        std::transform(gate_ids.begin(), gate_ids.end(), gates.begin(), [](u32 gate_id){return gNetlist->get_gate_by_id(gate_id);});
        selectGate(gates, clear_current_selection, navigate_to_selection);
    }

    void GuiApi::selectNet(Net* net, bool clear_current_selection, bool navigate_to_selection)
    {
        if(!gNetlist->is_net_in_netlist(net))
            return;

        if(clear_current_selection)
            gSelectionRelay->clear();

        gSelectionRelay->addNet(net->get_id());
        gSelectionRelay->setFocus(SelectionRelay::ItemType::Net,net->get_id());
        gSelectionRelay->selectionChanged(this);

        if(navigate_to_selection)
            Q_EMIT navigationRequested();
    }

    void GuiApi::selectNet(u32 netId, bool clear_current_selection, bool navigate_to_selection)
    {
        selectNet(gNetlist->get_net_by_id(netId), clear_current_selection, navigate_to_selection);
    }

    void GuiApi::selectNet(const std::vector<Net*>& nets, bool clear_current_selection, bool navigate_to_selection)
    {
        QSet<u32> net_ids;

        for(auto net : nets)
        {
            if(!gNetlist->is_net_in_netlist(net))
                return;

            net_ids.insert(net->get_id());
        }

        if(clear_current_selection)
            gSelectionRelay->clear();

        net_ids.unite(gSelectionRelay->selectedNets());
        gSelectionRelay->setSelectedNets(net_ids);
        gSelectionRelay->selectionChanged(this);

        if(navigate_to_selection)
            Q_EMIT navigationRequested();
    }

    void GuiApi::selectNet(const std::vector<u32>& net_ids, bool clear_current_selection, bool navigate_to_selection)
    {
        std::vector<Net*> nets(net_ids.size());
        std::transform(net_ids.begin(), net_ids.end(), nets.begin(), [](u32 mNetId){return gNetlist->get_net_by_id(mNetId);});
        selectNet(nets, clear_current_selection, navigate_to_selection);
    }

    void GuiApi::selectModule(Module* module, bool clear_current_selection, bool navigate_to_selection)
    {
        if(!gNetlist->is_module_in_netlist(module))
            return;

        if(clear_current_selection)
            gSelectionRelay->clear();

        gSelectionRelay->addModule(module->get_id());
        gSelectionRelay->setFocus(SelectionRelay::ItemType::Module,module->get_id());
        gSelectionRelay->selectionChanged(this);

        if(navigate_to_selection)
            Q_EMIT navigationRequested();
    }

    void GuiApi::selectModule(u32 module_id, bool clear_current_selection, bool navigate_to_selection)
    {
        selectModule(gNetlist->get_module_by_id(module_id), clear_current_selection, navigate_to_selection);
    }

    void GuiApi::selectModule(const std::vector<Module*>& modules, bool clear_current_selection, bool navigate_to_selection)
    {
        QSet<u32> module_ids;

        for(auto module : modules)
        {
            if(!gNetlist->is_module_in_netlist(module))
                return;

            module_ids.insert(module->get_id());
        }

        if(clear_current_selection)
            gSelectionRelay->clear();

        module_ids.unite(gSelectionRelay->selectedModules());
        gSelectionRelay->setSelectedModules(module_ids);
        gSelectionRelay->selectionChanged(this);

        if(navigate_to_selection)
            Q_EMIT navigationRequested();
    }

    void GuiApi::selectModule(const std::vector<u32>& module_ids, bool clear_current_selection, bool navigate_to_selection)
    {
        std::vector<Module*> modules(module_ids.size());
        std::transform(module_ids.begin(), module_ids.end(), modules.begin(), [](u32 g_id){return gNetlist->get_module_by_id(g_id);});
        selectModule(modules, clear_current_selection, navigate_to_selection);
    }

    void GuiApi::select(Gate* gate, bool clear_current_selection, bool navigate_to_selection)
    {
        selectGate(gate, clear_current_selection, navigate_to_selection);
    }

    void GuiApi::select(Net* net, bool clear_current_selection, bool navigate_to_selection)
    {
        selectNet(net, clear_current_selection, navigate_to_selection);
    }
    void GuiApi::select(Module* module, bool clear_current_selection, bool navigate_to_selection)
    {
        selectModule(module, clear_current_selection, navigate_to_selection);
    }

    void GuiApi::select(const std::vector<Gate*>& gates, bool clear_current_selection, bool navigate_to_selection)
    {
        selectGate(gates, clear_current_selection, navigate_to_selection);
    }
    void GuiApi::select(const std::vector<Net*>& nets, bool clear_current_selection, bool navigate_to_selection)
    {
        selectNet(nets, clear_current_selection, navigate_to_selection);
    }

    void GuiApi::select(const std::vector<Module*>& modules, bool clear_current_selection, bool navigate_to_selection)
    {
        selectModule(modules, clear_current_selection, navigate_to_selection);
    }

    void GuiApi::select(const std::vector<u32>& gate_ids, const std::vector<u32>& net_ids, const std::vector<u32>& module_ids, bool clear_current_selection, bool navigate_to_selection)
    {
        if(clear_current_selection)
            gSelectionRelay->clear();

        selectGate(gate_ids, false, false);
        selectNet(net_ids, false, false);
        selectModule(module_ids, false, false);

        if(navigate_to_selection)
            Q_EMIT navigationRequested();
    }

    void GuiApi::select(const std::vector<Gate*>& gates, const std::vector<Net*>& nets, const std::vector<Module*>& modules, bool clear_current_selection, bool navigate_to_selection)
    {
        if(clear_current_selection)
            gSelectionRelay->clear();

        selectGate(gates, false, false);
        selectNet(nets, false, false);
        selectModule(modules, false, false);

        if(navigate_to_selection)
            Q_EMIT navigationRequested();
    }

    void GuiApi::deselectGate(Gate* gate)
    {
        if(!gNetlist->is_gate_in_netlist(gate))
            return;

        gSelectionRelay->removeGate(gate->get_id());
        gSelectionRelay->selectionChanged(this);
    }

    void GuiApi::deselectGate(u32 gate_id)
    {
        deselectGate(gNetlist->get_gate_by_id(gate_id));
    }

    void GuiApi::deselectGate(const std::vector<Gate*>& gates)
    {
        QSet<u32> gate_ids = gSelectionRelay->selectedGates();

        for(Gate* gate : gates)
        {
            if(!gNetlist->is_gate_in_netlist(gate))
                return;

            auto it = gate_ids.find(gate->get_id());
            if (it != gate_ids.end()) gate_ids.erase(it);
        }

        gSelectionRelay->setSelectedGates(gate_ids);
        gSelectionRelay->selectionChanged(this);
    }

    void GuiApi::deselectGate(const std::vector<u32>& gate_ids)
    {
        std::vector<Gate*> gates(gate_ids.size());
        std::transform(gate_ids.begin(), gate_ids.end(), gates.begin(), [](u32 gate_id){return gNetlist->get_gate_by_id(gate_id);});
        deselectGate(gates);
    }

    void GuiApi::deselectNet(Net* net)
    {
        if(!gNetlist->is_net_in_netlist(net))
            return;

        gSelectionRelay->removeNet(net->get_id());
        gSelectionRelay->selectionChanged(this);
    }

    void GuiApi::deselectNet(u32 netId)
    {
        deselectNet(gNetlist->get_net_by_id(netId));
    }

    void GuiApi::deselectNet(const std::vector<Net*>& nets)
    {
        QSet<u32> net_ids = gSelectionRelay->selectedNets();

        for(Net* net : nets)
        {
            if(!gNetlist->is_net_in_netlist(net))
                return;

            auto it = net_ids.find(net->get_id());
            if (it != net_ids.end()) net_ids.erase(it);
        }

        gSelectionRelay->setSelectedNets(net_ids);
        gSelectionRelay->selectionChanged(this);
    }

    void GuiApi::deselectNet(const std::vector<u32>& net_ids)
    {
        std::vector<Net*> nets(net_ids.size());
        std::transform(net_ids.begin(), net_ids.end(), nets.begin(), [](u32 mNetId){return gNetlist->get_net_by_id(mNetId);});
        deselectNet(nets);
    }

    void GuiApi::deselectModule(Module* module)
    {
        if(!gNetlist->is_module_in_netlist(module))
            return;

        gSelectionRelay->removeModule(module->get_id());
        gSelectionRelay->selectionChanged(this);
    }

    void GuiApi::deselectModule(u32 module_id)
    {
        deselectModule(gNetlist->get_module_by_id(module_id));
    }

    void GuiApi::deselectModule(const std::vector<Module*>& modules)
    {
        QSet<u32> module_ids = gSelectionRelay->selectedModules();

        for(Module* module : modules)
        {
            if(!gNetlist->is_module_in_netlist(module))
                return;
            auto it = module_ids.find(module->get_id());
            if (it != module_ids.end()) module_ids.erase(it);
        }

        gSelectionRelay->setSelectedModules(module_ids);
        gSelectionRelay->selectionChanged(this);
    }

    void GuiApi::deselectModule(const std::vector<u32>& module_ids)
    {
        std::vector<Module*> modules(module_ids.size());
        std::transform(module_ids.begin(), module_ids.end(), modules.begin(), [](u32 module_id){return gNetlist->get_module_by_id(module_id);});
        deselectModule(modules);
    }

    void GuiApi::deselect(Gate* gate)
    {
        deselectGate(gate);
    }

    void GuiApi::deselect(Net* net)
    {
        deselectNet(net);
    }
    void GuiApi::deselect(Module* module)
    {
        deselectModule(module);
    }

    void GuiApi::deselect(const std::vector<Gate*>& gates)
    {
        deselectGate(gates);
    }
    void GuiApi::deselect(const std::vector<Net*>& nets)
    {
        deselectNet(nets);
    }

    void GuiApi::deselect(const std::vector<Module*>& modules)
    {
        deselectModule(modules);
    }

    void GuiApi::deselect(const std::vector<u32>& gate_ids, const std::vector<u32>& net_ids, const std::vector<u32>& module_ids)
    {
        deselectGate(gate_ids);
        deselectNet(net_ids);
        deselectModule(module_ids);
    }

    void GuiApi::deselect(const std::vector<Gate*>& gates, const std::vector<Net*>& nets, const std::vector<Module*>& modules)
    {
        deselectGate(gates);
        deselectNet(nets);
        deselectModule(modules);
    }

    void GuiApi::deselectAllItems()
    {
        gSelectionRelay->clearAndUpdate();
    }


    int GuiApiClasses::View::isolateInNew(std::vector<Module*> modules, std::vector<Gate*> gates)
    {
        //check if the inputs are valid
        for(Module* mod : modules)
            if(mod == nullptr) return 0;

        QString name;

        bool isModuleExclusive = false;


        //make sure that modules and gates are not empty
        if(modules.empty() && gates.empty())
            return 0;

        //Check if view should be bound exclusively to module
        if(modules.size() == 1 && gates.empty() && modules[0]){
            name = QString::fromStdString(modules[0]->get_name()) + QString(" (ID: %1)").arg(modules[0]->get_id());
            isModuleExclusive = true;
            //If the view already exists then return existing id
            if(gGraphContextManager->contextWithNameExists(name)){
                for(GraphContext* ctx : gGraphContextManager->getContexts()){
                    if(ctx->name() == name){
                        return ctx->id();
                    }
                }
                return 0;
            }
        }
        else
        {
            //Get the number which has to be appended to name
            name   = gGraphContextManager->nextViewName("Isolated View");
        }
        GuiApiClasses::View::ModuleGateIdPair pair = GuiApiClasses::View::getValidObjects(0, modules, gates);


        // Get ids from modules and gates
        QSet<u32> moduleIds = pair.moduleIds;
        QSet<u32> gateIds = pair.gateIds;

        if(moduleIds.isEmpty() && gateIds.isEmpty())
            return 0;

        UserActionCompound* act = new UserActionCompound;
        act->setUseCreatedObject();
        act->addAction(new ActionCreateObject(UserActionObjectType::Context, name));
        act->addAction(new ActionAddItemsToObject(moduleIds, gateIds));
        UserActionManager::instance()->executeActionBlockThread(act);

        if (isModuleExclusive){
            GraphContext* context = gGraphContextManager->getContextById(act->object().id());
            context->setDirty(false);
            context->setExclusiveModuleId(modules[0]->get_id());
        }


        return act->object().id();
    }

    bool GuiApiClasses::View::deleteView(int id)
    {
        if(gGraphContextManager->getContextById(id) == nullptr)
        {
            return false;
        }
        ActionDeleteObject* act = new ActionDeleteObject();
        act->setObject(UserActionObject(id, UserActionObjectType::Context));
        UserActionManager::instance()->executeActionBlockThread(act);
        return true;
    }


    bool GuiApiClasses::View::addTo(int id, const std::vector<Module*> modules, const std::vector<Gate*> gates)
    {
        //check if the inputs are valid
        for(Module* mod : modules)
            if(mod == nullptr) return false;
        for(Gate* gt : gates)
            if(gt == nullptr) return false;


        if (!gGraphContextManager->getContextById(id)) return false; // context does not exist


        if (gGraphContextManager->getContextById(id)->isShowingModuleExclusively() && gGraphContextManager->getContextById(id)->getExclusiveModuleId() == gNetlist->get_top_module()->get_id())
            return false; //context shows topmodule so we can not add anything to it


        GuiApiClasses::View::ModuleGateIdPair pair = GuiApiClasses::View::getValidObjects(id, modules, gates);

        // Get ids from modules and gates
        QSet<u32> moduleIds = pair.moduleIds;
        QSet<u32> gateIds = pair.gateIds;


        ActionAddItemsToObject* act = new ActionAddItemsToObject(moduleIds,gateIds);
        act->setObject(UserActionObject(id,UserActionObjectType::Context));
        UserActionManager::instance()->executeActionBlockThread(act);
        return true;
    }

    bool GuiApiClasses::View::removeFrom(int id, const std::vector<Module*> modules, const std::vector<Gate*> gates)
    {
        //check if the inputs are valid
        for(Module* mod : modules)
            if(mod == nullptr) return false;
        for(Gate* gt : gates)
            if(gt == nullptr) return false;

        if (!gGraphContextManager->getContextById(id)) return false; // context does not exist

        QSet<u32> moduleIds;
        QSet<u32> gateIds;

        for(Module* module : modules)
            moduleIds.insert(module->get_id());
        for(Gate* gate : gates)
            gateIds.insert(gate->get_id());

        ActionRemoveItemsFromObject* act = new ActionRemoveItemsFromObject(moduleIds,gateIds);
        act->setObject(UserActionObject(id,UserActionObjectType::Context));
        UserActionManager::instance()->executeActionBlockThread(act);
        return true;
    }

    bool GuiApiClasses::View::setName(int id, const std::string& name)
    {


        if (!gGraphContextManager->getContextById(id)) return false; // context does not exist

        //check if name is occupied
        if(gGraphContextManager->contextWithNameExists(QString::fromStdString(name)))
            return false;

        //check if view is exclusively bound to module
        if(gGraphContextManager->getContextById(id)->isShowingModuleExclusively())
            return false;

        //get context matching id and rename it
        ActionRenameObject* act = new ActionRenameObject(QString::fromStdString(name));
        act->setObject(UserActionObject(id,UserActionObjectType::Context));
        UserActionManager::instance()->executeActionBlockThread(act);
        return true;
    }

    int GuiApiClasses::View::getId(const std::string& name)
    {
        //find View related to the name
        for(GraphContext* ctx : gGraphContextManager->getContexts()){
            if(ctx->name() == QString::fromStdString(name)){
                return ctx->id();
            }
        }
        return 0;
    }

    std::string GuiApiClasses::View::getName(int id)
    {
        GraphContext* ctx = gGraphContextManager->getContextById(id);
        if(ctx != nullptr){
            return ctx->name().toStdString();
        }
        return std::string(); //
    }

    std::vector<Module*> GuiApiClasses::View::getModules(int id)
    {
        GraphContext* ctx = gGraphContextManager->getContextById(id);
        if(ctx != nullptr){
            std::vector<Module*> modules;
            for(u32 id : ctx->modules()){
                modules.push_back(gNetlist->get_module_by_id(id));
            }
            return modules;
        }
        return std::vector<Module*>();
    }

    std::vector<Gate*> GuiApiClasses::View::getGates(int id)
    {
        GraphContext* ctx = gGraphContextManager->getContextById(id);
        if(ctx != nullptr){
            std::vector<Gate*> gates;
            for(u32 id : ctx->gates()){
                gates.push_back(gNetlist->get_gate_by_id(id));
            }
            return gates;
        }
        return std::vector<Gate*>();
    }

    std::vector<u32> GuiApiClasses::View::getIds(const std::vector<Module*> modules, const std::vector<Gate*> gates)
    {
        //check if the inputs are valid
        for(Module* mod : modules)
            if(mod == nullptr) return {};
        for(Gate* gt : gates)
            if(gt == nullptr) return {};

        std::vector<u32> ids;

        QSet<u32> moduleIds;
        QSet<u32> gateIds;



        //Get ids of given modules and gates
        for(Module* module : modules)
        {
            if(module)
                moduleIds.insert(module->get_id());
        }
        for(Gate* gate : gates)
        {
            if(gate)
                gateIds.insert(gate->get_id());
        }

        //iterate over each context and look if its showing modules
        for(GraphContext* ctx : gGraphContextManager->getContexts()){
            bool isCandidate = true;

            //Check if modules are in ctx
            for(u32 moduleId : moduleIds){
                if(ctx->modules().contains(moduleId))
                    continue;
                isCandidate = false;
                break;
            }
            //Only check modules if ctx still a valid candidate
            if(isCandidate){
                for(u32 gateId : gateIds){
                    if(ctx->gates().contains(gateId))
                        continue;
                    isCandidate = false;
                    break;
                }
            }

            //add it to ids if its a candidate
            if(isCandidate)
                ids.push_back(ctx->id());

        }

        return ids;
    }
    bool GuiApiClasses::View::unfoldModule(int view_id, Module *module)
    {
        if(!module)
            return false;
        GraphContext* context = gGraphContextManager->getContextById(view_id);

        //check if the inputs are valid
        if(module == nullptr || context == nullptr) return false;


        if(!context->modules().contains(module->get_id())) return false;

        ActionUnfoldModule *act = new ActionUnfoldModule(module->get_id());
        UserActionManager::instance()->executeActionBlockThread(act);
        return true;
    }

    bool GuiApiClasses::View::foldModule(int view_id, Module *module)
    {
        if(!module)
            return false;
        GraphContext* context = gGraphContextManager->getContextById(view_id);

        //check if the inputs are valid
        if(module == nullptr || context == nullptr) return false;

        //get gates and submodules that belong to the current module
        std::vector<Module*> submodules = module->get_submodules();
        std::vector<Gate*> gates = module->get_gates();

        bool isValidToFold = false;
        //check if the view contains gates and submodules of the current module, return false if not
        for(Gate* gate : gates)
            if(context->gates().contains(gate->get_id())) {isValidToFold = true; break;}
        for(Module* submodule : submodules)
            if(context->modules().contains(submodule->get_id())) {isValidToFold = true; break;}


        if (isValidToFold)
        {
            ActionFoldModule *act = new ActionFoldModule(module->get_id());
            UserActionManager::instance()->executeActionBlockThread(act);
            return true;
        }
        return false;
    }

    GuiApiClasses::View::ModuleGateIdPair GuiApiClasses::View::getValidObjects(int viewId, const std::vector<Module*> mods, const std::vector<Gate*> gats)
    {
        Module* topModule = gNetlist->get_top_module();


        //copy to prevent inplace operations
        std::vector<Module*> modules = mods;
        std::vector<Gate*> gates     = gats;

        //remove nullptr from list
        modules.erase(std::remove_if(modules.begin(), modules.end(), [](const auto& element) { return element == nullptr; }), modules.end());
        gates.erase(std::remove_if(gates.begin(), gates.end(), [](const auto& element) { return element == nullptr; }), gates.end());

        //set to store already existing mods and gates
        QSet<u32> existingModules;
        QSet<u32> existingGates;

        QSet<u32> Parents;

        QSet<u32> modIds;
        QSet<u32> gatIds;


        //0) if its not a new view we have to check and remove all parents which have to be placed
        if (viewId)
        {
            //put topmodule into parents because we dont iterate over it here
            //Parents.insert(topModule->get_id());
            std::vector<Module*> validMods;

            //Add parents from the view modules to Parents
            for (Module* mod : GuiApiClasses::View::getModules(viewId))
            {
                existingModules.insert(mod->get_id());
                validMods.push_back(mod);
                Module* itr = mod->get_parent_module();
                while (itr != nullptr)
                {
                    if (Parents.contains(itr->get_id()))
                        break;
                    Parents.insert(itr->get_id());
                    itr = itr->get_parent_module();
                }

            }
            //Add parents from gate to parents
            for (Gate* gate : GuiApiClasses::View::getGates(viewId))
            {
                existingGates.insert(gate->get_id());
                Module* itr = gate->get_module();
                while (itr != nullptr)
                {
                    if (Parents.contains(itr->get_id()))
                        break;
                    Parents.insert(itr->get_id());
                    itr = itr->get_parent_module();
                }
            }

            //Delete every parent from the list if submodule is in the view
            for (Module* mod : modules)
            {
                if (Parents.contains(mod->get_id()))
                    continue;
                validMods.push_back(mod);
            }
            modules = validMods;


            for (Module* mod : modules)
            {
                qInfo() << mod->get_id();
            }
        }
        //get ids of modules
        for (Module* mod : modules)
        {
            modIds.insert(mod->get_id());
        }


        //1) sort them by priority in DESCENDING order
        std::sort(modules.begin(), modules.end(), [](const Module* a, const Module* b) -> bool
                  {
                      //sorting in DESCENDING order to check from lowest to highest priority
                      return a->get_submodule_depth() > b->get_submodule_depth();
                  }
        );



        //2) remove id if parent is in set
        for(Module* mod : modules){
            //check if top module
            if(mod == topModule){
                // only add the top module because it has highest priority
                QSet<u32> temp;
                temp.insert(topModule->get_id());
                modIds = temp;
                break;
            }

            //check if parent is in current set and if so  remove current mod
            Module* iterator = mod->get_parent_module();
            while(iterator != nullptr){
                if(modIds.contains(iterator->get_id()))
                {
                    // parent is already in the set so remove mod and stop traversing parent tree
                    modIds.remove(mod->get_id());
                    break;
                }
                iterator = iterator->get_parent_module();
            }
        }


        //3) remove all gates which has its ancestor in modIds

        //check ancestors until topmodule or found in modIds

        for (Gate* gate : gates)
        {
            Module* itr       = gate->get_module();
            bool shouldInsert = true;
            while (itr != nullptr)
            {
                if (modIds.contains(itr->get_id()))
                {
                    gatIds.remove(gate->get_id());
                    shouldInsert = false;
                    break;
                }
                itr = itr->get_parent_module();
            }
            if (shouldInsert)
                gatIds.insert(gate->get_id());
        }



        //remove duplicates
        if (viewId)
        {
            modIds -= existingModules;
            gatIds -= existingGates;
        }


        //create struct to return module and gate ID pairs
        GuiApiClasses::View::ModuleGateIdPair pair;
        pair.moduleIds = modIds;
        pair.gateIds = gatIds;

        return pair;

    }
}
