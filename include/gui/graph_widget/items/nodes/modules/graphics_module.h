#ifndef GRAPHICS_MODULE_H
#define GRAPHICS_MODULE_H

#include "gui/graph_widget/items/nodes/graphics_node.h"

#include <memory>

class module;

class graphics_module : public graphics_node
{
public:
    graphics_module(const std::shared_ptr<const module> m);

protected:
    struct module_pin // different struct for in / output pins ???
    {
        u32 net_id;
        QString pin_type;
        QString differentiation_number;
        QString occurrence_number;

        bool operator==(const module_pin& rhs) const
        {
            return this->net_id == rhs.net_id && this->pin_type == rhs.pin_type;
        }
    };

    QVector<module_pin> m_input_pins;
    QVector<module_pin> m_output_pins;

    static bool s_sort_pins;
    static bool s_show_differentiation_number;
    static bool s_show_occurence_number;
};

#endif // GRAPHICS_MODULE_H
