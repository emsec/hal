#include "graph_navigation_widget/old_graph_tree_model.h"
#include "netlist/net.h"
#include <tuple>

old_graph_tree_model::old_graph_tree_model(QObject* parent) : QAbstractItemModel(parent)
{
    rootItem = new old_tree_model_item();

    /*the header-label for the model*/
    m_columns << "Name"
              << "Value";
}

old_graph_tree_model::~old_graph_tree_model()
{
    delete rootItem;
}

QVariant old_graph_tree_model::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    old_tree_model_item* item = static_cast<old_tree_model_item*>(index.internalPointer());

    return item->data(index.column());
}

Qt::ItemFlags old_graph_tree_model::flags(const QModelIndex& index) const
{
    Qt::ItemFlags default_flags = QAbstractItemModel::flags(index);

    if (index.isValid())
        return Qt::ItemIsSelectable | default_flags;
    else
        return default_flags;
}

QVariant old_graph_tree_model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((role == Qt::DisplayRole) && (orientation == Qt::Horizontal) && (section >= 0) && (section < columnCount()))
        return m_columns.at(section);

    return QVariant();
}

QModelIndex old_graph_tree_model::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    old_tree_model_item* parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<old_tree_model_item*>(parent.internalPointer());

    old_tree_model_item* childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex old_graph_tree_model::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    old_tree_model_item* childItem  = static_cast<old_tree_model_item*>(index.internalPointer());
    old_tree_model_item* parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int old_graph_tree_model::rowCount(const QModelIndex& parent) const
{
    old_tree_model_item* parentItem;

    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<old_tree_model_item*>(parent.internalPointer());

    return parentItem->childCount();
}

int old_graph_tree_model::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return static_cast<old_tree_model_item*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

void old_graph_tree_model::setupModelData(std::shared_ptr<netlist> g)
{
    auto gloinput   = g->get_global_input_nets();
    auto globoutput = g->get_global_output_nets();
    auto globInout  = g->get_global_inout_nets();

    /*Global-Input subtree*/
    old_tree_model_item* gloinputitem = new old_tree_model_item("Global Input", "Dummy", rootItem);
    add_net_set_to_a_parent_item(gloinput, gloinputitem);
    rootItem->appendChild(gloinputitem);

    /*Global Output subtree*/
    old_tree_model_item* glooutputitem = new old_tree_model_item("Global Output", "Dummy", rootItem);
    add_net_set_to_a_parent_item(globoutput, glooutputitem);
    rootItem->appendChild(glooutputitem);

    /*Global Input/Output subtree*/
    old_tree_model_item* gloinoutitem = new old_tree_model_item("Global InOut", "Dummy", rootItem);
    add_net_set_to_a_parent_item(globInout, gloinoutitem);
    rootItem->appendChild(gloinoutitem);

    /*add all nets to the rootitem*/
    auto nets = g->get_nets();
    add_net_set_to_a_parent_item(nets, rootItem);
}

void old_graph_tree_model::add_net_set_to_a_parent_item(std::set<std::shared_ptr<net>> t, old_tree_model_item* parent)
{
    for (const auto& a : t)
    {
        /*toplvlnetitem, beneath the parent*/
        old_tree_model_item* item = new old_tree_model_item(a, parent);

        /*first, get the source*/
        auto src      = a->get_src();
        auto temp_src = src.gate;

        /*progress the source, if there is one*/
        if (temp_src != nullptr)
        {
            /*gate-item as a dummy*/
            old_tree_model_item* item2 = new old_tree_model_item("(SRC) " + QString::fromStdString(temp_src->get_name()), "", item);
            /*all the propertiegates*/
            old_tree_model_item* prop1 = new old_tree_model_item(temp_src, item_type_tree::id, item2);
            old_tree_model_item* prop2 = new old_tree_model_item(temp_src, item_type_tree::type, item2);
            old_tree_model_item* prop3 = new old_tree_model_item(temp_src, item_type_tree::location, item2);

            item2->appendChild(prop1);
            item2->appendChild(prop2);
            item2->appendChild(prop3);

            item->appendChild(item2);
        }

        /*get all the destinations of the net*/
        for (const auto& tup : a->get_dsts())
        {
            auto dst = tup.gate;
            if (dst == nullptr)
                continue;
            /*the gate-dummy-item*/
            old_tree_model_item* item2 = new old_tree_model_item("(DST) " + QString::fromStdString(dst->get_name()), "", item);
            /*propertiegates*/
            old_tree_model_item* prop1 = new old_tree_model_item(dst, item_type_tree::id, item2);
            old_tree_model_item* prop2 = new old_tree_model_item(dst, item_type_tree::type, item2);
            old_tree_model_item* prop3 = new old_tree_model_item(dst, item_type_tree::location, item2);

            item2->appendChild(prop1);
            item2->appendChild(prop2);
            item2->appendChild(prop3);

            item->appendChild(item2);
        }
        parent->appendChild(item);
    }
}
