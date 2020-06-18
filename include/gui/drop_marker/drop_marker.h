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

#ifndef DROP_MARKER_H
#define DROP_MARKER_H

#include <QFrame>

class QPropertyAnimation;

namespace hal{

class drop_marker : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(int fixed_width READ fixed_width WRITE set_fixed_width)
    Q_PROPERTY(int fixed_height READ fixed_height WRITE set_fixed_height)

public:
    drop_marker(Qt::Orientation orientation, QWidget* parent = nullptr);

    int fixed_width();
    int fixed_height();

    void set_fixed_width(int width);
    void set_fixed_height(int height);

    void set_end_value(int value);

    void expand();
    void collapse();

    void reset();

private:
    QPropertyAnimation* m_animation;

    Qt::Orientation m_orientation;

    int m_fixed_width;
    int m_fixed_height;
};
}

#endif    // DROP_MARKER_H
