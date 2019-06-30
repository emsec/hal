#include "graph_navigation_widget/graph_navigation_model.h"
#include "gui_globals.h"
#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/module.h"
#include <QDebug>

graph_navigation_model::graph_navigation_model(QObject* parent) : QAbstractItemModel(parent)
{
    m_root_item = new graph_navigation_item("Root", 0, graph_navigation_item::item_type::ignore);
    setupModelData();
}

graph_navigation_model::~graph_navigation_model()
{
    //Note: Even though there are other pointers such as m_top_level_item or m_net_item, they are destroyed
    //through the destruction of the root-item (chain-reaction, because every navigation item deletes its children)
    delete m_root_item;
}

QVariant graph_navigation_model::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (role == Qt::UserRole && index.column() == 0)
    {
        graph_navigation_item* item = static_cast<graph_navigation_item*>(index.internalPointer());
        return QVariant::fromValue(item->id());
    }

    if (role == Qt::UserRole + 1 && index.column() == 0)
    {
        graph_navigation_item* item = static_cast<graph_navigation_item*>(index.internalPointer());
        return QVariant::fromValue(item->type());
    }

    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    graph_navigation_item* item = static_cast<graph_navigation_item*>(index.internalPointer());
    return item->data(index.column());
}

Qt::ItemFlags graph_navigation_model::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index);
}

QVariant graph_navigation_model::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    Q_UNUSED(orientation)
    Q_UNUSED(role)
    return QVariant();
}

QModelIndex graph_navigation_model::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    graph_navigation_item* parent_item;

    if (!parent.isValid())
        parent_item = m_root_item;
    else
        parent_item = static_cast<graph_navigation_item*>(parent.internalPointer());

    graph_navigation_item* child_item = parent_item->child(row);
    if (child_item)
        return createIndex(row, column, child_item);
    else
        return QModelIndex();
}

int graph_navigation_model::rowCount(const QModelIndex& parent) const
{
    graph_navigation_item* parent_item;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parent_item = m_root_item;
    else
        parent_item = static_cast<graph_navigation_item*>(parent.internalPointer());

    return parent_item->childCount();
}

int graph_navigation_model::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return 1;
}

