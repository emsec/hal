#ifndef GUI_API_H
#define GUI_API_H

#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/module.h"

#include <vector>
#include <tuple>

class gui_api
{
public:
    gui_api();
    ~gui_api();

    std::vector<u32> get_selected_gate_ids() const;
    std::vector<u32> get_selected_net_ids() const;
    std::vector<u32> get_selected_module_ids() const;
    std::tuple<std::vector<u32>, std::vector<u32>, std::vector<u32>> get_selected_item_ids() const;

    std::vector<std::shared_ptr<gate>> get_selected_gates() const;
    std::vector<std::shared_ptr<net>> get_selected_nets() const;
    std::vector<std::shared_ptr<module>> get_selected_modules() const;
    std::tuple<std::vector<std::shared_ptr<gate>>, std::vector<std::shared_ptr<net>>, std::vector<std::shared_ptr<module>>> get_selected_items() const;

    /*
    void print_selected_gates() const;
    void print_selected_nets() const;
    void print_selected_modules() const;
    void print_selected_items() const;
    */

    void select_gate(u32 gate_id, bool clear_current_selection = true);
    void select_gate(const std::vector<u32>& gate_ids, bool clear_current_selection = true);
    void select_gate(const std::shared_ptr<gate>& gate, bool clear_current_selection = true);
    void select_gate(const std::vector<std::shared_ptr<gate>>& gates, bool clear_current_selection = true);

    void select_net(u32 net_id, bool clear_current_selection = true);
    void select_net(const std::vector<u32>& net_ids, bool clear_current_selection = true);
    void select_net(const std::shared_ptr<net>& net, bool clear_current_selection = true);
    void select_net(const std::vector<std::shared_ptr<net>>& nets, bool clear_current_selection = true);

    void select_module(u32 module_id, bool clear_current_selection = true);  
    void select_module(const std::vector<u32>& module_ids, bool clear_current_selection = true);
    void select_module(const std::shared_ptr<module>& module, bool clear_current_selection = true);
    void select_module(const std::vector<std::shared_ptr<module>>& modules, bool clear_current_selection = true);

    void select(const std::shared_ptr<gate>& gate, bool clear_current_selection = true);
    void select(const std::shared_ptr<net>& net, bool clear_current_selection = true);
    void select(const std::shared_ptr<module>& module, bool clear_current_selection = true);
    void select(const std::vector<std::shared_ptr<gate>>& gates, bool clear_current_selection = true);
    void select(const std::vector<std::shared_ptr<net>>& nets, bool clear_current_selection = true);
    void select(const std::vector<std::shared_ptr<module>>& modules, bool clear_current_selection = true);
    void select(const std::vector<u32>& gate_ids, const std::vector<u32>& net_ids, const std::vector<u32>& module_ids, bool clear_current_selection = true);
    void select(const std::vector<std::shared_ptr<gate>>& gates, const std::vector<std::shared_ptr<net>>& nets, const std::vector<std::shared_ptr<module>>& modules, bool clear_current_selection = true);

    void deselect_gate(u32 gate_id);
    void deselect_gate(const std::vector<u32>& gate_ids);
    void deselect_gate(const std::shared_ptr<gate>& gate);
    void deselect_gate(const std::vector<std::shared_ptr<gate>>& gates);

    void deselect_net(u32 net_id);
    void deselect_net(const std::vector<u32>& net_ids);
    void deselect_net(const std::shared_ptr<net>& net);
    void deselect_net(const std::vector<std::shared_ptr<net>>& nets);

    void deselect_module(u32 module_id);  
    void deselect_module(const std::vector<u32>& module_ids);
    void deselect_module(const std::shared_ptr<module>& module);
    void deselect_module(const std::vector<std::shared_ptr<module>>& modules);

    void deselect_all_items();
    void deselect(const std::shared_ptr<gate>& gate);
    void deselect(const std::shared_ptr<net>& net);
    void deselect(const std::shared_ptr<module>& module);
    void deselect(const std::vector<std::shared_ptr<gate>>& gates);
    void deselect(const std::vector<std::shared_ptr<net>>& nets);
    void deselect(const std::vector<std::shared_ptr<module>>& modules);
    void deselect(const std::vector<u32>& gate_ids, const std::vector<u32>& net_ids, const std::vector<u32>& module_ids );
    void deselect(const std::vector<std::shared_ptr<gate>>& gates, const std::vector<std::shared_ptr<net>>& nets, const std::vector<std::shared_ptr<module>>& modules);
};

#endif // GUI_API_H
