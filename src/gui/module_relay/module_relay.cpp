#include "module_relay/module_relay.h"

#include "gui_globals.h"
#include "netlist/module.h"

namespace hal
{
    module_relay::module_relay(QObject* parent) : QObject(parent), m_model(new module_model())
    {
        connect(&g_netlist_relay, &netlist_relay::module_event, this, &module_relay::handle_module_event);
    }

    void module_relay::set_module_color(const u32 id, const QColor& color)
    {
        Q_UNUSED(id);
        Q_UNUSED(color);
        //    if (!m_module_extensions.contains(id))
        //        return;

        //    m_module_extensions.value(id)->color = color;

        // SIGNAL OR
        // MANUAL UPDATE OF SUBSCRIBERS
    }

    void module_relay::set_module_compressed(const u32 id, const bool compressed)
    {
        Q_UNUSED(id);
        Q_UNUSED(compressed);
        //    if (!m_module_extensions.contains(id))
        //        return;

        //    m_module_extensions.value(id)->compressed = compressed;

        // SIGNAL OR
        // MANUAL UPDATE OF SUBSCRIBERS
    }

    void module_relay::set_module_hidden(const u32 id, const bool hidden)
    {
        Q_UNUSED(id);
        Q_UNUSED(hidden);
        //    if (!m_module_extensions.contains(id))
        //        return;

        //    m_module_extensions.value(id)->hidden = hidden;

        // SIGNAL OR
        // MANUAL UPDATE OF SUBSCRIBERS
    }

    void module_relay::handle_module_event(module_event_handler::event ev, std::shared_ptr<Module> object, u32 associated_data)
    {
        Q_UNUSED(associated_data);
        switch (ev)
        {
            case module_event_handler::event::created: {
                //< no associated_data

                module_item* parent_item = m_module_items.value(object->get_parent_module()->get_id());

                if (!parent_item)
                    return;    // SHOULD NOT BE POSSIBLE

                break;
            }
            case module_event_handler::event::removed: {
                //< no associated_data

                break;
            }
            case module_event_handler::event::name_changed: {
                //< no associated_data

                break;
            }
            case module_event_handler::event::type_changed: {
                //< no associated_data

                break;
            }
            case module_event_handler::event::parent_changed: {
                //< no associated_data

                break;
            }
            case module_event_handler::event::submodule_added: {
                //< associated_data = id of added module

                break;
            }
            case module_event_handler::event::submodule_removed: {
                //< associated_data = id of removed module

                break;
            }
            case module_event_handler::event::gate_assigned: {
                //< associated_data = id of inserted gate

                break;
            }
            case module_event_handler::event::gate_removed: {
                //< associated_data = id of removed gate

                break;
            }
            case module_event_handler::event::input_port_name_changed: {
                //< associated_data = id of respective net

                break;
            }
            case module_event_handler::event::output_port_name_changed: {
                //< associated_data = id of respective net

                break;
            }
        }
    }
}
