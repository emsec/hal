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
    class PluginParameter
    {
    public:
        enum ParameterType { Absent, Boolean, Color, Dictionary, ExistingDir, Float, Gate, Integer, Module, NewFile, PushButton, String, TabName };
    private:
        ParameterType m_type;
        std::string m_tagname;
        std::string m_label;
        std::string m_value;
    public:
        PluginParameter(ParameterType tp=Absent, const std::string& tag=std::string(), const std::string& lbl=std::string(), const std::string& val=std::string())
            : m_type(tp), m_tagname(tag), m_label(lbl), m_value(val) {;}
        std::string get_tagname() const { return m_tagname; }
        std::string get_label() const { return m_label; }
        std::string get_value() const { return m_value; }
        ParameterType get_type() const { return m_type; }
        void set_value(const std::string& v)  { m_value = v; }
        void set_tagname(const std::string& tg) { m_tagname = tg; }
    };
}
