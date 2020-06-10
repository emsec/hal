#pragma once

#include "gui/graph_widget/items/nodes/graphics_node.h"

#include <memory>

class module;

class graphics_module : public graphics_node
{
public:
    explicit graphics_module(const std::shared_ptr<module> m);

protected:
    struct module_pin
    {
        QString name;
        u32 net_id;

        bool operator==(const module_pin& rhs) const
        {
            return this->name == rhs.name && this->net_id == rhs.net_id;
        }
    };

    QVector<module_pin> m_input_pins;
    QVector<module_pin> m_output_pins;
};
