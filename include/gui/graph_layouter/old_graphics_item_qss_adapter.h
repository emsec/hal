//  MIT License
//
//  Copyright (c) 2019 Marc Fyrbiak
//  Copyright (c) 2019 Sebastian Wallat
//  Copyright (c) 2019 Max Hoffmann
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

#ifndef OLD_GRAPHICS_ITEM_QSS_ADAPTER
#define OLD_GRAPHICS_ITEM_QSS_ADAPTER

#include <QWidget>
#include <QColor>
#include <QDebug>

class old_graphics_item_qss_adapter : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor gate_default_color READ gate_default_color WRITE set_gate_default_color)
    Q_PROPERTY(QColor gate_background_color READ gate_background_color WRITE set_gate_background_color)
    Q_PROPERTY(QColor gate_font_color READ gate_font_color WRITE set_gate_font_color)
    Q_PROPERTY(QColor gate_selected_background_color READ gate_selected_background_color WRITE set_gate_selected_background_color)
    Q_PROPERTY(QColor gate_selected_font_color READ gate_selected_font_color WRITE set_gate_selected_font_color)

    Q_PROPERTY(QColor net_default_color READ net_default_color WRITE set_net_default_color)
    Q_PROPERTY(QColor net_selected_color READ net_selected_color WRITE set_net_selected_color)
    Q_PROPERTY(QColor net_global_input_output_color READ net_global_input_output_color WRITE set_net_global_input_output_color)

    Q_PROPERTY(QString tree_navigation_open_folder_style READ tree_navigation_open_folder_style WRITE set_tree_navigation_open_folder_style)
    Q_PROPERTY(QString tree_navigation_open_folder_path READ tree_navigation_open_folder_path WRITE set_tree_navigation_open_folder_path)

public:
    static old_graphics_item_qss_adapter* instance();

    QColor gate_default_color() const;
    QColor gate_background_color() const;
    QColor gate_font_color() const;
    QColor gate_selected_background_color() const;
    QColor gate_selected_font_color() const;

    QColor net_default_color() const;
    QColor net_selected_color() const;
    QColor net_global_input_output_color() const;

    QString tree_navigation_open_folder_style() const;
    QString tree_navigation_open_folder_path() const;

    void set_gate_default_color(const QColor& color);
    void set_gate_background_color(const QColor& color);
    void set_gate_font_color(const QColor& color);
    void set_gate_selected_background_color(const QColor& color);
    void set_gate_selected_font_color(const QColor& color);

    void set_net_default_color(const QColor& color);
    void set_net_selected_color(const QColor& color);
    void set_net_global_input_output_color(const QColor& color);

    void set_tree_navigation_open_folder_style(const QString& style);
    void set_tree_navigation_open_folder_path(const QString& path);

    void print_colors(){
        qDebug() << m_gate_default_color.name() << m_gate_background_color.name() << m_gate_font_color.name() << m_gate_selected_background_color.name()
                 << m_gate_selected_font_color.name() << m_net_default_color.name() << m_net_selected_color.name() << m_net_global_input_output_color.name();
    };

private:
    explicit old_graphics_item_qss_adapter(QWidget* parent = nullptr);

    QColor m_gate_default_color; //beschreibt den rand
    QColor m_gate_background_color;
    QColor m_gate_font_color;
    QColor m_gate_selected_background_color;
    QColor m_gate_selected_font_color;

    QColor m_net_default_color;
    QColor m_net_selected_color;
    QColor m_net_global_input_output_color;

    QString m_tree_navigation_open_folder_style;
    QString m_tree_navigation_open_folder_path;
};

#endif //OLD_GRAPHICS_ITEM_QSS_ADAPTER
