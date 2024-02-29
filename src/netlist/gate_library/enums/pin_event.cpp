#include "hal_core/netlist/gate_library/enums/pin_event.h"
#include "hal_core/netlist/event_system/event_handler.h"
#include "hal_core/netlist/module.h"
#include <algorithm>
#include <assert.h>

namespace hal {

    template<>
    std::map<PinEvent, std::string> EnumStrings<PinEvent>::data = {
        {PinEvent::unknown, "unknown"},
        {PinEvent::GroupCreate, "GroupCreate"},
        {PinEvent::GroupReorder, "GroupReorder"},
        {PinEvent::GroupRename, "GroupRename"},
        {PinEvent::GroupTypeChange, "GroupTypeChange"},
        {PinEvent::GroupDirChange, "GroupDirChange"},
        {PinEvent::GroupDelete, "GroupDelete"},
        {PinEvent::PinCreate, "PinCreate"},
        {PinEvent::PinReorder, "PinReorder"},
        {PinEvent::PinAssignToGroup, "PinAssignToGroup"},
        {PinEvent::PinRename, "PinRename"},
        {PinEvent::PinTypeChange, "PinTypeChange"},
        {PinEvent::PinDirChange, "PinDirChange"},
        {PinEvent::PinDelete, "PinDelete"}
    };

    std::unordered_map<Module*,PinChangedEvent::EventStack*> PinChangedEvent::s_event_stack;

    PinChangedEvent::PinChangedEvent(Module* m, PinEvent pev, u32 id)
        : m_module(m), m_event(pev), m_id(id)
    {;}

    void PinChangedEvent::send()
    {
        auto it = s_event_stack.find(m_module);
        if (it == s_event_stack.end())
        {
            // not stacked, send event immediately
            m_module->get_event_handler()->notify(ModuleEvent::event::pin_changed, m_module, associated_data());
            return;
        }

        // put event on stack to emit it later
        it->second->push_back(*this);
    }

    Module* PinChangedEvent::get_module() const
    {
        return m_module;
    }

    u32 PinChangedEvent::associated_data()
    {
        return (m_id << 4) | (((u32)m_event)&0xF);
    }

    bool pin_event_order(const PinChangedEvent& a, const PinChangedEvent& b)
    {
        if (a.m_event < b.m_event) return true;
        if (a.m_event > b.m_event) return false;
        return a.m_id<b.m_id;
    }

    void PinChangedEvent::EventStack::send_events(Module* m)
    {
        std::sort(begin(),end(),pin_event_order);
        for (auto it=begin(); it != end(); ++it)
            m->get_event_handler()->notify(ModuleEvent::event::pin_changed, m, it->associated_data());
    }

    PinChangedEventScope::PinChangedEventScope(Module* m)
        : m_module(m)
    {
        auto it = PinChangedEvent::s_event_stack.find(m);
        if (it == PinChangedEvent::s_event_stack.end())
            PinChangedEvent::s_event_stack[m] = new PinChangedEvent::EventStack;
        else
            ++it->second->m_count;
    }

    PinChangedEventScope::~PinChangedEventScope()
    {
        auto it = PinChangedEvent::s_event_stack.find(m_module);
        assert(it != PinChangedEvent::s_event_stack.end());
        if (it->second->m_count > 0)
            --it->second->m_count;
        else
        {
            delete it->second;
            PinChangedEvent::s_event_stack.erase(it);
        }
    }

    void PinChangedEventScope::send_events()
    {
        auto it = PinChangedEvent::s_event_stack.find(m_module);
        assert(it != PinChangedEvent::s_event_stack.end());
        if (it->second->m_count > 0)  // do not send yet
            return;
        it->second->send_events(m_module);
    }
}
