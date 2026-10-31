#include "hal_core/netlist/gate_library/gate_type_component/state_table_component.h"

#include "hal_core/netlist/pins/gate_pin.h"

namespace hal
{
    StateTableComponent::StateTableComponent(std::unique_ptr<GateTypeComponent> component, std::vector<StateTable> tables)
        : m_component(std::move(component)), m_tables(std::move(tables))
    {
    }

    std::unique_ptr<StateTableComponent> StateTableComponent::create(std::unique_ptr<GateTypeComponent> component, std::vector<StateTable> tables)
    {
        return std::unique_ptr<StateTableComponent>(new StateTableComponent(std::move(component), std::move(tables)));
    }

    StateTableComponent::ComponentType StateTableComponent::get_type() const
    {
        return m_type;
    }

    bool StateTableComponent::is_class_of(const GateTypeComponent* component)
    {
        return component->get_type() == m_type;
    }

    std::vector<GateTypeComponent*> StateTableComponent::get_components(const std::function<bool(const GateTypeComponent*)>& filter) const
    {
        if (m_component != nullptr)
        {
            std::vector<GateTypeComponent*> res = m_component->get_components(filter);
            if (filter)
            {
                if (filter(m_component.get()))
                {
                    res.push_back(m_component.get());
                }
            }
            else
            {
                res.push_back(m_component.get());
            }
            return res;
        }
        return {};
    }

    const std::vector<StateTableComponent::StateTable>& StateTableComponent::get_state_tables() const
    {
        return m_tables;
    }

    Result<const StateTableComponent::StateTable*> StateTableComponent::get_state_table_for_pin(const std::string& pin_name) const
    {
        for (const StateTable& t : m_tables)
        {
            if (t.pin_name == pin_name)
            {
                return OK(&t);
            }
        }
        return ERR("no state table found for pin '" + pin_name + "'");
    }

    Result<const StateTableComponent::StateTable*> StateTableComponent::get_state_table_for_pin(const GatePin* pin) const
    {
        if (pin == nullptr)
        {
            return ERR("pin is nullptr");
        }
        return get_state_table_for_pin(pin->get_name());
    }
}    // namespace hal
