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

#include "code_editor/code_editor_minimap.h"

#include <QPlainTextEdit>

class QPaintEvent;
class QPropertyAnimation;
class QResizeEvent;

namespace hal{

class code_editor_minimap;
class code_editor_scrollbar;
class line_number_area;

class code_editor : public QPlainTextEdit
{
    Q_OBJECT
    Q_PROPERTY(QFont line_number_font READ line_number_font WRITE set_line_number_font)
    Q_PROPERTY(QColor line_number_color READ line_number_color WRITE set_line_number_color)
    Q_PROPERTY(QColor line_number_background READ line_number_background WRITE set_line_number_background)
    Q_PROPERTY(QColor line_number_highlight_color READ line_number_highlight_color WRITE set_line_number_highlight_color)
    Q_PROPERTY(QColor line_number_highlight_background READ line_number_highlight_background WRITE set_line_number_highlight_background)
    Q_PROPERTY(QColor current_line_background READ current_line_background WRITE set_current_line_background)

public:
    explicit code_editor(QWidget* parent = nullptr);

    virtual bool eventFilter(QObject* object, QEvent* event) Q_DECL_OVERRIDE;

    void line_number_area_paint_event(QPaintEvent* event);
    void minimap_paint_event(QPaintEvent* event);

    int line_number_area_width();
    int minimap_width();

    int first_visible_block();
    int visible_block_count();

    void center_on_line(const int number);

    void handle_wheel_event(QWheelEvent* event);

    code_editor_minimap* minimap();

    QFont line_number_font() const;
    QColor line_number_color() const;
    QColor line_number_background() const;
    QColor line_number_highlight_color() const;
    QColor line_number_highlight_background() const;
    QColor current_line_background() const;

    void set_line_number_font(QFont& font);
    void set_line_number_color(QColor& color);
    void set_line_number_background(QColor& color);
    void set_line_number_highlight_color(QColor& color);
    void set_line_number_highlight_background(QColor& color);
    void set_current_line_background(QColor& color);

public Q_SLOTS:
    void search(const QString& string);

    void toggle_line_numbers();
    void toggle_minimap();

protected:
    virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void highlight_current_line();
    void handle_block_count_changed(int new_block_count);
    void update_line_number_area(const QRect& rect, int dy);
    void update_minimap(const QRect& rect, int dy);
    void handle_global_setting_changed(void* sender, const QString& key, const QVariant& value);

private:
    void update_layout();
    void clear_line_highlight();

    code_editor_scrollbar* m_scrollbar;

    line_number_area* m_line_number_area;
    code_editor_minimap* m_minimap;

    QPropertyAnimation* m_animation;

    bool m_line_numbers_enabled;
    bool m_line_highlight_enabled;
    bool m_minimap_enabled;
    bool m_line_wrap_enabled;

    QFont m_line_number_font;
    QColor m_line_number_color;
    QColor m_line_number_background;
    QColor m_line_number_highlight_color;
    QColor m_line_number_highlight_background;
    QColor m_current_line_background;
};
}
