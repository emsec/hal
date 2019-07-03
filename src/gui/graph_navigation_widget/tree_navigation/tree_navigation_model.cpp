#include "graph_navigation_widget/tree_navigation/tree_navigation_model.h"
#include "graph_navigation_widget/tree_navigation/tree_navigation_item.h"
#include "gui_globals.h"
#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/module.h"
#include "graph_layouter/old_graphics_item_qss_adapter.h"//for the styled icon, temporary
#include "gui_utility.h"

tree_navigation_model::tree_navigation_model(QObject* parent) : QAbstractItemModel(parent)
{
    QVector<QVariant> rootHeaderData;
    rootHeaderData << "Name"
                   << "ID"
                   << "Type";
    m_root_item = new tree_navigation_item(rootHeaderData);
    load_data_settings();
    setup_model_data();
}

tree_navigation_model::~tree_navigation_model()
{
    delete m_root_item;
}

QVariant tree_navigation_model::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (get_item(index)->get_type() == tree_navigation_item::item_type::structure && index.column() == 0)
    {
        if (role == Qt::FontRole)
            return m_structured_font;

        if (get_item(index) == m_top_design_item && role == Qt::DecorationRole)
            return m_design_icon;
    }

    if (role != Qt::DisplayRole)
        return QVariant();

    tree_navigation_item* item = get_item(index);
    return item->data(index.column());
}

QVariant tree_navigation_model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return m_root_item->data(section);

    return QVariant();
}

QModelIndex tree_navigation_model::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    tree_navigation_item* parentItem = get_item(parent);
    tree_navigation_item* childItem  = parentItem->get_child(row);

    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex tree_navigation_model::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    tree_navigation_item* childItem  = get_item(index);
    tree_navigation_item* parentItem = childItem->get_parent();

    if (parentItem == m_root_item)
        return QModelIndex();

    return createIndex(parentItem->get_row_number(), 0, parentItem);
}

Qt::ItemFlags tree_navigation_model::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}

int tree_navigation_model::rowCount(const QModelIndex& parent) const
{
    tree_navigation_item* item = get_item(parent);
    return item->get_child_count();
}

int tree_navigation_model::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return m_root_item->get_column_count();
}

QModelIndexList tree_navigation_model::get_corresponding_indexes(const QList<u32>& gate_ids, const QList<u32>& net_ids, const QList<u32>& module_ids)
{
    QModelIndexList list;
    for (int i = 0; i < m_gates_item->get_child_count(); i++)
    {
        tree_navigation_item* current_item = m_gates_item->get_child(i);
        if (gate_ids.contains(current_item->data(1).toInt()))
            list.append(get_modelindexes_for_row(current_item));
    }
    for (int i = 2; i < m_nets_item->get_child_count(); i++)    //first two items are the global input/output items
    {
        tree_navigation_item* current_item = m_nets_item->get_child(i);
        if (net_ids.contains(current_item->data(1).toInt()))
            list.append(get_modelindexes_for_row(current_item));
    }
    for (int i = 0; i < m_global_output_nets_item->get_child_count(); i++)
    {
        tree_navigation_item* current_item = m_global_output_nets_item->get_child(i);
        if (net_ids.contains(current_item->data(1).toInt()))
            list.append(get_modelindexes_for_row(current_item));
    }
    for (int i = 0; i < m_global_input_nets_item->get_child_count(); i++)
    {
        tree_navigation_item* current_item = m_global_input_nets_item->get_child(i);
        if (net_ids.contains(current_item->data(1).toInt()))
            list.append(get_modelindexes_for_row(current_item));
    }
    for (int i = 0; i < m_modules_item->get_child_count(); i++)
    {
        tree_navigation_item* current_item = m_modules_item->get_child(i);
        if (module_ids.contains(current_item->data(1).toInt()))
            list.append(get_modelindexes_for_row(current_item));
    }
    return list;
}


