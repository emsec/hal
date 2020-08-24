#include "hal_core/netlist/event_system/netlist_event_handler.h"

#include "hal_core/netlist/netlist.h"

namespace hal
{
    namespace netlist_event_handler
    {
        namespace
        {
            CallbackHook<void(event, Netlist*, u32)> m_callback;
            bool enabled = true;
        }    // namespace

        void enable(bool flag)
        {
            enabled = flag;
        }

        void notify(event c, Netlist* netlist, u32 associated_data)
        {
            if (enabled)
            {
                m_callback(c, netlist, associated_data);
            }
        }

        void register_callback(const std::string& name, std::function<void(event, Netlist*, u32)> function)
        {
            m_callback.add_callback(name, function);
        }

        void unregister_callback(const std::string& name)
        {
            m_callback.remove_callback(name);
        }
    }    // namespace netlist_event_handler
}    // namespace hal
