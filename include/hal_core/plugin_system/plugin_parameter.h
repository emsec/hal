// MIT License
// 
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <string>

namespace hal
{
    /**
     * A single configurable parameter of a plugin, described by its type, tag name, label, and current value.
     * The GUI uses these descriptions to render input widgets for a plugin without knowing anything about the plugin itself.
     */
    class PluginParameter
    {
    public:
        /**
         * The kind of value that a parameter holds, which also determines how it is presented to the user.
         */
        enum ParameterType
        {
            Absent,          /**< No parameter, used as the default of an unset parameter. */
            Boolean,         /**< A boolean value. */
            Color,           /**< A color. */
            ComboBox,        /**< One value picked from a list of alternatives. */
            Dictionary,      /**< A collection of key-value pairs. */
            ExistingDir,     /**< The path to a directory that already exists. */
            ExistingFile,    /**< The path to a file that already exists. */
            Float,           /**< A floating point number. */
            Gate,            /**< A gate of the netlist. */
            Integer,         /**< An integer number. */
            Label,           /**< A read-only text that is displayed to the user but cannot be edited. */
            Module,          /**< A module of the netlist. */
            NewFile,         /**< The path to a file that is yet to be created. */
            PushButton,      /**< A button that triggers an action instead of holding a value. */
            String,          /**< An arbitrary text. */
            TabName          /**< No value of its own, it starts a new tab that groups the parameters that follow. */
        };

    private:
        ParameterType m_type;
        std::string m_tagname;
        std::string m_label;
        std::string m_value;

    public:
        /**
         * Construct a new plugin parameter.
         *
         * @param[in] tp - The type of the parameter. Defaults to `PluginParameter::Absent`.
         * @param[in] tag - The tag name by which the plugin identifies the parameter. Defaults to an empty string.
         * @param[in] lbl - The label that is shown to the user. Defaults to an empty string.
         * @param[in] val - The initial value. Defaults to an empty string.
         */
        PluginParameter(ParameterType tp=Absent, const std::string& tag=std::string(), const std::string& lbl=std::string(), const std::string& val=std::string())
            : m_type(tp), m_tagname(tag), m_label(lbl), m_value(val) {;}

        /**
         * Get the tag name by which the plugin identifies the parameter.
         *
         * @returns The tag name.
         */
        std::string get_tagname() const { return m_tagname; }

        /**
         * Get the label that is shown to the user.
         *
         * @returns The label.
         */
        std::string get_label() const { return m_label; }

        /**
         * Get the value of the parameter.
         * The value is always stored as a string, no matter which type the parameter has.
         *
         * @returns The value.
         */
        std::string get_value() const { return m_value; }

        /**
         * Get the type of the parameter.
         *
         * @returns The type.
         */
        ParameterType get_type() const { return m_type; }

        /**
         * Set the value of the parameter.
         * The value is always stored as a string, no matter which type the parameter has.
         *
         * @param[in] v - The new value.
         */
        void set_value(const std::string& v)  { m_value = v; }

        /**
         * Set the tag name by which the plugin identifies the parameter.
         *
         * @param[in] tg - The new tag name.
         */
        void set_tagname(const std::string& tg) { m_tagname = tg; }
    };
}
