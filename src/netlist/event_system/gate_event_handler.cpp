#include "netlist/event_system/gate_event_handler.h"

#include "netlist/gate.h"

namespace hal
{
    namespace gate_event_handler
    {
        namespace
        {
            callback_hook<void(event, std::shared_ptr<gate>, u32)> m_callback;
            bool enabled = true;
        }    // namespace

        void enable(bool flag)
        {
            enabled = flag;
        }

        void notify(event c, std::shared_ptr<gate> gate, u32 associated_data)
        {
            if (enabled)
            {
                m_callback(c, gate, associated_data);
            }
        }

        void register_callback(const std::string& name, std::function<void(event, std::shared_ptr<gate>, u32)> function)
        {
            m_callback.add_callback(name, function);
        }

        void unregister_callback(const std::string& name)
        {
            m_callback.remove_callback(name);
        }
    }    // namespace gate_event_handler
}    // namespace hal
