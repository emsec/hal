#pragma once

#include "hal_core/defines.h"

#include <vector>

namespace hal
{
    class VerilogSignal
    {
    public:
        /**
         * Constructs a VerilogSignal object.
         *
         * @param[in] line_number - The line number of the signal definition within the HDL file.
         * @param[in] name - The name of the signal.
         * @param[in] ranges - The signal range for multi-bit signals, e.g., '{0, 1, 2, 3}' for a four bit signal.
         * @param[in] binary - Binary signal ('true') or not ('false'). If true, the signal name holds the signals values as bits.
         * @param[in] ranges_known - Ranges are known on construction ('true') or not ('false').
         */
        VerilogSignal(u32 line_number, const std::string& name, const std::vector<std::vector<u32>>& ranges = {}, bool binary = false, bool ranges_known = true)
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
        const std::string& get_name() const
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
        bool operator<(const VerilogSignal& other) const
        {
            return (m_name < other.get_name()) && (m_ranges < other.get_ranges());
        }

    private:
        u32 m_line_number;

        // name (may either be the identifier of the signal or a binary string in case of direct assignments)
        std::string m_name;

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
}    // namespace hal