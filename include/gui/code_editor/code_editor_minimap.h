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

#ifndef CODE_EDITOR_MINIMAP_H
#define CODE_EDITOR_MINIMAP_H

#include <QWidget>

class QTextDocument;

namespace hal {

class code_editor;
class minimap_scrollbar;

class code_editor_minimap : public QWidget
{
    Q_OBJECT

public:
    explicit code_editor_minimap(code_editor* editor);

    minimap_scrollbar* scrollbar();
    QTextDocument* document();

    void adjust_slider_height(int viewport_height);
    void adjust_slider_height(qreal ratio);
    void adjust_slider_height(int first_visible_block, int last_visible_block);

    void repolish();

public Q_SLOTS:
    void handle_document_size_changed(const QSizeF& new_size);
    void handle_contents_change(int position, int chars_removed, int chars_added);

protected:
    void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent* event) Q_DECL_OVERRIDE;

private:
    void resize_scrollbar();

    code_editor* m_editor;
    QTextDocument* m_document;
    minimap_scrollbar* m_scrollbar;

    int m_document_height;
    qreal m_offset;
};
}

#endif    // CODE_EDITOR_MINIMAP_H
