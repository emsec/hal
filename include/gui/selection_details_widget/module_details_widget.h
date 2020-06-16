#ifndef HAL_MODULE_DETAILS_WIDGET_H
#define HAL_MODULE_DETAILS_WIDGET_H

#include "def.h"

#include "gui/gui_def.h"
#include "netlist/endpoint.h"
#include "netlist_relay/netlist_relay.h"

#include <QWidget>

/* forward declaration */
class QLabel;
class QTableWidget;
class QTableWidgetItem;
class QTreeWidget;
class QTreeWidgetItem;
class QVBoxLayout;
class QHBoxLayout;
class QScrollArea;
class QGridLayout;
class QModelIndex;
class QFont;
class QPushButton;

class module_details_widget : public QWidget
{
    Q_OBJECT
public:
    module_details_widget(QWidget* parent = nullptr);

    void update(const u32 module_id);

public Q_SLOTS:

    void handle_netlist_marked_global_input(std::shared_ptr<Netlist> netlist, u32 associated_data);
    void handle_netlist_marked_global_output(std::shared_ptr<Netlist> netlist, u32 associated_data);
    void handle_netlist_marked_global_inout(std::shared_ptr<Netlist> netlist, u32 associated_data);
    void handle_netlist_unmarked_global_input(std::shared_ptr<Netlist> netlist, u32 associated_data);
    void handle_netlist_unmarked_global_output(std::shared_ptr<Netlist> netlist, u32 associated_data);
    void handle_netlist_unmarked_global_inout(std::shared_ptr<Netlist> netlist, u32 associated_data);

    void handle_module_name_changed(std::shared_ptr<Module> module);
    void handle_submodule_added(std::shared_ptr<Module> module, u32 associated_data);
    void handle_submodule_removed(std::shared_ptr<Module> module, u32 associated_data);
    void handle_module_gate_assigned(std::shared_ptr<Module> module, u32 associated_data);
    void handle_module_gate_removed(std::shared_ptr<Module> module, u32 associated_data);
    void handle_module_input_port_name_changed(std::shared_ptr<Module> module, u32 associated_data);
    void handle_module_output_port_name_changed(std::shared_ptr<Module> module, u32 associated_data);
    void handle_module_type_changed(std::shared_ptr<Module> module);

    void handle_net_name_changed(std::shared_ptr<Net> net);
    void handle_net_source_added(std::shared_ptr<Net> net, const u32 src_gate_id);
    void handle_net_source_removed(std::shared_ptr<Net> net, const u32 src_gate_id);
    void handle_net_destination_added(std::shared_ptr<Net> net, const u32 dst_gate_id);
    void handle_net_destination_removed(std::shared_ptr<Net> net, const u32 dst_gate_id);

private:
    QFont m_key_font;

    QScrollArea* m_scroll_area;
    QWidget* m_top_lvl_container;
    QVBoxLayout* m_top_lvl_layout;
    QVBoxLayout* m_content_layout;

    QPushButton* m_general_info_button;
    QPushButton* m_input_ports_button;
    QPushButton* m_output_ports_button;

    QTableWidget* m_general_table;

    QTableWidgetItem* m_name_item;
    QTableWidgetItem* m_id_item;
    QTableWidgetItem* m_type_item;
    QTableWidgetItem* m_number_of_gates_item;
    QTableWidgetItem* m_number_of_submodules_item;
    QTableWidgetItem* m_number_of_nets_item;

    QTableWidget* m_input_ports_table;

    QTableWidget* m_output_ports_table;

    void handle_buttons_clicked();

    QSize calculate_table_size(QTableWidget* table);

    u32 m_current_id;

    void add_general_table_static_item(QTableWidgetItem* item);
    void add_general_table_dynamic_item(QTableWidgetItem* item);
    void style_table(QTableWidget* table);
};

#endif /* HAL_MODULE_DETAILS_WIDGET_H*/
