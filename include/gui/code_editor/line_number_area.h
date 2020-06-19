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

#include <QWidget>
namespace hal{
class code_editor;

class line_number_area : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int left_offset READ left_offset WRITE set_left_offset)
    Q_PROPERTY(int right_offset READ right_offset WRITE set_right_offset)

public:
    explicit line_number_area(code_editor* editor);

    int left_offset() const;
    int right_offset() const;

    void set_left_offset(const int offset);
    void set_right_offset(const int offset);

    void repolish();

protected:
    void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent* event) Q_DECL_OVERRIDE;

private:
    code_editor* m_editor;

    int m_left_offset;
    int m_right_offset;
};
}