void graph_navigation_model::setupModelData()
{
    //TODO React to Gate / Net / module number changes
    QString design_name = QString::fromStdString(g_netlist->get_input_filename().filename().string());
    m_top_level_item    = new graph_navigation_item(design_name, 0, graph_navigation_item::item_type::ignore, m_root_item);
    QString text        = "gates : " + QString::number(g_netlist->get_gates().size());
    m_gate_item         = new graph_navigation_item(text, 0, graph_navigation_item::item_type::ignore, m_top_level_item);
    text                = "nets : " + QString::number(g_netlist->get_nets().size());
    m_net_item          = new graph_navigation_item(text, 0, graph_navigation_item::item_type::ignore, m_top_level_item);
    text                = "modules : " + QString::number(g_netlist->get_modules().size());
    m_module_item    = new graph_navigation_item(text, 0, graph_navigation_item::item_type::ignore, m_top_level_item);

    struct less_than_gate
    {
        inline bool operator()(std::shared_ptr<gate>& g1, std::shared_ptr<gate>& g2)
        {
            return (QString::fromStdString(g1->get_name()).toLower() < QString::fromStdString(g2->get_name()).toLower());
        }
    };

    struct less_than_net
    {
        inline bool operator()(std::shared_ptr<net>& n1, std::shared_ptr<net>& n2)
        {
            return (QString::fromStdString(n1->get_name()).toLower() < QString::fromStdString(n2->get_name()).toLower());
        }
    };

    std::vector<std::shared_ptr<gate>> sorted_gates;

    for (auto gate1 : g_netlist->get_gates())
        sorted_gates.push_back(gate1);

    std::sort(sorted_gates.begin(), sorted_gates.end(), less_than_gate());

    for (auto gate : sorted_gates)
    {
        text = QString::fromStdString(gate->get_name()) + " : " + QString::fromStdString(gate->get_type());
        new graph_navigation_item(text, gate->get_id(), graph_navigation_item::item_type::gate, m_gate_item);
    }

    std::vector<std::shared_ptr<net>> sorted_nets;

    for (auto net1 : g_netlist->get_nets())
        sorted_nets.push_back(net1);

    std::sort(sorted_nets.begin(), sorted_nets.end(), less_than_net());

    std::vector<std::shared_ptr<net>> sorted_global_input_nets;
    std::vector<std::shared_ptr<net>> sorted_global_output_nets;
    std::vector<std::shared_ptr<net>> sorted_global_inout_nets;

    for (std::shared_ptr<net> net2 : sorted_nets)
    {
        if (g_netlist->is_global_inout_net(net2))
            sorted_global_inout_nets.push_back(net2);
        else
        {
            if (g_netlist->is_global_input_net(net2))
                sorted_global_input_nets.push_back(net2);
            else
            {
                if (g_netlist->is_global_output_net(net2))
                    sorted_global_output_nets.push_back(net2);
            }
        }
    }

    text                = "global inputs : " + QString::number(sorted_global_input_nets.size());
    m_global_input_item = new graph_navigation_item(text, 0, graph_navigation_item::item_type::ignore, m_net_item);
    for (std::shared_ptr<net> net3 : sorted_global_input_nets)
        new graph_navigation_item(QString::fromStdString(net3->get_name()), net3->get_id(), graph_navigation_item::item_type::net, m_global_input_item);

    text                 = "global outputs : " + QString::number(sorted_global_output_nets.size());
    m_global_output_item = new graph_navigation_item(text, 0, graph_navigation_item::item_type::ignore, m_net_item);
    for (std::shared_ptr<net> net4 : sorted_global_output_nets)
        new graph_navigation_item(QString::fromStdString(net4->get_name()), net4->get_id(), graph_navigation_item::item_type::net, m_global_output_item);

    text                = "global inouts : " + QString::number(sorted_global_inout_nets.size());
    m_global_inout_item = new graph_navigation_item(text, 0, graph_navigation_item::item_type::ignore, m_net_item);
    for (std::shared_ptr<net> net5 : sorted_global_inout_nets)
        new graph_navigation_item(QString::fromStdString(net5->get_name()), net5->get_id(), graph_navigation_item::item_type::net, m_global_inout_item);

    std::vector<std::shared_ptr<net>> sorted_routed_nets;

    for (std::shared_ptr<net> net6 : sorted_nets)
    {
        if (!net6->is_unrouted())
            sorted_routed_nets.push_back(net6);
    }

    //std::sort(sorted_unrouted_nets.begin(), sorted_unrouted_nets.end(), less_than_net());

    for (std::shared_ptr<net> net : sorted_routed_nets)
    {
        //        QString type = "";
        //        if (g_netlist->is_global_inout_net(net))
        //            type = " : inout";
        //        else
        //        {
        //            if (g_netlist->is_global_input_net(net))
        //                type = " : input";
        //            else
        //            {
        //                if (g_netlist->is_global_output_net(net))
        //                    type = " : output";
        //            }
        //        }

        //        if (g_netlist->is_global_input_net(net))
        //            type = " : input";
        //        if (g_netlist->is_global_output_net(net))
        //            type = " : output";
        //        if (g_netlist->is_global_inout_net(net))
        //            type = " : inout";

        //        text = QString::fromStdString(net->get_name()) + type;
        text = QString::fromStdString(net->get_name());
        new graph_navigation_item(text, net->get_id(), graph_navigation_item::item_type::net, m_net_item);
    }

    for (auto const module : g_netlist->get_modules())
    {
        graph_navigation_item* module_item =
            new graph_navigation_item(QString::fromStdString(module->get_name()) + " [" + QString::number(module->get_id()) + "]", 0, graph_navigation_item::item_type::module, m_module_item);
        graph_navigation_item* module_gates_item = new graph_navigation_item("gates", 0, graph_navigation_item::item_type::ignore, module_item);
        graph_navigation_item* module_nets_item  = new graph_navigation_item("nets", 0, graph_navigation_item::item_type::ignore, module_item);

        for (auto const gate : module->get_gates())
            new graph_navigation_item(QString::fromStdString(gate->get_name()), gate->get_id(), graph_navigation_item::item_type::gate, module_gates_item);

        // todo modules do not have nets anymore
        //for (auto const net : module->get_nets())
        //    new graph_navigation_item(QString::fromStdString(net->get_name()), net->get_id(), graph_navigation_item::item_type::net, module_nets_item);
    }
}

QModelIndex graph_navigation_model::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    graph_navigation_item* child_item  = static_cast<graph_navigation_item*>(index.internalPointer());
    graph_navigation_item* parent_item = child_item->parent_item();

    if (parent_item == m_root_item)
        return QModelIndex();

    return createIndex(parent_item->row(), 0, parent_item);
}

QModelIndex* graph_navigation_model::get_index_by_logger_name(const std::string& logger_name)
{
    QModelIndexList match_result = this->match(index(0, 0, QModelIndex()), Qt::UserRole, QVariant::fromValue(QString::fromStdString(logger_name)), 1, Qt::MatchRecursive | Qt::MatchExactly);
    if (!match_result.empty())
    {
        QModelIndex* index = new QModelIndex(match_result.at(0));
        return index;
    }
    else
    {
        return nullptr;
    }
}

QModelIndexList graph_navigation_model::corresponding_indexes(const QList<u32>& gate_ids, const QList<u32>& net_ids, const QList<u32>& module_ids)
{
    QModelIndexList list;
    match_ids_recursive(m_root_item, list, gate_ids, net_ids, module_ids);
    return list;
    //horribly inefficient, TODO optimize
}

