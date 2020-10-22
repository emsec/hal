#include "hal_core/netlist/event_system/grouping_event_handler.h"

#include "hal_core/netlist/grouping.h"

namespace hal
{
    namespace grouping_event_handler
    {
        namespace
        {
            CallbackHook<void(event, Grouping*, u32)> m_callback;
            bool enabled = true;
        }    // namespace

        void enable(bool flag)
        {
            enabled = flag;
        }

        void notify(event c, Grouping* grouping, u32 associated_data)
        {
            if (enabled)
            {
                m_callback(c, grouping, associated_data);
            }
        }

        void register_callback(const std::string& name, std::function<void(event, Grouping*, u32)> function)
        {
            m_callback.add_callback(name, function);
        }

        void unregister_callback(const std::string& name)
        {
            m_callback.remove_callback(name);
        }
    }    // namespace grouping_event_handler
}    // namespace hal