void tree_navigation_model::handle_module_created(std::shared_ptr<module> m)
{

    m_modules_item->set_data(NAME_COLUMN, "modules : " + QString::number(g_netlist->get_modules().size()));
    dataChanged(get_modelindex(m_modules_item), get_modelindex(m_modules_item), QVector<int>() << Qt::DisplayRole);
    tree_navigation_item* module_item =
        new tree_navigation_item(QVector<QVariant>() << QString::fromStdString(m->get_name()) << (int)m->get_id() << "", tree_navigation_item::item_type::module, m_modules_item);
    module_item->insert_child(0,
                                 new tree_navigation_item(QVector<QVariant>() << "Gates"
                                                                              << ""
                                                                              << "",
                                                          tree_navigation_item::item_type::ignore,
                                                          module_item));
    /*
    module_item->insert_child(1,
                                 new tree_navigation_item(QVector<QVariant>() << "Nets"
                                                                              << ""
                                                                              << "",
                                                          tree_navigation_item::item_type::ignore,
                                                          module_item));
    */
insert_item(m_modules_item, m_modules_item->get_child_count(), module_item);

}

void tree_navigation_model::handle_module_gate_assigned(std::shared_ptr<module> m, u32 assigned_gate)
{
    for (int i = 0; i < m_modules_item->get_child_count(); i++)
    {
        tree_navigation_item* current_submod_item = m_modules_item->get_child(i);
        if (current_submod_item->data(ID_COLUMN).toInt() == (int)m->get_id())
        {
            auto added_gate          = g_netlist->get_gate_by_id(assigned_gate);
            auto gate_structure_item = current_submod_item->get_child(0);
            insert_item(gate_structure_item,
                        gate_structure_item->get_child_count(),
                        new tree_navigation_item(QVector<QVariant>() << QString::fromStdString(added_gate->get_name()) << added_gate->get_id() << QString::fromStdString(added_gate->get_type()),
                                                 tree_navigation_item::item_type::gate,
                                                 gate_structure_item));
            break;
        }
    }
}

void tree_navigation_model::handle_module_gate_removed(std::shared_ptr<module> m, u32 removed_gate)
{
    for (int i = 0; i < m_modules_item->get_child_count(); i++)
    {
        auto current_submod_item = m_modules_item->get_child(i);
        if (current_submod_item->data(ID_COLUMN).toInt() == (int)m->get_id())
        {
            auto gate_structure_item = current_submod_item->get_child(0);
            for (int j = 0; j < gate_structure_item->get_child_count(); j++)
            {
                auto current_gate_item = gate_structure_item->get_child(j);
                if (current_gate_item->data(ID_COLUMN).toInt() == (int)removed_gate)
                {
                    remove_item(current_gate_item);
                    break;
                }
            }
            break;
        }
    }

}

void tree_navigation_model::handle_module_name_changed(std::shared_ptr<module> m)
{
    for (int i = 0; i < m_modules_item->get_child_count(); i++)
    {
        auto current_submod_item = m_modules_item->get_child(i);
        if (current_submod_item->data(ID_COLUMN) == (int)m->get_id())
        {
            current_submod_item->set_data(NAME_COLUMN, QString::fromStdString(m->get_name()));
            const auto index = get_modelindex(current_submod_item);
            dataChanged(index, index, QVector<int>() << Qt::DisplayRole);
        }
    }
}

void tree_navigation_model::handle_module_removed(std::shared_ptr<module> m)
{
    for (int i = 0; i < m_modules_item->get_child_count(); i++)
    {
        auto current_submod_item = m_modules_item->get_child(i);
        if (current_submod_item->data(ID_COLUMN).toInt() == (int)m->get_id())
        {
            remove_item(current_submod_item);
            break;
        }
    }
    m_modules_item->set_data(NAME_COLUMN, "modules : " + QString::number(g_netlist->get_modules().size()));
    dataChanged(get_modelindex(m_modules_item), get_modelindex(m_modules_item), QVector<int>() << Qt::DisplayRole);

}

