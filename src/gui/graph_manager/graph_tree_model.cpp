#include "graph_manager/graph_tree_model.h"
#include "netlist/net.h"
#include <tuple>

graph_tree_model::graph_tree_model(QObject* parent) : QAbstractItemModel(parent)
{
    rootItem = new tree_model_item();

    /*the header-label for the model*/
    m_columns << "Name"
              << "Value";
}

graph_tree_model::~graph_tree_model()
{
    delete rootItem;
}

QVariant graph_tree_model::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    tree_model_item* item = static_cast<tree_model_item*>(index.internalPointer());

    return item->data(index.column());
}

Qt::ItemFlags graph_tree_model::flags(const QModelIndex& index) const
{
    Qt::ItemFlags default_flags = QAbstractItemModel::flags(index);

    if (index.isValid())
        return Qt::ItemIsSelectable | default_flags;
    else
        return default_flags;
}

QVariant graph_tree_model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((role == Qt::DisplayRole) && (orientation == Qt::Horizontal) && (section >= 0) && (section < columnCount()))
        return m_columns.at(section);

    return QVariant();
}

QModelIndex graph_tree_model::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    tree_model_item* parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<tree_model_item*>(parent.internalPointer());

    tree_model_item* childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex graph_tree_model::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    tree_model_item* childItem  = static_cast<tree_model_item*>(index.internalPointer());
    tree_model_item* parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int graph_tree_model::rowCount(const QModelIndex& parent) const
{
    tree_model_item* parentItem;

    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<tree_model_item*>(parent.internalPointer());

    return parentItem->childCount();
}

int graph_tree_model::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return static_cast<tree_model_item*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

void graph_tree_model::setupModelData(std::shared_ptr<netlist> g)
{
    auto gloinput   = g->get_global_input_nets();
    auto globoutput = g->get_global_output_nets();

    /*Global-Input subtree*/
    tree_model_item* gloinputitem = new tree_model_item("Global Input", "Dummy", rootItem);
    add_net_set_to_a_parent_item(gloinput, gloinputitem);
    rootItem->appendChild(gloinputitem);

    /*Global Output subtree*/
    tree_model_item* glooutputitem = new tree_model_item("Global Output", "Dummy", rootItem);
    add_net_set_to_a_parent_item(globoutput, glooutputitem);
    rootItem->appendChild(glooutputitem);

    /*add all nets to the rootitem*/
    auto nets = g->get_nets();
    add_net_set_to_a_parent_item(nets, rootItem);
}

void graph_tree_model::add_net_set_to_a_parent_item(std::set<std::shared_ptr<net>> t, tree_model_item* parent)
{
    for (const auto& a : t)
    {
        /*toplvlnetitem, beneath the parent*/
        tree_model_item* item = new tree_model_item(a, parent);

        /*first, get the source*/
        auto src      = a->get_src();
        auto temp_src = src.gate;

        /*progress the source, if there is one*/
        if (temp_src != nullptr)
        {
            /*gate-item as a dummy*/
            tree_model_item* item2 = new tree_model_item("(SRC) " + QString::fromStdString(temp_src->get_name()), "", item);
            /*all the propertiegates*/
            tree_model_item* prop1 = new tree_model_item(temp_src, item_type_tree::id, item2);
            tree_model_item* prop2 = new tree_model_item(temp_src, item_type_tree::type, item2);
            tree_model_item* prop3 = new tree_model_item(temp_src, item_type_tree::location, item2);

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
            tree_model_item* item2 = new tree_model_item("(DST) " + QString::fromStdString(dst->get_name()), "", item);
            /*propertiegates*/
            tree_model_item* prop1 = new tree_model_item(dst, item_type_tree::id, item2);
            tree_model_item* prop2 = new tree_model_item(dst, item_type_tree::type, item2);
            tree_model_item* prop3 = new tree_model_item(dst, item_type_tree::location, item2);

            item2->appendChild(prop1);
            item2->appendChild(prop2);
            item2->appendChild(prop3);

            item->appendChild(item2);
        }
        parent->appendChild(item);
    }
}
