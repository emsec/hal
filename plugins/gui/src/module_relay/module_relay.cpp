#include "gui/module_relay/module_relay.h"

#include "gui/gui_globals.h"
#include "hal_core/netlist/module.h"

namespace hal
{
    ModuleRelay::ModuleRelay(QObject* parent) : QObject(parent), mModel(new ModuleModel())
    {
        connect(gNetlistRelay, &NetlistRelay::moduleEvent, this, &ModuleRelay::handleModuleEvent);
    }

    void ModuleRelay::setModuleColor(const u32 id, const QColor& color)
    {
        Q_UNUSED(id);
        Q_UNUSED(color);
        //    if (!m_module_extensions.contains(id))
        //        return;

        //    m_module_extensions.value(id)->color = color;

        // SIGNAL OR
        // MANUAL UPDATE OF SUBSCRIBERS
    }

    void ModuleRelay::setModuleCompressed(const u32 id, const bool mCompressed)
    {
        Q_UNUSED(id);
        Q_UNUSED(mCompressed);
        //    if (!m_module_extensions.contains(id))
        //        return;

        //    m_module_extensions.value(id)->mCompressed = mCompressed;

        // SIGNAL OR
        // MANUAL UPDATE OF SUBSCRIBERS
    }

    void ModuleRelay::setModuleHidden(const u32 id, const bool hidden)
    {
        Q_UNUSED(id);
        Q_UNUSED(hidden);
        //    if (!m_module_extensions.contains(id))
        //        return;

        //    m_module_extensions.value(id)->hidden = hidden;

        // SIGNAL OR
        // MANUAL UPDATE OF SUBSCRIBERS
    }

    void ModuleRelay::handleModuleEvent(ModuleEvent::event ev, Module* object, u32 associated_data)
    {
        Q_UNUSED(associated_data);
        switch (ev)
        {
            case ModuleEvent::event::created: {
                //< no associated_data

                ModuleItem* parent_item = mModuleItems.value(object->get_parent_module()->get_id());

                if (!parent_item)
                    return;    // SHOULD NOT BE POSSIBLE

                break;
            }
            case ModuleEvent::event::removed: {
                //< no associated_data

                break;
            }
            case ModuleEvent::event::name_changed: {
                //< no associated_data

                break;
            }
            case ModuleEvent::event::type_changed: {
                //< no associated_data

                break;
            }
            case ModuleEvent::event::parent_changed: {
                //< no associated_data

                break;
            }
            case ModuleEvent::event::submodule_added: {
                //< associated_data = id of added module

                break;
            }
            case ModuleEvent::event::submodule_removed: {
                //< associated_data = id of removed module

                break;
            }
            case ModuleEvent::event::gate_assigned: {
                //< associated_data = id of inserted gate

                break;
            }
            case ModuleEvent::event::gate_removed: {
                //< associated_data = id of removed gate

                break;
            }
            case ModuleEvent::event::input_port_name_changed: {
                //< associated_data = id of respective net

                break;
            }
            case ModuleEvent::event::output_port_name_changed: {
                //< associated_data = id of respective net

                break;
            }
        }
    }
}