void graph_navigation_model::update_modules()
{
    delete m_module_item;
    QString text     = "modules : " + QString::number(g_netlist->get_modules().size());
    m_module_item = new graph_navigation_item(text, 0, graph_navigation_item::item_type::ignore, m_top_level_item);

    for (auto const module : g_netlist->get_modules())
    {
        graph_navigation_item* module_item =
            new graph_navigation_item(QString::fromStdString(module->get_name()) + " [" + QString::number(module->get_id()) + "]", 0, graph_navigation_item::item_type::module, m_module_item);
        graph_navigation_item* module_gates_item = new graph_navigation_item("gates", 0, graph_navigation_item::item_type::ignore, module_item);
        graph_navigation_item* module_nets_item  = new graph_navigation_item("nets", 0, graph_navigation_item::item_type::ignore, module_item);

        for (auto const gate : module->get_gates())
            new graph_navigation_item(QString::fromStdString(gate->get_name()), gate->get_id(), graph_navigation_item::item_type::gate, module_gates_item);

        // todo modules do not have nets anymore
        // for (auto const net : module->get_nets())
        //    new graph_navigation_item(QString::fromStdString(net->get_name()), net->get_id(), graph_navigation_item::item_type::net, module_nets_item);
    }
}

void graph_navigation_model::match_ids_recursive(graph_navigation_item* item, QModelIndexList& list, const QList<u32>& gate_ids, const QList<u32>& net_ids, const QList<u32>& module_ids)
{
    QList<u32> const* id_list = nullptr;
    switch (item->type())
    {
        case graph_navigation_item::item_type::ignore:
            break;
        case graph_navigation_item::item_type::gate:
            id_list = &gate_ids;
            break;
        case graph_navigation_item::item_type::net:
            id_list = &net_ids;
            break;
        case graph_navigation_item::item_type::module:
            id_list = &module_ids;
            break;
    }

    if (id_list)
    {
        for (auto id : *id_list)
        {
            if (item->id() == id)
            {
                list.append(createIndex(item->row(), 0, item));
                break;
            }
        }
    }

    int childcount = item->childCount();

    for (int i = 0; i < childcount; i++)
    {
        match_ids_recursive(item->child(i), list, gate_ids, net_ids, module_ids);
    }
}

//hal_graph_navigation_item *hal_graph_navigation_model::add_item(QString name, int id, const std::string &parent_item_logger_name)
//{
//    hal_graph_navigation_item *item = nullptr;
//    if (parent_item_logger_name == "null") {
//        beginInsertRows(QModelIndex(), 1, 1);
//        item = new hal_graph_navigation_item(name, id, m_root_item);
//        m_root_item->insert_child(1, item);
//        endInsertRows();

//        //TODO check if task items are still active before deleting
//        //TODO also delete child items if parent is deleted
//        if (m_top_level_items.full()) {
//            beginRemoveRows(QModelIndex(), m_root_item->childCount() - 1, m_root_item->childCount() - 1);
//            delete m_top_level_items.front();
//            endRemoveRows();
//        }
//        m_top_level_items.push_back(item);
//    } else {
//        QModelIndex *index = get_index_by_logger_name(parent_item_logger_name);
//        if (index != nullptr) {
//            hal_graph_navigation_item *parent_item_pointer = static_cast<hal_graph_navigation_item *>(index->internalPointer());
//            beginInsertRows(*index, 0, 0);
//            item = new hal_graph_navigation_item(name, id, parent_item_pointer);
//            parent_item_pointer->insert_child(0, item);
//            endInsertRows();
//            delete index;
//        } else {
//            //TODO
//            //log failed to create task_status_item
//            return nullptr;
//        }
//    }
//    return item;
//}

//void hal_graph_navigation_model::update_item_log(spdlog::level::level_enum t,
//                                            const std::string &logger_name,
//                                            std::string const &msg)
//{
//    QModelIndex *index = get_index_by_logger_name(logger_name);
//    if (index != nullptr) {
//        hal_graph_navigation_item *item = static_cast<hal_graph_navigation_item *>(index->internalPointer());
//        hal_task_log_entry *entry = new hal_task_log_entry(msg, t);
//        item->append_status_log_entry(entry);
//        delete index;
//    } else {
//        //TODO
//        //do something to handle this case
//        //(id not found)
//    }
//}

//void hal_graph_navigation_model::update_item_progress(const std::string &logger_name, int progress)
//{
//    QModelIndex *index = get_index_by_logger_name(logger_name);
//    if (index != nullptr) {
//        hal_graph_navigation_item *item = static_cast<hal_graph_navigation_item *>(index->internalPointer());
//        item->set_progress(progress);
//        Q_EMIT dataChanged(*index,
//                         createIndex(index->row(), static_cast<int>(ColumnNumber::StatusColumn))); // not sure if needed
//        delete index;
//    } else {
//        //TODO
//        //do something to handle this case
//        //(id not found)
//    }
//}

//void hal_graph_navigation_model::update_item_status(const std::string &logger_name, QString status)
//{
//    QModelIndex *index = get_index_by_logger_name(logger_name);
//    if (index != nullptr) {
//        hal_graph_navigation_item *item = static_cast<hal_graph_navigation_item *>(index->internalPointer());
//        item->set_current_status(status);
//        Q_EMIT dataChanged(*index, *index);
//        delete index;
//    } else {
//        //TODO
//        //do something to handle this case
//        //(id not found)
//    }
//}
