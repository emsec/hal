#ifndef GRAPHICS_MODULE_H
#define GRAPHICS_MODULE_H

#include "gui/graph_widget/graphics_items/graphics_node.h"

#include <memory>

class module;

class graphics_module : public graphics_node
{
public:
    graphics_module(const std::shared_ptr<const module> m);

    virtual void set_visuals(const visuals& v) Q_DECL_OVERRIDE;

protected:
    struct module_pin
    {
        u32 net_id;
        QString pin_type;

        bool operator==(const module_pin& rhs) const
        {
            return this->net_id == rhs.net_id && this->pin_type == rhs.pin_type;
        }
    };

    QVector<module_pin> m_input_pins;
    QVector<module_pin> m_output_pins;
};

#endif // GRAPHICS_MODULE_H
