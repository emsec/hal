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

#ifndef STYLE_MANAGER_H
#define STYLE_MANAGER_H

#include <QObject>
namespace hal{
class graphics_qss_adapter;
class notification_manager_qss_adapter;
class python_qss_adapter;
class shared_properties_qss_adapter;
class vhdl_qss_adapter;

class QApplication;

class style_manager : public QObject
{
    Q_OBJECT

public:
    static style_manager* get_instance();

    ~style_manager();

    style_manager(style_manager const&) = delete;
    void operator=(style_manager const&) = delete;

    void update_style();

    const shared_properties_qss_adapter* shared_properties() const;
    const notification_manager_qss_adapter* notification_manager() const;
    const python_qss_adapter* python_syntax_highlighter() const;
    const vhdl_qss_adapter* vhdl_syntax_highlighter() const;

private:
    explicit style_manager(QObject* parent = nullptr);
    void set_default(QApplication* app);

    graphics_qss_adapter* m_graphics_qss_adapter;
    shared_properties_qss_adapter* m_shared_properties;
    notification_manager_qss_adapter* m_notification_manager;
    python_qss_adapter* m_python_syntax_highlighter;
    vhdl_qss_adapter* m_vhdl_syntax_highlighter;
};
}

#endif    // STYLE_MANAGER_H