void tree_navigation_model::handle_gate_created(std::shared_ptr<gate> g)
{
    tree_navigation_item* new_gate_item =
        new tree_navigation_item(QVector<QVariant>() << QString::fromStdString(g->get_name()) << (int)g->get_id() << QString::fromStdString(g->get_type()),
                                 tree_navigation_item::item_type::gate,
                                 m_gates_item);
    m_gates_item->set_data(NAME_COLUMN, "Gates : " + QString::number(g_netlist->get_gates().size()));
    dataChanged(get_modelindex(m_gates_item), get_modelindex(m_gates_item), QVector<int>() << Qt::DisplayRole);
    for (int i = 0; i < m_gates_item->get_child_count(); i++)
    {
        if (m_gates_item->get_child(i)->data(NAME_COLUMN).toString() > new_gate_item->data(NAME_COLUMN).toString())
        {
            insert_item(m_gates_item, i, new_gate_item);
            return;
        }
    }
    insert_item(m_gates_item, m_gates_item->get_child_count(), new_gate_item);

}

void tree_navigation_model::handle_gate_removed(std::shared_ptr<gate> g)
{
    m_gates_item->set_data(NAME_COLUMN, "Gates : " + QString::number(g_netlist->get_gates().size()));
    dataChanged(get_modelindex(m_gates_item), get_modelindex(m_gates_item), QVector<int>() << Qt::DisplayRole);
    for (int i = 0; i < m_gates_item->get_child_count(); i++)
    {
        auto current_gate_item = m_gates_item->get_child(i);
        if ((int)g->get_id() == current_gate_item->data(ID_COLUMN).toInt())
        {
            remove_item(current_gate_item);
            break;
        }
    }
}

void tree_navigation_model::handle_gate_name_changed(std::shared_ptr<gate> g)
{
    for (int i = 0; i < m_gates_item->get_child_count(); i++)
    {
        if (m_gates_item->get_child(i)->data(ID_COLUMN).toInt() == (int)g->get_id())
        {
            m_gates_item->get_child(i)->set_data(NAME_COLUMN, QString::fromStdString(g->get_name()));
            const auto modelindex = get_modelindex(m_gates_item->get_child(i));
            dataChanged(modelindex, modelindex, QVector<int>() << Qt::DisplayRole);
        }
    }
    //go through the modules and check if any submod has the gate in it to change to name accordingly
    for (int i = 0; i < m_modules_item->get_child_count(); i++)
    {
        auto current_submod_item = m_modules_item->get_child(i);
        if (g_netlist->get_module_by_id(current_submod_item->data(ID_COLUMN).toInt())->contains_gate(g))
        {
            auto submod_gates_item = current_submod_item->get_child(0);
            for (int j = 0; j < submod_gates_item->get_child_count(); j++)
            {
                if (submod_gates_item->get_child(j)->data(ID_COLUMN).toInt() == (int)g->get_id())
                {
                    submod_gates_item->get_child(j)->set_data(NAME_COLUMN, QString::fromStdString(g->get_name()));
                    const auto modelindex = get_modelindex(submod_gates_item->get_child(j));
                    dataChanged(modelindex, modelindex, QVector<int>() << Qt::DisplayRole);
                }
            }
        }
    }
}

void tree_navigation_model::handle_net_created(std::shared_ptr<net> n)
{
    tree_navigation_item* new_net_item =
        new tree_navigation_item(QVector<QVariant>() << QString::fromStdString(n->get_name()) << (int)n->get_id() << "", tree_navigation_item::item_type::net, m_nets_item);
    m_nets_item->set_data(NAME_COLUMN, "Nets : " + QString::number(g_netlist->get_nets().size()));
    dataChanged(get_modelindex(m_nets_item), get_modelindex(m_nets_item), QVector<int>() << Qt::DisplayRole);
    for (int i = 2; i < m_nets_item->get_child_count(); i++)
    {
        if (m_nets_item->get_child(i)->data(NAME_COLUMN).toString() > new_net_item->data(NAME_COLUMN).toString())
        {
            insert_item(m_nets_item, i, new_net_item);
            return;
        }
    }
    insert_item(m_nets_item, m_nets_item->get_child_count(), new_net_item);
}

