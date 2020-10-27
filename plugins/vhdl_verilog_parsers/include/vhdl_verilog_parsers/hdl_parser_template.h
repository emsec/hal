//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/gate_library/gate_type/gate_type.h"
#include "hal_core/netlist/hdl_parser/hdl_parser.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/special_strings.h"

#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace hal
{
    /* forward declaration*/
    class Netlist;
    class GateLibrary;

    /**
     * @ingroup hdl_parsers
     */
    template<typename T>
    class HDL_PARSER_API HDLParserTemplate : public HDLParser
    {
    public:
        /**
         * Constructs a verilog parser object.
         *
         * @param[in] stream - The string stream filled with the hdl code.
         */
        explicit HDLParserTemplate()
        {
            m_netlist = nullptr;
        }

        virtual ~HDLParserTemplate() = default;

        /**
         * Parses a netlist into an intermediate format.
         *
         * @returns True on success, false otherwise.
         */
        virtual bool parse(std::stringstream& stream) = 0;

        /**
         * Instantiates a previously parsed netlist using the specified gate library.
         *
         * @param[in] gl - The gate library.
         * @returns A pointer to the resulting netlist.
         */
        std::unique_ptr<Netlist> instantiate(const GateLibrary* gl)
        {
            // create empty netlist
            auto result = netlist_factory::create_netlist(gl);
            m_netlist   = result.get();

            if (m_netlist == nullptr)
            {
                // error printed in subfunction
                return nullptr;
            }

            // any entities in netlist?
            if (m_entities.empty())
            {
                log_error("hdl_parser", "file did not contain any entities");
                return nullptr;
            }

            m_signal_name_occurrences.clear();
            m_instance_name_occurrences.clear();
            m_net_by_name.clear();
            m_nets_to_merge.clear();
            m_tmp_gate_types.clear();
            m_module_ports.clear();

            // retrieve available gate types
            if constexpr (std::is_same<T, std::string>::value)
            {
                m_tmp_gate_types = m_netlist->get_gate_library()->get_gate_types();
            }
            else
            {
                for (const auto& gt : m_netlist->get_gate_library()->get_gate_types())
                {
                    m_tmp_gate_types.emplace(core_strings::convert_string<std::string, T>(gt.first), gt.second);
                }
            }

            // match each entity's instances to entities and gate types
            for (auto& e : m_entities)
            {
                for (auto& [inst_name, inst] : e.second.get_instances())
                {
                    auto& port_assignments = inst.get_port_assignments();

                    // instance of entity
                    if (const auto entity_it = m_entities.find(inst.get_type()); entity_it != m_entities.end())
                    {
                        // fill in port width
                        const auto& entity_ports = entity_it->second.get_ports();

                        for (auto& [port, assignments] : port_assignments)
                        {
                            if (!port.is_ranges_known())
                            {
                                if (const auto port_it = entity_ports.find(port.get_name()); port_it != entity_ports.end())
                                {
                                    port.set_ranges(port_it->second.second.get_ranges());

                                    const i32 left_size = port.get_size();
                                    i32 right_size      = 0;
                                    for (const auto& s : assignments)
                                    {
                                        right_size += s.get_size();
                                    }

                                    if constexpr (!std::is_same<T, std::string>::value)
                                    {
                                        // verify matching port assignment width for VHDL parser
                                        if (left_size != right_size)
                                        {
                                            log_error("hdl_parser",
                                                      "port assignment width mismatch: left side has size {} and right side has size {} in line {}",
                                                      left_size,
                                                      right_size,
                                                      port.get_line_number());
                                            return nullptr;
                                        }
                                    }
                                }
                                else
                                {
                                    log_error(
                                        "hdl_parser", "port '{}' is no valid port for instance '{}' of entity '{}' in line {}", port.get_name(), inst_name, entity_it->first, port.get_line_number());
                                    return nullptr;
                                }
                            }
                        }
                    }
                    // instance of gate type
                    else if (const auto gate_it = m_tmp_gate_types.find(inst.get_type()); gate_it != m_tmp_gate_types.end())
                    {
                        // cache pin types
                        std::vector<T> pins;
                        std::unordered_map<T, std::map<u32, std::string>> pin_groups;

                        if constexpr (std::is_same<T, std::string>::value)
                        {
                            pins                       = gate_it->second->get_input_pins();
                            std::vector<T> output_pins = gate_it->second->get_output_pins();
                            pins.insert(pins.end(), output_pins.begin(), output_pins.end());

                            pin_groups                                                          = gate_it->second->get_input_pin_groups();
                            std::unordered_map<T, std::map<u32, std::string>> output_pin_groups = gate_it->second->get_output_pin_groups();
                            pin_groups.insert(output_pin_groups.begin(), output_pin_groups.end());
                        }
                        else
                        {
                            for (const auto& pin : gate_it->second->get_input_pins())
                            {
                                pins.push_back(core_strings::convert_string<std::string, T>(pin));
                            }
                            for (const auto& pin : gate_it->second->get_output_pins())
                            {
                                pins.push_back(core_strings::convert_string<std::string, T>(pin));
                            }

                            for (const auto& pin_group : gate_it->second->get_input_pin_groups())
                            {
                                pin_groups.emplace(core_strings::convert_string<std::string, T>(pin_group.first), pin_group.second);
                            }
                            for (const auto& pin_group : gate_it->second->get_output_pin_groups())
                            {
                                pin_groups.emplace(core_strings::convert_string<std::string, T>(pin_group.first), pin_group.second);
                            }
                        }

                        for (auto& [port, assignments] : port_assignments)
                        {
                            if (!port.is_ranges_known())
                            {
                                if (const auto pin_it = std::find(pins.begin(), pins.end(), port.get_name()); pin_it != pins.end())
                                {
                                    port.set_ranges({});
                                }
                                else if (const auto pin_group_it = pin_groups.find(port.get_name()); pin_group_it != pin_groups.end())
                                {
                                    std::vector<u32> range;
                                    for (const auto& pin : pin_group_it->second)
                                    {
                                        range.push_back(pin.first);
                                    }
                                    port.set_ranges({range});
                                }
                                else
                                {
                                    log_error("hdl_parser", "pin '{}' is no valid pin for gate '{}' of type '{}' in line {}", port.get_name(), inst_name, gate_it->first, port.get_line_number());
                                    return nullptr;
                                }

                                const i32 left_size = port.get_size();
                                i32 right_size      = 0;
                                for (const auto& s : assignments)
                                {
                                    right_size += s.get_size();
                                }

                                if constexpr (!std::is_same<T, std::string>::value)
                                {
                                    if (left_size != right_size)
                                    {
                                        log_error("hdl_parser",
                                                  "port assignment width mismatch: port has width {} and assigned signal has width {} in line {}",
                                                  left_size,
                                                  right_size,
                                                  port.get_line_number());
                                        return nullptr;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        log_error("hdl_parser", "type '{}' of instance '{}' is neither an entity, nor a gate type in line {}", inst.get_type(), inst_name, inst.get_line_number());
                        return nullptr;
                    }
                }
            }

            // create const 0 and const 1 net, will be removed if unused
            m_zero_net = m_netlist->create_net("'0'");
            if (m_zero_net == nullptr)
            {
                return nullptr;
            }
            m_net_by_name[core_strings::convert_string<std::string, T>(m_zero_net->get_name())] = m_zero_net;

            m_one_net = m_netlist->create_net("'1'");
            if (m_one_net == nullptr)
            {
                return nullptr;
            }
            m_net_by_name[core_strings::convert_string<std::string, T>(m_one_net->get_name())] = m_one_net;

            // build the netlist from the intermediate format
            // the last entity in the file is considered the top module
            if (!build_netlist(m_last_entity))
            {
                return nullptr;
            }

            // add global GND gate if required by any instance
            if (!m_zero_net->get_destinations().empty())
            {
                const auto gnd_type   = m_netlist->get_gate_library()->get_gnd_gate_types().begin()->second;
                const auto output_pin = gnd_type->get_output_pins().at(0);
                const auto gnd        = m_netlist->create_gate(m_netlist->get_unique_gate_id(), gnd_type, "global_gnd");

                if (!m_netlist->mark_gnd_gate(gnd))
                {
                    return nullptr;
                }

                if (auto gnd_net = m_net_by_name.find("'0'")->second; !gnd_net->add_source(gnd, output_pin))
                {
                    return nullptr;
                }
            }
            else
            {
                m_netlist->delete_net(m_zero_net);
            }

            // add global VCC gate if required by any instance
            if (!m_one_net->get_destinations().empty())
            {
                const auto vcc_type   = m_netlist->get_gate_library()->get_vcc_gate_types().begin()->second;
                const auto output_pin = vcc_type->get_output_pins().at(0);
                const auto vcc        = m_netlist->create_gate(m_netlist->get_unique_gate_id(), vcc_type, "global_vcc");

                if (!m_netlist->mark_vcc_gate(vcc))
                {
                    return nullptr;
                }

                if (auto vcc_net = m_net_by_name.find("'1'")->second; !vcc_net->add_source(vcc, output_pin))
                {
                    return nullptr;
                }
            }
            else
            {
                m_netlist->delete_net(m_one_net);
            }

            for (auto net : m_netlist->get_nets())
            {
                const bool no_source      = net->get_num_of_sources() == 0 && !net->is_global_input_net();
                const bool no_destination = net->get_num_of_destinations() == 0 && !net->is_global_output_net();
                if (no_source && no_destination)
                {
                    m_netlist->delete_net(net);
                }
            }

            return result;
        }

    protected:
        enum class port_direction
        {
            IN,
            OUT,
            INOUT
        };

        class signal
        {
        public:
            /**
             * Constructs a signal object
             *
             * @param[in] line_number - The line number of the signal definition within the HDL file.
             * @param[in] name - The name of the signal.
             * @param[in] ranges - The signal range for multi-bit signals, e.g., '{0, 1, 2, 3}' for a four bit signal.
             * @param[in] binary - Binary signal ('true') or not ('false'). If true, the signal name holds the signals values as bits.
             * @param[in] ranges_known - Ranges are known on construction ('true') or not ('false').
             */
            signal(u32 line_number, const T& name, const std::vector<std::vector<u32>>& ranges = {}, bool binary = false, bool ranges_known = true)
                : m_line_number(line_number), m_name(name), m_ranges(ranges), m_binary(binary), m_ranges_known(ranges_known)
            {
                compute_size();
            }

            /**
             * Gets the line number of the signal definition.
             *
             * @returns The line number.
             */
            u32 get_line_number() const
            {
                return m_line_number;
            }

            /**
             * Gets the name of the signal.
             *
             * @returns The signal's name.
             */
            const T& get_name() const
            {
                return m_name;
            }

            /**
             * Gets the size of the signal in bits.
             *
             * @returns The signal's size.
             */
            i32 get_size() const
            {
                return m_size;
            }

            /**
             * Sets the range of the signal, e.g., '{0, 1, 2, 3}' for a four bit signal.
             *
             * @param[in] ranges - The signal's range.
             */
            void set_ranges(const std::vector<std::vector<u32>>& ranges)
            {
                m_ranges       = ranges;
                m_ranges_known = true;
                compute_size();
            }

            /**
             * Gets the range of the signal, e.g., '{0, 1, 2, 3}' for a four bit signal.
             *
             * @returns The signal's range.
             */
            const std::vector<std::vector<u32>>& get_ranges() const
            {
                return m_ranges;
            }

            /**
             * Adds an attribute to the signal.
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
             * Gets the signal's attributes.
             *
             * @returns A vector of attributes in the form '(name, type, value)'.
             */
            const std::vector<std::tuple<std::string, std::string, std::string>>& get_attributes() const
            {
                return m_attributes;
            }

            /**
             * Checks whether the signal is a binary signal.
             *
             * @returns True if binary, false otherwise.
             */
            bool is_binary() const
            {
                return m_binary;
            }

            /**
             * Checks whether the range of the signal is known.
             *
             * @returns True if range is known, false otherwise.
             */
            bool is_ranges_known() const
            {
                return m_ranges_known;
            }

            /**
             * Tests whether a signal is considered smaller than another signal.
             *
             * @param[in] other - The signal to compare with.
             * @returns True if the signal is smaller than 'other', false otherwise.
             */
            bool operator<(const signal& other) const
            {
                return (m_name < other._name) && (m_ranges < other._ranges);
            }

        private:
            u32 m_line_number;

            // name (may either be the identifier of the signal or a binary string in case of direct assignments)
            T m_name;

            // number of bits belonging to the signal (-1 implies "unknown")
            i32 m_size;

            // ranges
            std::vector<std::vector<u32>> m_ranges;

            // attributes: set(attribute_name, attribute_type, attribute_value)
            std::vector<std::tuple<std::string, std::string, std::string>> m_attributes;

            // is binary string?
            bool m_binary = false;

            // are bounds already known? (should only be unknown for left side of port assignments)
            bool m_ranges_known = true;

            /**
             * Computes the size of the signal.
             */
            void compute_size()
            {
                if (m_ranges_known)
                {
                    if (m_binary)
                    {
                        m_size = m_name.size();
                    }
                    else if (m_ranges.empty())
                    {
                        m_size = 1;
                    }
                    else
                    {
                        u32 dimension = m_ranges.size();
                        m_size        = m_ranges.at(0).size();

                        for (u32 i = 1; i < dimension; i++)
                        {
                            m_size *= m_ranges.at(i).size();
                        }
                    }
                }
                else
                {
                    m_size = -1;
                }
            }
        };

        class instance
        {
        public:
            /**
             * Constructs an instance object.
             *
             * @param[in] line number - The line number of the instance definition within the HDL file.
             * @param[in] type - The type of the instance.
             * @param[in] name - The name of the instance.
             */
            instance(u32 line_number, const T& type, const T& name = "") : m_line_number(line_number), m_type(type), m_name(name)
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
            const T& get_type() const
            {
                return m_type;
            }

            /**
             * Sets the name of the instance
             *
             * @param[in] name - The instance's name.
             */
            void set_name(const T& name)
            {
                m_name = name;
            }

            /**
             * Gets the name of the instance
             *
             * @returns The instance's name.
             */
            const T& get_name() const
            {
                return m_name;
            }

            /**
             * Adds a port assignment to a port of the instance.
             *
             * @param[in] port - The port signal.
             * @param[in] assignment - The signals that are assigned to the port.
             */
            void add_port_assignment(const signal& port, const std::vector<signal>& assignment)
            {
                m_port_assignments.push_back(std::make_pair(port, assignment));
            }

            /**
             * Gets the port assignments of the instance.
             *
             * @returns A vector of port assignments in the form '(port, assignment)'.
             */
            std::vector<std::pair<signal, std::vector<signal>>>& get_port_assignments()
            {
                return m_port_assignments;
            }

            /**
             * @copydoc get_port_assignments()
             */
            const std::vector<std::pair<signal, std::vector<signal>>>& get_port_assignments() const
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
            bool operator<(const instance& other) const
            {
                return m_name < other._name;
            }

        private:
            u32 m_line_number;

            // type
            T m_type;

            // name
            T m_name;

            // port assignments: port_name -> (port_signal, assignment_signals)
            std::vector<std::pair<signal, std::vector<signal>>> m_port_assignments;

            // generic assignments: generic_name -> (data_type, data_value)
            std::map<std::string, std::pair<std::string, std::string>> m_generic_assignments;

            // attributes: set(attribute_name, attribute_type, attribute_value)
            std::vector<std::tuple<std::string, std::string, std::string>> m_attributes;
        };

        class entity
        {
        public:
            /**
             * Constructs an entity object.
             *
             * @param[in] line number - The line number of the entity definition within the HDL file.
             * @param[in] name - The name of the entity.
             */
            entity(u32 line_number, const T& name) : m_line_number(line_number), m_name(name)
            {
            }

            entity() = default;

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
            const T& get_name() const
            {
                return m_name;
            }

            /**
             * Adds a port to the entity.
             *
             * @param[in] direction - The direction of the port.
             * @param[in] port - The port signal.
             */
            void add_port(port_direction direction, const signal& port)
            {
                m_ports.emplace(port.get_name(), std::make_pair(direction, port));
            }

            /**
             * Gets the ports of the entity.
             *
             * @returns A map from port name to the respective port and its direction.
             */
            std::map<T, std::pair<port_direction, signal>>& get_ports()
            {
                return m_ports;
            }

            /**
             * @copydoc get_ports()
             */
            const std::map<T, std::pair<port_direction, signal>>& get_ports() const
            {
                return m_ports;
            }

            /**
             * Adds a signal to the entity.
             *
             * @param[in] s - The signal.
             */
            void add_signal(const signal& s)
            {
                m_signals.emplace(s.get_name(), s);
            }

            /**
             * Adds several signals to the entity at once.
             *
             * @param[in] signals - A map from signal name to the respective signal.
             */
            void add_signals(const std::map<T, signal>& signals)
            {
                m_signals.insert(signals.begin(), signals.end());
            }

            /**
             * Gets the signals of the entity.
             *
             * @returns A map from signal name to the respective signal.
             */
            std::map<T, signal>& get_signals()
            {
                return m_signals;
            }

            /**
             * @copydoc get_signals()
             */
            const std::map<T, signal>& get_signals() const
            {
                return m_signals;
            }

            /**
             * Adds a direct assignment to the entity.
             *
             * @param[in] s - The signal that is assigned to.
             * @param[in] assignment - The assignment that is assigned to the signal.
             */
            void add_assignment(const std::vector<signal>& s, const std::vector<signal>& assignment)
            {
                m_assignments.push_back(std::make_pair(s, assignment));
            }

            /**
             * Gets the direct assignments of the entity.
             *
             * @returns A vector of direct assignments.
             */
            const std::vector<std::pair<std::vector<signal>, std::vector<signal>>>& get_assignments() const
            {
                return m_assignments;
            }

            /**
             * Adds an instance to the entity.
             *
             * @param[in] inst - The instance.
             */
            void add_instance(const instance& inst)
            {
                m_instances.emplace(inst.get_name(), inst);
            }

            /**
             * Gets the instances of the entity.
             *
             * @returns A map from the instance name to the respective instance.
             */
            std::map<T, instance>& get_instances()
            {
                return m_instances;
            }

            /**
             * @copydoc get_instances()
             */
            const std::map<T, instance>& get_instances() const
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
             * Expands the ports, signals and assignments of the entity.
             *
             * @param[in] parser - The parser used for parsering of the netlist.
             */
            void initialize(HDLParserTemplate<T>* parser)
            {
                if (m_initialized)
                {
                    m_expanded_ports.clear();
                    m_expanded_signals.clear();
                    m_expanded_assignments.clear();
                }

                for (const auto& [port_name, p] : m_ports)
                {
                    m_expanded_ports.emplace(port_name, parser->expand_signal(p.second));
                }

                for (const auto& [signal_name, s] : m_signals)
                {
                    m_expanded_signals.emplace(signal_name, parser->expand_signal(s));
                }

                std::vector<T> expanded_signals;
                std::vector<T> expanded_assignments;

                for (const auto& [signals, assignments] : m_assignments)
                {
                    for (const auto& s : signals)
                    {
                        std::vector<T> expanded;
                        expanded = parser->expand_signal(s);
                        std::move(expanded.begin(), expanded.end(), std::back_inserter(expanded_signals));
                    }

                    for (const auto& s : assignments)
                    {
                        std::vector<T> expanded;
                        if (s.is_binary())
                        {
                            expanded = parser->expand_binary_signal(s);
                        }
                        else
                        {
                            expanded = parser->expand_signal(s);
                        }
                        std::move(expanded.begin(), expanded.end(), std::back_inserter(expanded_assignments));
                    }
                }

                std::transform(
                    expanded_signals.begin(), expanded_signals.end(), expanded_assignments.begin(), std::inserter(m_expanded_assignments, m_expanded_assignments.end()), [](const T& s, const T& a) {
                        return std::make_pair(s, a);
                    });

                m_initialized = true;
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
            const std::map<T, std::vector<T>>& get_expanded_ports() const
            {
                return m_expanded_ports;
            }

            /**
             * Gets the expanded signals of the entity.
             *
             * @returns The expanded signals.
             */
            const std::map<T, std::vector<T>>& get_expanded_signals() const
            {
                return m_expanded_signals;
            }

            /**
             * Gets the expanded assignments of the entity.
             *
             * @returns The expanded assignments.
             */
            const std::map<T, T>& get_expanded_assignments() const
            {
                return m_expanded_assignments;
            }

            /**
             * Tests whether an entity is considered smaller than another entity.
             *
             * @param[in] other - The entity to compare with.
             * @returns True if the entity is smaller than 'other', false otherwise.
             */
            bool operator<(const entity& other) const
            {
                return m_name < other.get_name();
            }

        private:
            u32 m_line_number;

            // name
            T m_name;

            // ports: port_name -> (direction, signal)
            std::map<T, std::pair<port_direction, signal>> m_ports;

            // signals: signal_name -> signal
            std::map<T, signal> m_signals;

            // assignments: set(lhs, rhs)
            std::vector<std::pair<std::vector<signal>, std::vector<signal>>> m_assignments;

            // instances: instance_name -> instance
            std::map<T, instance> m_instances;

            // attributes: set(attribute_name, attribute_type, attribute_value)
            std::vector<std::tuple<std::string, std::string, std::string>> m_attributes;

            // is already initialized?
            bool m_initialized = false;

            // expanded ports: port_name -> expanded_ports
            std::map<T, std::vector<T>> m_expanded_ports;

            // expanded signals: signal_name -> expanded_signals
            std::map<T, std::vector<T>> m_expanded_signals;

            // expanded assignments: expanded_signal_name -> expanded_assignment_name
            std::map<T, T> m_expanded_assignments;
        };

        // stores the input stream to the file
        std::stringstream* m_fs;

        // map of all entities
        std::unordered_map<T, entity> m_entities;
        T m_last_entity;

    private:
        // temporarily stores the netlist during serialization
        Netlist* m_netlist;

        // unique alias generation
        std::unordered_map<T, u32> m_signal_name_occurrences;
        std::unordered_map<T, u32> m_instance_name_occurrences;

        // net generation
        Net* m_zero_net;
        Net* m_one_net;
        std::unordered_map<T, Net*> m_net_by_name;
        std::unordered_map<T, std::vector<T>> m_nets_to_merge;

        // buffer gate types
        std::unordered_map<T, const GateType*> m_tmp_gate_types;
        std::unordered_map<Net*, std::tuple<port_direction, std::string, Module*>> m_module_ports;

        bool build_netlist(const T& top_module)
        {
            m_netlist->set_design_name(core_strings::convert_string<T, std::string>(top_module));

            auto& top_entity = m_entities[top_module];

            std::unordered_map<T, u32> instantiation_count;

            // preparations for alias: count the occurences of all names
            std::queue<entity*> q;
            q.push(&top_entity);

            // top entity instance will be named after its entity, so take into account for aliases
            m_instance_name_occurrences["top_module"]++;

            // signals will be named after ports, so take into account for aliases
            for (const auto& port : top_entity.get_ports())
            {
                m_signal_name_occurrences[port.first]++;
            }

            while (!q.empty())
            {
                auto e = q.front();
                q.pop();

                instantiation_count[e->get_name()]++;

                for (const auto& s : e->get_signals())
                {
                    m_signal_name_occurrences[s.first]++;
                }

                for (const auto& inst : e->get_instances())
                {
                    m_instance_name_occurrences[inst.first]++;

                    if (const auto it = m_entities.find(inst.second.get_type()); it != m_entities.end())
                    {
                        q.push(&(it->second));
                    }
                }
            }

            // detect unused entities
            for (auto& e : m_entities)
            {
                if (!e.second.is_initialized())
                {
                    log_warning("hdl_parser", "entity '{}' has not been initialized during parsing, this may affect performance", e.first);
                    e.second.initialize(this);
                }

                if (instantiation_count[e.first] == 0)
                {
                    log_warning("hdl_parser", "entity '{}' defined but not used", e.first);
                }
            }

            // for the top module, generate global i/o signals for all ports
            std::map<T, T> top_assignments;
            auto& expanded_ports = top_entity.get_expanded_ports();

            for (const auto& [port_name, port] : top_entity.get_ports())
            {
                const auto direction = port.first;

                for (const auto& expanded_name : expanded_ports.at(port_name))
                {
                    Net* new_net = m_netlist->create_net(core_strings::convert_string<T, std::string>(expanded_name));
                    if (new_net == nullptr)
                    {
                        log_error("hdl_parser", "could not create new net '{}'", expanded_name);
                        return false;
                    }

                    m_net_by_name[expanded_name] = new_net;

                    // for instances, point the ports to the newly generated signals
                    top_assignments[expanded_name] = core_strings::convert_string<std::string, T>(new_net->get_name());

                    if (direction == port_direction::IN || direction == port_direction::INOUT)
                    {
                        if (!new_net->mark_global_input_net())
                        {
                            log_error("hdl_parser", "could not mark net '{}' as global input", expanded_name);
                            return false;
                        }
                    }

                    if (direction == port_direction::OUT || direction == port_direction::INOUT)
                    {
                        if (!new_net->mark_global_output_net())
                        {
                            log_error("hdl_parser", "could not mark net '{}' as global output", expanded_name);
                            return false;
                        }
                    }
                }
            }

            instance top_instance(top_entity.get_line_number(), top_entity.get_name(), "top_module");
            if (instantiate(top_instance, nullptr, top_assignments) == nullptr)
            {
                // error printed in subfunction
                return false;
            }

            // merge nets without gates in between them
            while (!m_nets_to_merge.empty())
            {
                // master = net that other nets are merged into
                // slave = net to merge into master and then delete

                bool progress_made = false;

                for (const auto& [master, merge_set] : m_nets_to_merge)
                {
                    // check if none of the slaves is itself a master
                    bool is_master = false;
                    for (const auto& slave : merge_set)
                    {
                        if (m_nets_to_merge.find(slave) != m_nets_to_merge.end())
                        {
                            is_master = true;
                            break;
                        }
                    }

                    if (is_master)
                    {
                        continue;
                    }

                    auto master_net = m_net_by_name.at(master);
                    for (const auto& slave : merge_set)
                    {
                        auto slave_net = m_net_by_name.at(slave);

                        // merge sources
                        if (slave_net->is_global_input_net())
                        {
                            master_net->mark_global_input_net();
                        }

                        for (auto src : slave_net->get_sources())
                        {
                            Gate* src_gate      = src->get_gate();
                            std::string src_pin = src->get_pin();

                            slave_net->remove_source(src);

                            if (!master_net->is_a_source(src_gate, src_pin))
                            {
                                master_net->add_source(src_gate, src_pin);
                            }
                        }

                        // merge destinations
                        if (slave_net->is_global_output_net())
                        {
                            master_net->mark_global_output_net();
                        }

                        for (auto dst : slave_net->get_destinations())
                        {
                            Gate* dst_gate      = dst->get_gate();
                            std::string dst_pin = dst->get_pin();

                            slave_net->remove_destination(dst);

                            if (!master_net->is_a_destination(dst_gate, dst_pin))
                            {
                                master_net->add_destination(dst_gate, dst_pin);
                            }
                        }

                        // merge generics and attributes
                        for (const auto it : slave_net->get_data())
                        {
                            if (!master_net->set_data(std::get<0>(it.first), std::get<1>(it.first), std::get<0>(it.second), std::get<1>(it.second)))
                            {
                                log_error("hdl_parser", "couldn't set data");
                            }
                        }

                        // update module ports
                        if (const auto it = m_module_ports.find(slave_net); it != m_module_ports.end())
                        {
                            m_module_ports[master_net] = it->second;
                            m_module_ports.erase(it);
                        }

                        // make sure to keep module ports up to date
                        m_netlist->delete_net(slave_net);
                        m_net_by_name.erase(slave);
                    }

                    m_nets_to_merge.erase(master);
                    progress_made = true;
                    break;
                }

                if (!progress_made)
                {
                    log_error("hdl_parser", "cyclic dependency between signals found, cannot parse netlist");
                    return false;
                }
            }

            // assign module ports
            for (const auto& [net, port_info] : m_module_ports)
            {
                const auto direction = std::get<0>(port_info);
                const auto port_name = std::get<1>(port_info);
                auto module          = std::get<2>(port_info);

                if (direction == port_direction::IN || direction == port_direction::INOUT)
                {
                    module->set_input_port_name(net, port_name);
                }

                if (direction == port_direction::OUT || direction == port_direction::INOUT)
                {
                    module->set_output_port_name(net, port_name);
                }
            }

            return true;
        }

        Module* instantiate(const instance& entity_inst, Module* parent, const std::map<T, T>& parent_module_assignments)
        {
            std::map<T, T> signal_alias;
            std::map<T, T> instance_alias;

            const T entity_inst_name   = entity_inst.get_name();
            const T entity_inst_type   = entity_inst.get_type();
            const auto& e              = m_entities.at(entity_inst_type);
            const auto& entity_signals = e.get_signals();

            instance_alias[entity_inst_name] = get_unique_alias(m_instance_name_occurrences, entity_inst_name);

            Module* module;

            if (parent == nullptr)
            {
                module = m_netlist->get_top_module();
                module->set_name(core_strings::convert_string<T, std::string>(instance_alias.at(entity_inst_name)));
            }
            else
            {
                module = m_netlist->create_module(core_strings::convert_string<T, std::string>(instance_alias.at(entity_inst_name)), parent);
            }

            if (module == nullptr)
            {
                log_error("hdl_parser", "could not instantiate instance '{}' of entity '{}'", entity_inst_name, e.get_name());
                return nullptr;
            }

            module->set_type(core_strings::convert_string<T, std::string>(e.get_name()));

            // assign entity-level attributes
            for (const auto& attr : e.get_attributes())
            {
                if (!module->set_data("attribute", std::get<0>(attr), std::get<1>(attr), std::get<2>(attr)))
                {
                    log_warning("hdl_parser",
                                "could not set data for entity '{}' in line {}: key: {}, value_data_type: {}, value: {}",
                                e.get_name(),
                                e.get_line_number(),
                                std::get<0>(attr),
                                std::get<1>(attr),
                                std::get<2>(attr));
                }
            }

            // assign module port names
            const auto& expanded = e.get_expanded_ports();
            for (const auto& [port_name, port] : e.get_ports())
            {
                const auto direction   = port.first;
                const auto& attributes = port.second.get_attributes();

                for (const auto& expanded_name : expanded.at(port_name))
                {
                    if (const auto it = parent_module_assignments.find(expanded_name); it != parent_module_assignments.end())
                    {
                        auto net            = m_net_by_name.at(it->second);
                        m_module_ports[net] = std::make_tuple(direction, core_strings::convert_string<T, std::string>(expanded_name), module);

                        // assign port attributes
                        for (const auto& attr : attributes)
                        {
                            if (!net->set_data("attribute", std::get<0>(attr), std::get<1>(attr), std::get<2>(attr)))
                            {
                                log_warning("hdl_parser",
                                            "could not set data for signal '{}' in line {}: key: {}, value_data_type: {}, value: {}",
                                            expanded_name,
                                            port.second.get_line_number(),
                                            std::get<0>(attr),
                                            std::get<1>(attr),
                                            std::get<2>(attr));
                            }
                        }
                    }
                }
            }

            // create all internal signals
            for (const auto& [signal_name, expanded_signal] : e.get_expanded_signals())
            {
                const auto& attributes = entity_signals.at(signal_name).get_attributes();
                for (const auto& expanded_name : expanded_signal)
                {
                    signal_alias[expanded_name] = get_unique_alias(m_signal_name_occurrences, expanded_name);

                    // create new net for the signal
                    auto new_net = m_netlist->create_net(core_strings::convert_string<T, std::string>(signal_alias.at(expanded_name)));
                    if (new_net == nullptr)
                    {
                        log_error("hdl_parser", "could not instantiate net '{}' of instance '{}' of entity '{}'", expanded_name, entity_inst_name, e.get_name());
                        return nullptr;
                    }

                    m_net_by_name[signal_alias.at(expanded_name)] = new_net;

                    // assign signal attributes
                    for (const auto& attr : attributes)
                    {
                        if (!new_net->set_data("attribute", std::get<0>(attr), std::get<1>(attr), std::get<2>(attr)))
                        {
                            log_warning("hdl_parser",
                                        "could not set data for signal '{}' in line {}: key: {}, value_data_type: {}, value: {}",
                                        expanded_name,
                                        entity_signals.at(signal_name).get_line_number(),
                                        std::get<0>(attr),
                                        std::get<1>(attr),
                                        std::get<2>(attr));
                        }
                    }
                }
            }

            // schedule assigned nets for merging
            for (const auto& [s, assignment] : e.get_expanded_assignments())
            {
                T a = s;
                T b = assignment;

                if (const auto parent_it = parent_module_assignments.find(a); parent_it != parent_module_assignments.end())
                {
                    a = parent_it->second;
                }
                else if (const auto alias_it = signal_alias.find(a); alias_it != signal_alias.end())
                {
                    a = alias_it->second;
                }
                else    //if (a != "'0'" && a != "'1'" && a != "'Z'")
                {
                    log_warning("hdl_parser", "no alias for net '{}'", a);
                }

                if (const auto parent_it = parent_module_assignments.find(b); parent_it != parent_module_assignments.end())
                {
                    b = parent_it->second;
                }
                else if (const auto alias_it = signal_alias.find(b); alias_it != signal_alias.end())
                {
                    b = alias_it->second;
                }
                else if (b != "'0'" && b != "'1'" && b != "'Z'")
                {
                    log_warning("hdl_parser", "no alias for net '{}'", b);
                }

                m_nets_to_merge[b].push_back(a);
            }

            std::unordered_map<T, const GateType*> vcc_gate_types;
            std::unordered_map<T, const GateType*> gnd_gate_types;

            if constexpr (std::is_same<T, std::string>::value)
            {
                vcc_gate_types = m_netlist->get_gate_library()->get_vcc_gate_types();
                gnd_gate_types = m_netlist->get_gate_library()->get_gnd_gate_types();
            }
            else
            {
                for (const auto& [name, gt] : m_netlist->get_gate_library()->get_vcc_gate_types())
                {
                    vcc_gate_types.emplace(core_strings::convert_string<std::string, T>(name), gt);
                }

                for (const auto& [name, gt] : m_netlist->get_gate_library()->get_gnd_gate_types())
                {
                    gnd_gate_types.emplace(core_strings::convert_string<std::string, T>(name), gt);
                }
            }

            // process instances i.e. gates or other entities
            for (const auto& [inst_name, inst] : e.get_instances())
            {
                const auto& inst_type = inst.get_type();

                // will later hold either module or gate, so attributes can be assigned properly
                DataContainer* container;

                // assign actual signal names to ports
                std::map<T, T> instance_assignments;

                // expand port assignments
                std::vector<T> expanded_ports;
                std::vector<T> expanded_assignments;

                for (const auto& [port, assignments] : inst.get_port_assignments())
                {
                    const std::vector<T> expanded_port = expand_signal(port);

                    std::vector<T> expanded_assignment;
                    for (const auto& s : assignments)
                    {
                        if (s.is_binary())
                        {
                            for (const T& expanded_signal : expand_binary_signal(s))
                            {
                                expanded_assignment.push_back(expanded_signal);
                            }
                        }
                        else
                        {
                            for (const T& expanded_signal : expand_signal(s))
                            {
                                expanded_assignment.push_back(expanded_signal);
                            }
                        }
                    }

                    if (expanded_assignment.size() <= expanded_port.size())
                    {
                        for (u32 i = 0; i < expanded_assignment.size(); i++)
                        {
                            expanded_ports.push_back(expanded_port.at(i));
                            expanded_assignments.push_back(expanded_assignment.at(i));
                        }
                    }
                    else
                    {
                        for (u32 i = 0; i < expanded_port.size(); i++)
                        {
                            expanded_ports.push_back(expanded_port.at(i));
                            expanded_assignments.push_back(expanded_assignment.at(i));
                        }
                    }
                }

                for (unsigned int i = 0; i < expanded_assignments.size(); i++)
                {
                    if (const auto it = parent_module_assignments.find(expanded_assignments[i]); it != parent_module_assignments.end())
                    {
                        instance_assignments[expanded_ports[i]] = it->second;
                    }
                    else
                    {
                        if (const auto alias_it = signal_alias.find(expanded_assignments[i]); alias_it != signal_alias.end())
                        {
                            instance_assignments[expanded_ports[i]] = alias_it->second;
                        }
                        else if (expanded_assignments[i] == "'0'" || expanded_assignments[i] == "'1'" || expanded_assignments[i] == "'Z'")
                        {
                            instance_assignments[expanded_ports[i]] = expanded_assignments[i];
                        }
                        else
                        {
                            log_error("hdl_parser", "signal assignment \"{} = {}\" of instance '{}' is invalid", expanded_ports[i], expanded_assignments[i], inst_name);
                            return nullptr;
                        }
                    }
                }

                // if the instance is another entity, recursively instantiate it
                if (m_entities.find(inst_type) != m_entities.end())
                {
                    container = instantiate(inst, module, instance_assignments);
                    if (container == nullptr)
                    {
                        return nullptr;
                    }
                }
                // otherwise it has to be an element from the gate library
                else if (const auto gate_type_it = m_tmp_gate_types.find(inst_type); gate_type_it != m_tmp_gate_types.end())
                {
                    // create the new gate
                    instance_alias[inst_name] = get_unique_alias(m_instance_name_occurrences, inst_name);

                    Gate* new_gate = m_netlist->create_gate(gate_type_it->second, core_strings::convert_string<T, std::string>(instance_alias.at(inst_name)));
                    if (new_gate == nullptr)
                    {
                        log_error("hdl_parser", "could not instantiate gate '{}' within entity '{}'", inst_name, e.get_name());
                        return nullptr;
                    }

                    module->assign_gate(new_gate);
                    container = new_gate;

                    // if gate is a global type, register it as such
                    if (vcc_gate_types.find(inst_type) != vcc_gate_types.end() && !new_gate->mark_vcc_gate())
                    {
                        return nullptr;
                    }
                    if (gnd_gate_types.find(inst_type) != gnd_gate_types.end() && !new_gate->mark_gnd_gate())
                    {
                        return nullptr;
                    }

                    // cache pin types
                    std::vector<T> input_pins;
                    std::vector<T> output_pins;
                    if constexpr (std::is_same<T, std::string>::value)
                    {
                        input_pins  = new_gate->get_input_pins();
                        output_pins = new_gate->get_output_pins();
                    }
                    else
                    {
                        for (const auto& pin : new_gate->get_input_pins())
                        {
                            input_pins.push_back(core_strings::convert_string<std::string, T>(pin));
                        }

                        for (const auto& pin : new_gate->get_output_pins())
                        {
                            output_pins.push_back(core_strings::convert_string<std::string, T>(pin));
                        }
                    }

                    // check for port
                    for (const auto& [port, assignment] : instance_assignments)
                    {
                        T pin = port;

                        // get the respective net for the assignment
                        if (const auto net_it = m_net_by_name.find(assignment); net_it == m_net_by_name.end())
                        {
                            log_error("hdl_parser", "signal '{}' of entity '{}' has not been declared", assignment, inst_type);
                            return nullptr;
                        }
                        else
                        {
                            auto current_net = net_it->second;

                            // add net src/dst by pin types
                            bool is_input = false;
                            if (const auto input_it = std::find(input_pins.begin(), input_pins.end(), port); input_it != input_pins.end())
                            {
                                is_input = true;
                                pin      = *input_it;
                            }

                            bool is_output = false;
                            if (const auto output_it = std::find(output_pins.begin(), output_pins.end(), port); output_it != output_pins.end())
                            {
                                is_output = true;
                                pin       = *output_it;
                            }

                            if (!is_input && !is_output)
                            {
                                log_error("hdl_parser", "undefined pin '{}' for gate '{}' of type '{}'", port, new_gate->get_name(), new_gate->get_type()->get_name());
                                return nullptr;
                            }

                            if (is_output && !current_net->add_source(new_gate, core_strings::convert_string<T, std::string>(pin)))
                            {
                                return nullptr;
                            }

                            if (is_input && !current_net->add_destination(new_gate, core_strings::convert_string<T, std::string>(pin)))
                            {
                                return nullptr;
                            }
                        }
                    }
                }
                else
                {
                    log_error("hdl_parser", "could not find gate type '{}' in gate library '{}'", inst_type, m_netlist->get_gate_library()->get_name());
                    return nullptr;
                }

                // assign instance attributes
                for (const auto& attr : inst.get_attributes())
                {
                    if (!container->set_data("attribute", std::get<0>(attr), std::get<1>(attr), std::get<2>(attr)))
                    {
                        log_warning("hdl_parser",
                                    "could not set data for instance '{}' in line {}: key: {}, value_data_type: {}, value: {}",
                                    inst_name,
                                    inst.get_line_number(),
                                    std::get<0>(attr),
                                    std::get<1>(attr),
                                    std::get<2>(attr));
                    }
                }

                // process generics
                for (const auto& [generic_name, generic] : inst.get_generic_assignments())
                {
                    if (!container->set_data("generic", generic_name, generic.first, generic.second))
                    {
                        log_warning("hdl_parser",
                                    "could not set data for instance '{}' in line {}: key: {}, value_data_type: {}, value: {}",
                                    inst_name,
                                    inst.get_line_number(),
                                    generic_name,
                                    generic.first,
                                    generic.second);
                    }
                }
            }

            return module;
        }

        T get_unique_alias(std::unordered_map<T, u32>& name_occurrences, const T& name)
        {
            // if the name only appears once, we don't have to suffix it
            if (name_occurrences[name] < 2)
            {
                return name;
            }

            name_occurrences[name]++;

            // otherwise, add a unique string to the name
            return name + core_strings::convert_string<std::string, T>("__[" + std::to_string(name_occurrences[name]) + "]__");
        }

        std::vector<T> expand_binary_signal(const signal& s)
        {
            std::vector<T> res;

            for (auto bin_value : s.get_name())
            {
                res.push_back("'" + T(1, bin_value) + "'");
            }

            return res;
        }

        std::vector<T> expand_signal(const signal& s)
        {
            std::vector<T> res;

            expand_signal_recursively(res, s.get_name(), s.get_ranges(), 0);

            return res;
        }

        void expand_signal_recursively(std::vector<T>& expanded_signal, const T& current_signal, const std::vector<std::vector<u32>>& ranges, u32 dimension)
        {
            // expand signal recursively
            if (ranges.size() > dimension)
            {
                for (const auto& index : ranges[dimension])
                {
                    expand_signal_recursively(expanded_signal, current_signal + "(" + core_strings::convert_string<std::string, T>(std::to_string(index)) + ")", ranges, dimension + 1);
                }
            }
            else
            {
                // last dimension
                expanded_signal.push_back(current_signal);
            }
        }

        std::vector<T> expand_signal_vector(const std::vector<signal>& signals, bool allow_binary)
        {
            std::vector<T> res;

            for (const auto& s : signals)
            {
                std::vector<T> expanded;

                if (allow_binary && s.is_binary())
                {
                    expanded = expand_binary_signal(s);
                }
                else
                {
                    expanded = expand_signal(s);
                }

                res.insert(expanded.begin(), expanded.end());
            }

            return res;
        }
    };
}    // namespace hal
