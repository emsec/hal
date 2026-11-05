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
        {PinEvent::PinDelete, "PinDelete"},
        {PinEvent::PinsReload, "PinsReload"}
    };

    std::unordered_map<Module*,PinChangedEvent::EventStack*> PinChangedEvent::s_event_stack;
    u64 PinChangedEvent::s_order = 0;
    int PinChangedEvent::s_bulk_depth = 0;

    PinChangedEvent::PinChangedEvent(Module* m, PinEvent pev, u32 id)
        : m_module(m), m_event(pev), m_id(id), m_order(++s_order)
    {;}

    void PinChangedEvent::send()
    {
        auto it = s_event_stack.find(m_module);
        if (it == s_event_stack.end())
        {
            if (s_bulk_depth <= 0)
            {
                // not stacked, send event immediately
                m_module->get_event_handler()->notify(ModuleEvent::event::pin_changed, m_module, associated_data());
                return;
            }

            // within a bulk scope every module collects its events, the bulk scope owns the stack
            it              = s_event_stack.emplace(m_module, new EventStack).first;
            it->second->m_sticky = true;
        }

        // put event on stack to emit it later
        it->second->push_back(*this);
    }

    void PinChangedEvent::discard(Module* m)
    {
        auto it = s_event_stack.find(m);
        if (it == s_event_stack.end())
            return;
        delete it->second;
        s_event_stack.erase(it);
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
        if (a.m_event == PinEvent::PinAssignToGroup)
            return a.m_order > b.m_order; // revert order for descending pin groups
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
        {
            auto* stack      = new PinChangedEvent::EventStack;
            stack->m_sticky  = (PinChangedEvent::s_bulk_depth > 0);
            PinChangedEvent::s_event_stack[m] = stack;
        }
        else
            ++it->second->m_count;
    }

    PinChangedEventScope::~PinChangedEventScope()
    {
        auto it = PinChangedEvent::s_event_stack.find(m_module);
        if (it == PinChangedEvent::s_event_stack.end())
            return;    // module got destroyed within the scope, stack has been discarded
        if (it->second->m_count > 0)
            --it->second->m_count;
        else if (!it->second->m_sticky)  // sticky stacks are owned by the enclosing bulk scope
        {
            delete it->second;
            PinChangedEvent::s_event_stack.erase(it);
        }
    }

    void PinChangedEventScope::send_events()
    {
        auto it = PinChangedEvent::s_event_stack.find(m_module);
        if (it == PinChangedEvent::s_event_stack.end())
            return;    // module got destroyed within the scope, stack has been discarded
        if (it->second->m_count > 0)  // do not send yet
            return;
        if (it->second->m_sticky)     // coalesced and sent by the enclosing bulk scope
            return;
        it->second->send_events(m_module);
    }

    PinChangedBulkScope::PinChangedBulkScope()
    {
        ++PinChangedEvent::s_bulk_depth;
    }

    PinChangedBulkScope::~PinChangedBulkScope()
    {
        if (--PinChangedEvent::s_bulk_depth > 0)
            return;

        // hand the stacks over before sending, listeners are free to change pins from within their handler
        std::unordered_map<Module*,PinChangedEvent::EventStack*> stacks;
        stacks.swap(PinChangedEvent::s_event_stack);

        for (auto& [module, stack] : stacks)
        {
            if (!stack->empty())
                module->get_event_handler()->notify(ModuleEvent::event::pin_changed, module, PinChangedEvent(module, PinEvent::PinsReload, 0).associated_data());
            delete stack;
        }
    }
}
