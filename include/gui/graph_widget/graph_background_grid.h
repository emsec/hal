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

#ifndef GRAPH_BACKGROUND_GRID_H
#define GRAPH_BACKGROUND_GRID_H

#include <QWidget>

class graph_background_grid : public QWidget
{
    Q_OBJECT

public:
    explicit graph_background_grid(QWidget* parent = 0);

    void paintEvent(QPaintEvent*) Q_DECL_OVERRIDE;

    void set_grid_size(int size);
    void set_cluster_size(int size);
    bool grid_visible();

private Q_SLOTS:
    void set_draw_grid_false();
    void set_draw_grid_true();

private:
    int m_grid_size;
    int m_cluster_size;

    bool m_draw_grid;
};

#endif    // GRAPH_BACKGROUND_GRID_H
