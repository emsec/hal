#include "graph_manager/graph_model_item.h"

graph_model_item::graph_model_item(std::shared_ptr<gate> gate)
{
    refgate = gate;
    refNet  = nullptr;
    name    = QString::fromStdString(gate->get_name());
    id      = QString::number(gate->get_id());
    type    = QString::fromStdString(gate->get_type()->get_name());
    //location = QString::fromStdString(gate->get_location());

    auto loc = gate->get_data_by_key("generic", "LOC");
    if (!std::get<1>(loc).empty())
        location = QString::fromStdString(std::get<1>(loc));

    i_type = item_type::gate;
}

graph_model_item::graph_model_item(std::shared_ptr<net> net)
{
    refNet  = net;
    refgate = nullptr;
    name    = QString::fromStdString(net->get_name());
    id      = QString::number(net->get_id());

    i_type = item_type::net;
}

graph_model_item::~graph_model_item()
{
}

void graph_model_item::setgate(std::shared_ptr<gate> gate)
{
    refgate = gate;
    refNet  = nullptr;
    name    = QString::fromStdString(gate->get_name());
    id      = QString::number(gate->get_id());
    type    = QString::fromStdString(gate->get_type()->get_name());
    //location = QString::fromStdString(gate->get_location());

    auto loc = gate->get_data_by_key("generic", "LOC");
    if (!std::get<1>(loc).empty())
        location = QString::fromStdString(std::get<1>(loc));

    i_type = item_type::gate;
}

void graph_model_item::setNet(std::shared_ptr<net> net)
{
    refNet  = net;
    refgate = nullptr;
    name    = QString::fromStdString(net->get_name());
    id      = QString::number(net->get_id());

    i_type = item_type::net;
}

QString graph_model_item::getName()
{
    return name;
}

QString graph_model_item::getID()
{
    return id;
}

QString graph_model_item::getType()
{
    return type;
}

QString graph_model_item::getLocation()
{
    if (location.isEmpty())
        return QString("N/A");
    return location;
}

item_type graph_model_item::getItemType()
{
    return i_type;
}
