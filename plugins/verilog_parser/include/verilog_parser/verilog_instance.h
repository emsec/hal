#pragma once

#include "hal_core/defines.h"
#include "verilog_parser/verilog_signal.h"

#include <map>

namespace hal
{
    class VerilogInstance
    {
    public:
        /**
         * Constructs an instance object.
         *
         * @param[in] line number - The line number of the instance definition within the HDL file.
         * @param[in] type - The type of the instance.
         * @param[in] name - The name of the instance.
         */
        VerilogInstance(u32 line_number, const std::string& type, const std::string& name = "") : m_line_number(line_number), m_type(type), m_name(name)
        {
        }

        /**
         * Gets the line number of the instance definition.
         *
         * @returns The line number.
         */
        u32 get_line_number() const
        {
            return m_line_number;
        }

        /**
         * Gets the type of the instance
         *
         * @returns The instance's type.
         */
        const std::string& get_type() const
        {
            return m_type;
        }

        /**
         * Sets the name of the instance
         *
         * @param[in] name - The instance's name.
         */
        void set_name(const std::string& name)
        {
            m_name = name;
        }

        /**
         * Gets the name of the instance
         *
         * @returns The instance's name.
         */
        const std::string& get_name() const
        {
            return m_name;
        }

        /**
         * Adds a port assignment to a port of the instance.
         *
         * @param[in] port - The port signal.
         * @param[in] assignment - The signals that are assigned to the port.
         */
        void add_port_assignment(const VerilogSignal& port, const std::vector<VerilogSignal>& assignment)
        {
            m_port_assignments.push_back(std::make_pair(port, assignment));
        }

        /**
         * Gets the port assignments of the instance.
         *
         * @returns A vector of port assignments in the form '(port, assignment)'.
         */
        std::vector<std::pair<VerilogSignal, std::vector<VerilogSignal>>>& get_port_assignments()
        {
            return m_port_assignments;
        }

        /**
         * @copydoc get_port_assignments()
         */
        const std::vector<std::pair<VerilogSignal, std::vector<VerilogSignal>>>& get_port_assignments() const
        {
            return m_port_assignments;
        }

        /**
         * Adds a generic assignment to the instance.
         *
         * @param[in] generic - The name of the generic parameter.
         * @param[in] data_type - The data type of the generic parameter.
         * @param[in] assignment - The value assigned to the generic parameter.
         */
        void add_generic_assignment(const std::string& generic, const std::string& data_type, const std::string& assignment)
        {
            m_generic_assignments.emplace(generic, std::make_pair(data_type, assignment));
        }

        /**
         * Gets the generic assignments of the instance.
         *
         * @returns A vector of generic assignments in the form '(generic, data_type, assignment)'.
         */
        const std::map<std::string, std::pair<std::string, std::string>>& get_generic_assignments() const
        {
            return m_generic_assignments;
        }

        /**
         * Adds an attribute to the instance.
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
         * Gets the instance's attributes.
         *
         * @returns A vector of attributes in the form '(name, type, value)'.
         */
        const std::vector<std::tuple<std::string, std::string, std::string>>& get_attributes() const
        {
            return m_attributes;
        }

        /**
         * Tests whether an instance is considered smaller than another instance.
         *
         * @param[in] other - The instance to compare with.
         * @returns True if the instance is smaller than 'other', false otherwise.
         */
        bool operator<(const VerilogInstance& other) const
        {
            return m_name < other.get_name();
        }

    private:
        u32 m_line_number;

        // type
        std::string m_type;

        // name
        std::string m_name;

        // port assignments: port_name -> (port_signal, assignment_signals)
        std::vector<std::pair<VerilogSignal, std::vector<VerilogSignal>>> m_port_assignments;

        // generic assignments: generic_name -> (data_type, data_value)
        std::map<std::string, std::pair<std::string, std::string>> m_generic_assignments;

        // attributes: set(attribute_name, attribute_type, attribute_value)
        std::vector<std::tuple<std::string, std::string, std::string>> m_attributes;
    };
}