void tree_navigation_model::handle_net_removed(std::shared_ptr<net> n)
{
    m_nets_item->set_data(NAME_COLUMN, "Nets : " + QString::number(g_netlist->get_nets().size()));
    dataChanged(get_modelindex(m_nets_item), get_modelindex(m_nets_item), QVector<int>() << Qt::DisplayRole);
    //cant check if its a global input/output net, it may have lost its type
    for (int i = 0; i < m_global_input_nets_item->get_child_count(); i++)
    {
        if ((int)n->get_id() == m_global_input_nets_item->get_child(i)->data(ID_COLUMN).toInt())
        {
            remove_item(m_global_input_nets_item->get_child(i));
            return;
        }
    }
    for (int i = 0; i < m_global_output_nets_item->get_child_count(); i++)
    {
        if ((int)n->get_id() == m_global_output_nets_item->get_child(i)->data(ID_COLUMN).toInt())
        {
            remove_item(m_global_output_nets_item->get_child(i));
            return;
        }
    }
    for (int i = 2; i < m_nets_item->get_child_count(); i++)
    {
        if ((int)n->get_id() == m_nets_item->get_child(i)->data(ID_COLUMN).toInt())
        {
            remove_item(m_nets_item->get_child(i));
            return;
        }
    }
}

void tree_navigation_model::handle_net_name_changed(std::shared_ptr<net> n)
{
    if (g_netlist->is_global_input_net(n))
    {
        for (int i = 0; i < m_global_input_nets_item->get_child_count(); i++)
        {
            if (m_global_input_nets_item->get_child(i)->data(ID_COLUMN).toInt() == (int)n->get_id())
            {
                m_global_input_nets_item->get_child(i)->set_data(NAME_COLUMN, QString::fromStdString(n->get_name()));
                dataChanged(get_modelindex(m_global_input_nets_item->get_child(i)), get_modelindex(m_global_input_nets_item->get_child(i)), QVector<int>() << Qt::DisplayRole);
                break;
            }
        }
    }
    if (g_netlist->is_global_output_net(n))
    {
        for (int i = 0; i < m_global_output_nets_item->get_child_count(); i++)
        {
            if (m_global_output_nets_item->get_child(i)->data(ID_COLUMN).toInt() == (int)n->get_id())
            {
                m_global_output_nets_item->get_child(i)->set_data(NAME_COLUMN, QString::fromStdString(n->get_name()));
                dataChanged(get_modelindex(m_global_output_nets_item->get_child(i)), get_modelindex(m_global_output_nets_item->get_child(i)), QVector<int>() << Qt::DisplayRole);
                break;
            }
        }
    }
    else
    {
        for (int i = 2; i < m_nets_item->get_child_count(); i++)
        {
            if (m_nets_item->get_child(i)->data(ID_COLUMN).toInt() == (int)n->get_id())
            {
                m_nets_item->get_child(i)->set_data(NAME_COLUMN, QString::fromStdString(n->get_name()));
                auto modelindex = get_modelindex(m_nets_item->get_child(i));
                dataChanged(modelindex, modelindex, QVector<int>() << Qt::DisplayRole);
            }
        }
    }

}

