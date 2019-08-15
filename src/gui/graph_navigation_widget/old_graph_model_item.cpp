#include "graph_navigation_widget/old_graph_model_item.h"

old_graph_model_item::old_graph_model_item(std::shared_ptr<gate> gate)
{
    refgate = gate;
    refNet  = nullptr;
    name    = QString::fromStdString(gate->get_name());
    id      = QString::number(gate->get_id());
    type    = QString::fromStdString(gate->get_type());
    //location = QString::fromStdString(gate->get_location());

    auto loc = gate->get_data_by_key("generic", "LOC");
    if (!std::get<1>(loc).empty())
        location = QString::fromStdString(std::get<1>(loc));

    i_type = item_type::gate;
}

old_graph_model_item::old_graph_model_item(std::shared_ptr<net> net)
{
    refNet  = net;
    refgate = nullptr;
    name    = QString::fromStdString(net->get_name());
    id      = QString::number(net->get_id());

    i_type = item_type::net;
}

old_graph_model_item::~old_graph_model_item()
{
}

void old_graph_model_item::setgate(std::shared_ptr<gate> gate)
{
    refgate = gate;
    refNet  = nullptr;
    name    = QString::fromStdString(gate->get_name());
    id      = QString::number(gate->get_id());
    type    = QString::fromStdString(gate->get_type());
    //location = QString::fromStdString(gate->get_location());

    auto loc = gate->get_data_by_key("generic", "LOC");
    if (!std::get<1>(loc).empty())
        location = QString::fromStdString(std::get<1>(loc));

    i_type = item_type::gate;
}

void old_graph_model_item::setNet(std::shared_ptr<net> net)
{
    refNet  = net;
    refgate = nullptr;
    name    = QString::fromStdString(net->get_name());
    id      = QString::number(net->get_id());

    i_type = item_type::net;
}

QString old_graph_model_item::getName()
{
    return name;
}

QString old_graph_model_item::getID()
{
    return id;
}

QString old_graph_model_item::getType()
{
    return type;
}

QString old_graph_model_item::getLocation()
{
    if (location.isEmpty())
        return QString("N/A");
    return location;
}

item_type old_graph_model_item::getItemType()
{
    return i_type;
}
