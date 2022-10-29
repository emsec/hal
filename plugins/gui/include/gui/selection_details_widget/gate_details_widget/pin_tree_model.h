// MIT License
// 
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

//#include "gui/new_selection_details_widget/models/base_tree_model.h"
#include "gui/basic_tree_model/base_tree_model.h"
#include <QMap>

namespace hal
{

    class Gate;

/**
 * @ingroup gui
 * @brief A model to display the pins of a gate.
 */
class GatePinsTreeModel : public BaseTreeModel
{
    Q_OBJECT

public:

    //metatype declaration at the end of file
    enum class itemType {grouping = 0, pin = 1};

    /**
     * The constructor.
     *
     * @param parent - The model's parent.
     */
    GatePinsTreeModel(QObject* parent = nullptr);

    /**
      * The destructor.
      */
    ~GatePinsTreeModel();

    /**
     * Overwritten BaseTreeModel function.
     */
    void clear() override;

    /**
     * Sets the gate of the pins this model will represent.
     *
     * @param g - The gate of the pins.
     */
    void setGate(Gate* g);

    /**
     * Get the ID of the gate that is currently displayed. If no gate is
     * currently displayed, -1 is returned.
     *
     * @return The gate's id.
     */
    int getCurrentGateID();

    /**
     * Get the connected nets for a given treeitem (represents a pin). If the
     * item is grouping type or the pin has no connected net, an empty list
     * is returned. In case of an inout pin, even multiple connected nets are possible.
     *
     * @param item - The treeitem from which to get the connected nets.
     * @return A list of net ids.
     */
    QList<int> getNetIDsOfTreeItem(TreeItem* item);

    /**
     * Get the type (enum) of a given item.
     *
     * @param item - The item for which the type is requested.
     * @return The item's type.
     */
    itemType getTypeOfItem(TreeItem* item);

    /**
     * Get the number of displayed pins (the number of pins of all types).
     *
     * @return The number of pins.
     */
    int getNumberOfDisplayedPins();


    //column identifier
    static const int sNameColumn = 0;
    static const int sDirectionColumn = 1;
    static const int sTypeColumn = 2;
    static const int sConnectedNetColumn = 3;

    //additional data keys
    const QString keyType = "type";
    const QString keyRepresentedNetsID = "netID"; //might not be needed

private:
    int mGateId;
    QMap<std::string, TreeItem*> mPinGroupingToTreeItem;

};

}

Q_DECLARE_METATYPE(hal::GatePinsTreeModel::itemType)
