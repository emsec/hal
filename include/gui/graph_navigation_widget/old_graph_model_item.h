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

#ifndef old_GRAPH_MODEL_ITEM_H
#define old_GRAPH_MODEL_ITEM_H

#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/netlist.h"
#include <QList>
#include <QVariant>

enum class item_type
{
    gate = 0,
    net  = 1
};

/*the tablemodelitemclass, also functions as a adapter too*/
class old_graph_model_item
{
public:
    old_graph_model_item();

    /*constructor for the gate-item*/
    old_graph_model_item(std::shared_ptr<gate> gate);

    /*constructor for the net-item*/
    old_graph_model_item(std::shared_ptr<net> net);

    ~old_graph_model_item();

    /*adapter-stuff*/
    QString getName();

    QString getType();

    QString getID();

    QString getLocation();

    item_type getItemType();

    /*functions that may be needed in the future*/
    void setgate(std::shared_ptr<gate> gate);

    void setNet(std::shared_ptr<net> net);

private:
    /*adapter-stuff*/
    std::shared_ptr<gate> refgate;

    std::shared_ptr<net> refNet;

    QString name;

    QString type;

    QString id;

    QString location;

    item_type i_type;
};

#endif    // old_GRAPH_MODEL_ITEM_H