void tree_navigation_model::setup_model_data()
{
    QString design_name       = QString::fromStdString(g_netlist->get_input_filename().filename().string());
    m_top_design_item         = new tree_navigation_item(QVector<QVariant>() << design_name << ""
                                                                     << "",
                                                 tree_navigation_item::item_type::structure,
                                                 m_root_item);
    m_gates_item              = new tree_navigation_item(QVector<QVariant>() << "Gates : " + QString::number(g_netlist->get_gates().size()) << ""
                                                                << "",
                                            tree_navigation_item::item_type::structure,
                                            m_top_design_item);
    m_nets_item               = new tree_navigation_item(QVector<QVariant>() << "Nets : " + QString::number(g_netlist->get_nets().size()) << ""
                                                               << "",
                                           tree_navigation_item::item_type::structure,
                                           m_top_design_item);
    m_global_input_nets_item  = new tree_navigation_item(QVector<QVariant>() << "Global Inputs"
                                                                            << ""
                                                                            << "",
                                                        tree_navigation_item::item_type::structure,
                                                        m_nets_item);
    m_global_output_nets_item = new tree_navigation_item(QVector<QVariant>() << "Global Outputs"
                                                                             << ""
                                                                             << "",
                                                         tree_navigation_item::item_type::structure,
                                                         m_nets_item);
    m_modules_item         = new tree_navigation_item(QVector<QVariant>() << "modules : " + QString::number(g_netlist->get_modules().size()) << ""
                                                                     << "",
                                                 tree_navigation_item::item_type::structure,
                                                 m_top_design_item);

    m_root_item->insert_child(0, m_top_design_item);
    m_top_design_item->insert_child(0, m_gates_item);
    m_top_design_item->insert_child(1, m_nets_item);
    m_nets_item->insert_child(0, m_global_input_nets_item);
    m_nets_item->insert_child(1, m_global_output_nets_item);
    m_top_design_item->insert_child(2, m_modules_item);

    //****Insert sorted gates into the model
    struct gate_comparison
    {
        inline bool operator()(std::shared_ptr<gate>& g1, std::shared_ptr<gate>& g2)
        {
            return (QString::fromStdString(g1->get_name()).toLower() < QString::fromStdString(g2->get_name()).toLower());
        }
    };
    auto gates = g_netlist->get_gates();    //neccessary
    std::vector<std::shared_ptr<gate>> sorted_gates(gates.begin(), gates.end());
    std::sort(sorted_gates.begin(), sorted_gates.end(), gate_comparison());

    for (const auto& _gate : sorted_gates)
    {
        tree_navigation_item* gate_tree_item = new tree_navigation_item(
            QVector<QVariant>() << QString::fromStdString(_gate->get_name()) << (int)_gate->get_id() << QString::fromStdString(_gate->get_type()), tree_navigation_item::item_type::gate, m_gates_item);
        m_gates_item->insert_child(m_gates_item->get_child_count(), gate_tree_item);
    }

    //****Insert sorted nets into the model
    struct net_comparison
    {
        inline bool operator()(std::shared_ptr<net>& n1, std::shared_ptr<net>& n2)
        {
            return (QString::fromStdString(n1->get_name()).toLower() < QString::fromStdString(n2->get_name()).toLower());
        }
    };
    auto nets = g_netlist->get_nets();
    std::vector<std::shared_ptr<net>> sorted_nets(nets.begin(), nets.end());
    std::sort(sorted_nets.begin(), sorted_nets.end(), net_comparison());

    for (const auto& _net : sorted_nets)
    {
        if (g_netlist->is_global_input_net(_net))
        {
            tree_navigation_item* item =
                new tree_navigation_item(QVector<QVariant>() << QString::fromStdString(_net->get_name()) << (int)_net->get_id() << "", tree_navigation_item::item_type::net, m_global_input_nets_item);
            m_global_input_nets_item->insert_child(m_global_input_nets_item->get_child_count(), item);
            continue;
        }
        if (g_netlist->is_global_output_net(_net))
        {
            tree_navigation_item* item =
                new tree_navigation_item(QVector<QVariant>() << QString::fromStdString(_net->get_name()) << (int)_net->get_id() << "", tree_navigation_item::item_type::net, m_global_output_nets_item);
            m_global_output_nets_item->insert_child(m_global_output_nets_item->get_child_count(), item);
            continue;
        }

        tree_navigation_item* item =
            new tree_navigation_item(QVector<QVariant>() << QString::fromStdString(_net->get_name()) << (int)_net->get_id() << "", tree_navigation_item::item_type::net, m_nets_item);
        m_nets_item->insert_child(m_nets_item->get_child_count(), item);
    }

    for (const auto& _module : g_netlist->get_modules())
    {
        tree_navigation_item* current_module_tree_item = new tree_navigation_item(
            QVector<QVariant>() << QString::fromStdString(_module->get_name()) << (int)_module->get_id() << "", tree_navigation_item::item_type::module, m_modules_item);
        m_modules_item->insert_child(m_modules_item->get_child_count(), current_module_tree_item);

        tree_navigation_item* sub_gates_item = new tree_navigation_item(QVector<QVariant>() << "Gates"
                                                                                            << ""
                                                                                            << "",
                                                                        tree_navigation_item::item_type::ignore,
                                                                        current_module_tree_item);
        current_module_tree_item->insert_child(0, sub_gates_item);

        /*
        tree_navigation_item* sub_nets_item  = new tree_navigation_item(QVector<QVariant>() << "Nets"
                                                                                           << ""
                                                                                           << "",
                                                                       tree_navigation_item::item_type::ignore,
                                                                       current_module_tree_item);
        current_module_tree_item->insert_child(1, sub_nets_item);
        */

        for (const auto& _gate : _module->get_gates())
        {
            tree_navigation_item* tmp_gate =
                new tree_navigation_item(QVector<QVariant>() << QString::fromStdString(_gate->get_name()) << (int)_gate->get_id() << QString::fromStdString(_gate->get_type()),
                                         tree_navigation_item::item_type::gate,
                                         sub_gates_item);
            sub_gates_item->insert_child(sub_gates_item->get_child_count(), tmp_gate);
        }

        /*
        for (const auto& _net : _module->get_nets())
        {
            tree_navigation_item* tmp_net =
                new tree_navigation_item(QVector<QVariant>() << QString::fromStdString(_net->get_name()) << (int)_net->get_id() << "", tree_navigation_item::item_type::net, sub_nets_item);
            sub_nets_item->insert_child(sub_nets_item->get_child_count(), tmp_net);
        }
        */
    }
}

