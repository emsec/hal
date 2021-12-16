#include "gui/gui_api/gui_api.h"

#include "gui/gui_globals.h"

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
}
