#include "graph_manager/tree_model_item.h"

tree_model_item::tree_model_item(tree_model_item* parent){Q_UNUSED(parent)}

//VARIABLE "name" SHADOWS A MEMBER
tree_model_item::tree_model_item(QString item_name, QString beschr, tree_model_item* parent)
{
    parentItem = parent;
    typ        = item_type_tree::dummy;

    item_data.append(item_name);
    item_data.append(beschr);
}

tree_model_item::tree_model_item(std::shared_ptr<net> net, tree_model_item* parent)
{
    parentItem = parent;
    refNet     = net;
    refgate    = nullptr;
    typ        = item_type_tree::net;

    name = QString::fromStdString(net->get_name());
    id   = QString::number(net->get_id());

    item_data.append(name);
    item_data.append("");
}

tree_model_item::tree_model_item(std::shared_ptr<gate> gate, tree_model_item* parent)
{
    parentItem = parent;
    refNet     = nullptr;
    refgate    = gate;
    typ        = item_type_tree::gate;

    name = QString::fromStdString(gate->get_name());
    id   = QString::number(gate->get_id());

    auto loc = gate->get_data_by_key("generic", "LOC");
    if (!std::get<1>(loc).empty())
        location = QString::fromStdString(std::get<1>(loc));

    type = QString::fromStdString(gate->get_type()->get_name());

    item_data.append(name);
    item_data.append("");
}

tree_model_item::tree_model_item(std::shared_ptr<gate> ref, item_type_tree proptype, tree_model_item* parent)
{
    parentItem = parent;
    refNet     = nullptr;
    refgate    = ref;
    typ        = proptype;

    /*needs to know which kind of property the item is supposed to be*/
    switch (proptype)
    {
        case item_type_tree::id:
            id = QString::number(ref->get_id());
            item_data.append("ID");
            item_data.append(id);
            break;
        case item_type_tree::location:
            location = QString::fromStdString(std::get<1>(ref->get_data_by_key("generic", "LOC")));
            item_data.append("LOCATION");
            item_data.append(location);
            break;
        case item_type_tree::type:
            type = QString::fromStdString(ref->get_type()->get_name());
            item_data.append("TYPE");
            item_data.append(type);
            break;
        default:
            break;    //hier dürfte er eh nicht reingehen da dieser konstruktor nur für properties gedacht ist
    }
}

tree_model_item::~tree_model_item()
{
}

void tree_model_item::appendChild(tree_model_item* child)
{
    children.append(child);
}

tree_model_item* tree_model_item::child(int row)
{
    return children.at(row);
}

int tree_model_item::childCount() const
{
    return children.count();
}

int tree_model_item::columnCount() const
{
    return 2;    //hardcoded now, may be changed later
}

QVariant tree_model_item::data(int column) const
{
    return item_data.at(column);
}

int tree_model_item::row() const
{
    if (parentItem)
        return parentItem->children.indexOf(const_cast<tree_model_item*>(this));

    return 0;
}

tree_model_item* tree_model_item::parent()
{
    return parentItem;
}

/*adapter-stuff*/
QString tree_model_item::getName()
{
    return name;
}

QString tree_model_item::getID()
{
    return id;
}

QString tree_model_item::getLocation()
{
    return location;
}

QString tree_model_item::getType()
{
    return type;
}

item_type_tree tree_model_item::getItemType()
{
    return typ;
}

/*needs to be implemented when needed, for example changing the graphgate/net value through the view*/
void tree_model_item::setID()
{
}

void tree_model_item::setLocation()
{
}

void tree_model_item::setName()
{
}

void tree_model_item::setType()
{
}
