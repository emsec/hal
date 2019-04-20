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

#ifndef GRAPH_LAYOUTER_MANAGER_H
#define GRAPH_LAYOUTER_MANAGER_H

#include <QList>
#include <QObject>
#include <QPair>

class graph_layouter;
class graph_scene_manager;
class graph_scene;

class graph_layouter_manager : public QObject
{
    Q_OBJECT

public:
    static graph_layouter_manager& get_instance();
    ~graph_layouter_manager();

    graph_scene* subscribe(QString layouter);
    void unsubscribe(QString layouter);

    QStringList get_layouters();
    QString get_name(QString layouter);
    QString get_description(QString layouter);

private:
    explicit graph_layouter_manager(QObject* parent = 0);

    QList<QPair<graph_layouter*, int>> m_layouters;
    QList<QPair<graph_layouter*, graph_scene_manager*>> m_scenes;
};

#endif    // GRAPH_LAYOUTER_MANAGER_H
