#include "netlist/event_system/net_event_handler.h"

#include "netlist/net.h"

namespace hal
{
    namespace net_event_handler
    {
        namespace
        {
            CallbackHook<void(event, std::shared_ptr<Net>, u32)> m_callback;
            bool enabled = true;
        }    // namespace

        void enable(bool flag)
        {
            enabled = flag;
        }

        void notify(event c, std::shared_ptr<Net> net, u32 associated_data)
        {
            if (enabled)
            {
                m_callback(c, net, associated_data);
            }
        }

        void register_callback(const std::string& name, std::function<void(event, std::shared_ptr<Net>, u32)> function)
        {
            m_callback.add_callback(name, function);
        }

        void unregister_callback(const std::string& name)
        {
            m_callback.remove_callback(name);
        }
    }    // namespace net_event_handler
}    // namespace hal
