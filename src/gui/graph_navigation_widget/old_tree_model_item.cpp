#include "graph_navigation_widget/old_tree_model_item.h"

old_tree_model_item::old_tree_model_item(old_tree_model_item* parent){Q_UNUSED(parent)}

//VARIABLE "name" SHADOWS A MEMBER
old_tree_model_item::old_tree_model_item(QString item_name, QString beschr, old_tree_model_item* parent)
{
    parentItem = parent;
    typ        = item_type_tree::dummy;

    item_data.append(item_name);
    item_data.append(beschr);
}

old_tree_model_item::old_tree_model_item(std::shared_ptr<net> net, old_tree_model_item* parent)
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

old_tree_model_item::old_tree_model_item(std::shared_ptr<gate> gate, old_tree_model_item* parent)
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

    type = QString::fromStdString(gate->get_type());

    item_data.append(name);
    item_data.append("");
}

old_tree_model_item::old_tree_model_item(std::shared_ptr<gate> ref, item_type_tree proptype, old_tree_model_item* parent)
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
            type = QString::fromStdString(ref->get_type());
            item_data.append("TYPE");
            item_data.append(type);
            break;
        default:
            break;    //hier dürfte er eh nicht reingehen da dieser konstruktor nur für properties gedacht ist
    }
}

old_tree_model_item::~old_tree_model_item()
{
}

void old_tree_model_item::appendChild(old_tree_model_item* child)
{
    children.append(child);
}

old_tree_model_item* old_tree_model_item::child(int row)
{
    return children.at(row);
}

int old_tree_model_item::childCount() const
{
    return children.count();
}

int old_tree_model_item::columnCount() const
{
    return 2;    //hardcoded now, may be changed later
}

QVariant old_tree_model_item::data(int column) const
{
    return item_data.at(column);
}

int old_tree_model_item::row() const
{
    if (parentItem)
        return parentItem->children.indexOf(const_cast<old_tree_model_item*>(this));

    return 0;
}

old_tree_model_item* old_tree_model_item::parent()
{
    return parentItem;
}

/*adapter-stuff*/
QString old_tree_model_item::getName()
{
    return name;
}

QString old_tree_model_item::getID()
{
    return id;
}

QString old_tree_model_item::getLocation()
{
    return location;
}

QString old_tree_model_item::getType()
{
    return type;
}

item_type_tree old_tree_model_item::getItemType()
{
    return typ;
}

/*needs to be implemented when needed, for example changing the graphgate/net value through the view*/
void old_tree_model_item::setID()
{
}

void old_tree_model_item::setLocation()
{
}

void old_tree_model_item::setName()
{
}

void old_tree_model_item::setType()
{
}
