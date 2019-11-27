/*
 * MIT License
 *
 * Copyright (c) 2019 Chair for Embedded Security, Ruhr-University Bochum
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, includig without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __MODULE_TREE_VIEW_H
#define __MODULE_TREE_VIEW_H

#include <QTreeView>

//This class is neccessary to prevent that the right click on the tree-view does not select the
//underlying item, otherwise you cant select the option "add selection to module" in
//the contextmenu
class module_tree_view : public QTreeView
{
    Q_OBJECT
public:
    module_tree_view(QWidget* parent = nullptr);
    void mousePressEvent(QMouseEvent *event);

};


#endif //__MODULE_TREE_VIEW_H
