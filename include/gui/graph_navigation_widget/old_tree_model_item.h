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

#ifndef _GRAPH_TREE_ITEM_H_
#define _GRAPH_TREE_ITEM_H_

#include "netlist/gate.h"
#include "netlist/net.h"
#include <QList>
#include <QString>
#include <QVariant>

/*later maybe with flags, would be much better (i think)*/
enum class item_type_tree
{
    net      = 0,
    gate     = 1,
    dummy    = 2,    //needed for treeitems that are not part of the actual graph
    name     = 3,
    id       = 4,
    location = 5,
    type     = 7
};

class old_tree_model_item
{
public:
    /*A constructor just for the root-item*/
    old_tree_model_item(old_tree_model_item* parent = 0);

    /*dummy-constructor for items that dont belong to the graph(like inoutnet)*/
    old_tree_model_item(QString item_name, QString beschr, old_tree_model_item* parent = 0);

    /*constructor for the net-item*/
    old_tree_model_item(std::shared_ptr<net> net, old_tree_model_item* parent);

    /*constructor for the gate-item*/
    old_tree_model_item(std::shared_ptr<gate> gate, old_tree_model_item* parent);

    /*constructor for the properties-items(needs to have a reference to the gate)*/
    old_tree_model_item(std::shared_ptr<gate> refgate, item_type_tree proptype, old_tree_model_item* parent);

    ~old_tree_model_item();

    /*functions for the treemodel*/
    void appendChild(old_tree_model_item* child);

    old_tree_model_item* child(int row);

    int childCount() const;

    int columnCount() const;

    QVariant data(int column) const;

    int row() const;

    old_tree_model_item* parent();

    /*adapter-stuff*/
    QString getName();

    QString getID();

    QString getType();

    QString getLocation();

    item_type_tree getItemType();

    /*functions that are needed if you want to change the graphdata through the model*/
    void setName();

    void setType();

    void setLocation();

    void setID();

private:
    /*stuff for the treemodel*/
    QList<old_tree_model_item*> children;

    old_tree_model_item* parentItem;

    QList<QVariant> item_data;

    /*adapter-stuff*/
    item_type_tree typ;

    std::shared_ptr<net> refNet;

    std::shared_ptr<gate> refgate;

    QString name;

    QString id;

    QString type;

    QString location;
};

#endif    //_GRAPH_TREE_ITEM_H_
