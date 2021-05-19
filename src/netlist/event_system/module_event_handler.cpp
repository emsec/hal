#include "hal_core/netlist/event_system/module_event_handler.h"

#include "hal_core/netlist/module.h"

namespace hal
{
    namespace module_event_handler
    {
        namespace
        {
            CallbackHook<void(event, Module*, u32)> m_callback;
            bool enabled = true;
        }    // namespace

        void enable(bool flag)
        {
            enabled = flag;
        }

        void notify(event c, Module* module, u32 associated_data)
        {
//            ModuleEvent::dump((ModuleEvent::event)((int)c), false);
            if (enabled)
            {
                m_callback(c, module, associated_data);
            }
        }

        void register_callback(const std::string& name, std::function<void(event, Module*, u32)> function)
        {
            m_callback.add_callback(name, function);
        }

        void unregister_callback(const std::string& name)
        {
            m_callback.remove_callback(name);
        }
    }    // namespace module_event_handler
}    // namespace hal
