#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "verilog_parser/verilog_instance.h"
#include "verilog_parser/verilog_signal.h"
namespace hal
{
    class VerilogEntity
    {
    public:
        /**
         * Constructs a NetlistEntity object.
         *
         * @param[in] line_number - The line number of the entity definition within the HDL file.
         * @param[in] name - The name of the entity.
         */
        VerilogEntity(u32 line_number, const std::string& name) : m_line_number(line_number), m_name(name)
        {
        }

        /**
         * Constructs a NetlistEntity object.
         */
        VerilogEntity() = default;

        /**
         * Gets the line number of the entity definition.
         *
         * @returns The line number.
         */
        u32 get_line_number() const
        {
            return m_line_number;
        }

        /**
         * Gets the name of the entity
         *
         * @returns The entity's name.
         */
        const std::string& get_name() const
        {
            return m_name;
        }

        /**
         * Adds a port to the entity.
         *
         * @param[in] direction - The direction of the port.
         * @param[in] port - The port signal.
         */
        void add_port(PinDirection direction, const VerilogSignal& port)
        {
            m_ports.emplace(port.get_name(), std::make_pair(direction, port));
        }

        /**
         * Gets the ports of the entity.
         *
         * @returns A map from port name to the respective port and its direction.
         */
        std::map<std::string, std::pair<PinDirection, VerilogSignal>>& get_ports()
        {
            return m_ports;
        }

        /**
         * @copydoc get_ports()
         */
        const std::map<std::string, std::pair<PinDirection, VerilogSignal>>& get_ports() const
        {
            return m_ports;
        }

        /**
         * Adds a signal to the entity.
         *
         * @param[in] s - The signal.
         */
        void add_signal(const VerilogSignal& s)
        {
            m_signals.emplace(s.get_name(), s);
        }

        /**
         * Adds several signals to the entity at once.
         *
         * @param[in] signals - A map from signal name to the respective signal.
         */
        void add_signals(const std::map<std::string, VerilogSignal>& signals)
        {
            m_signals.insert(signals.begin(), signals.end());
        }

        /**
         * Gets the signals of the entity.
         *
         * @returns A map from signal name to the respective signal.
         */
        std::map<std::string, VerilogSignal>& get_signals()
        {
            return m_signals;
        }

        /**
         * @copydoc get_signals()
         */
        const std::map<std::string, VerilogSignal>& get_signals() const
        {
            return m_signals;
        }

        /**
         * Adds a direct assignment to the entity.
         *
         * @param[in] s - The signal that is assigned to.
         * @param[in] assignment - The assignment that is assigned to the signal.
         */
        void add_assignment(const std::vector<VerilogSignal>& s, const std::vector<VerilogSignal>& assignment)
        {
            m_assignments.push_back(std::make_pair(s, assignment));
        }

        /**
         * Gets the direct assignments of the entity.
         *
         * @returns A vector of direct assignments.
         */
        const std::vector<std::pair<std::vector<VerilogSignal>, std::vector<VerilogSignal>>>& get_assignments() const
        {
            return m_assignments;
        }

        /**
         * Adds an instance to the entity.
         *
         * @param[in] inst - The instance.
         */
        void add_instance(const VerilogInstance& inst)
        {
            m_instances.emplace(inst.get_name(), inst);
        }

        /**
         * Gets the instances of the entity.
         *
         * @returns A map from the instance name to the respective instance.
         */
        std::map<std::string, VerilogInstance>& get_instances()
        {
            return m_instances;
        }

        /**
         * @copydoc get_instances()
         */
        const std::map<std::string, VerilogInstance>& get_instances() const
        {
            return m_instances;
        }

        /**
         * Adds an attribute to the entity.
         *
         * @param[in] name - The name of the attribute.
         * @param[in] type - The type of the attribute.
         * @param[in] value - The value of the attribute.
         */
        void add_attribute(const std::string& name, const std::string& type, const std::string& value)
        {
            m_attributes.push_back(std::make_tuple(name, type, value));
        }

        /**
         * Gets the entity's attributes.
         *
         * @returns A vector of attributes in the form '(name, type, value)'.
         */
        const std::vector<std::tuple<std::string, std::string, std::string>>& get_attributes() const
        {
            return m_attributes;
        }

        /**
         * Checks whether the entity has been initialized.
         *
         * @returns True if initialized, false otherwise.
         */
        bool is_initialized() const
        {
            return m_initialized;
        }

        /**
         * Gets the expanded ports of the entity.
         *
         * @returns The expanded ports.
         */
        const std::map<std::string, std::vector<std::string>>& get_expanded_ports() const
        {
            return m_expanded_ports;
        }

        /**
         * Gets the expanded signals of the entity.
         *
         * @returns The expanded signals.
         */
        const std::map<std::string, std::vector<std::string>>& get_expanded_signals() const
        {
            return m_expanded_signals;
        }

        /**
         * Gets the expanded assignments of the entity.
         *
         * @returns The expanded assignments.
         */
        const std::map<std::string, std::string>& get_expanded_assignments() const
        {
            return m_expanded_assignments;
        }

        /**
         * Tests whether an entity is considered smaller than another entity.
         *
         * @param[in] other - The entity to compare with.
         * @returns True if the entity is smaller than 'other', false otherwise.
         */
        bool operator<(const VerilogEntity& other) const
        {
            return m_name < other.get_name();
        }

    private:
        u32 m_line_number;

        // name
        std::string m_name;

        // ports: port_name -> (direction, signal)
        std::map<std::string, std::pair<PinDirection, VerilogSignal>> m_ports;

        // signals: signal_name -> signal
        std::map<std::string, VerilogSignal> m_signals;

        // assignments: set(lhs, rhs)
        std::vector<std::pair<std::vector<VerilogSignal>, std::vector<VerilogSignal>>> m_assignments;

        // instances: instance_name -> instance
        std::map<std::string, VerilogInstance> m_instances;

        // attributes: set(attribute_name, attribute_type, attribute_value)
        std::vector<std::tuple<std::string, std::string, std::string>> m_attributes;

        // is already initialized?
        bool m_initialized = false;

        // expanded ports: port_name -> expanded_ports
        std::map<std::string, std::vector<std::string>> m_expanded_ports;

        // expanded signals: signal_name -> expanded_signals
        std::map<std::string, std::vector<std::string>> m_expanded_signals;

        // expanded assignments: expanded_signal_name -> expanded_assignment_name
        std::map<std::string, std::string> m_expanded_assignments;
    };
}    // namespace hal