tree_navigation_item* tree_navigation_model::get_item(const QModelIndex& index) const
{
    if (index.isValid())
    {
        tree_navigation_item* item = static_cast<tree_navigation_item*>(index.internalPointer());
        if (item)
            return item;
    }
    return m_root_item;
}

QModelIndex tree_navigation_model::get_modelindex(tree_navigation_item* item)
{
    if (item == m_root_item)
        return QModelIndex();

    int row    = item->get_row_number();
    int column = 0;
    return createIndex(row, column, item);
}

QList<QModelIndex> tree_navigation_model::get_modelindexes_for_row(tree_navigation_item* item)
{
    QList<QModelIndex> row_indexes;
    if (item == m_root_item)
        return row_indexes;

    int row = item->get_row_number();
    for (int column = 0; column < columnCount(); column++)
        row_indexes.append(createIndex(row, column, item));
    return row_indexes;
}

void tree_navigation_model::insert_item(tree_navigation_item* parent, int position, tree_navigation_item* item)
{
    const QModelIndex parent_index = get_modelindex(parent);
    int firstRow                   = position;
    int lastRow                    = position;

    beginInsertRows(parent_index, firstRow, lastRow);
    parent->insert_child(position, item);
    endInsertRows();
}

void tree_navigation_model::remove_item(tree_navigation_item* item)
{
    tree_navigation_item* parent  = item->get_parent();
    const QModelIndex parentIndex = get_modelindex(parent);
    int pos                       = item->get_row_number();
    int firstRow                  = pos;
    int lastRow                   = pos;
    beginRemoveRows(parentIndex, firstRow, lastRow);
    parent->remove_children(pos, 1);
    endRemoveRows();
}

void tree_navigation_model::load_data_settings()
{
    m_structured_font = QFont();
    m_structured_font.setBold(true);
    m_structured_font.setPixelSize(15);
    m_design_icon = gui_utility::get_styled_svg_icon(old_graphics_item_qss_adapter::instance()->tree_navigation_open_folder_style(), old_graphics_item_qss_adapter::instance()->tree_navigation_open_folder_path());
    //m_design_icon = gui_utility::get_styled_svg_icon("all->#000000", ":/icons/open");
    //m_design_icon = QIcon(":/icons/open");
}
