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
#include "hal_core/defines.h"
#include "gui/basic_tree_model/base_tree_model.h"
#include <QMap>
#include <QList>

namespace hal
{

    class Gate;

    class PinTreeItem : public BaseTreeItem
    {
    public:
        enum Type {None, Pin, Group};

        private:
            Type mType;
            QList<u32> mNetIds;
            std::string mPinName;
            QString mPinDirection;
            QString mPinType;
            QString mNetName;
            int mIndex;
        public:

            PinTreeItem(const std::string& pinName, QString pinDirection, QString pinTypee, QString netName, int inx);
            PinTreeItem();
            QVariant getData(int column) const override;
            void setData(QList<QVariant> data) override;
            void setDataAtIndex(int index, QVariant& data) override;
            void appendData(QVariant data) override;
            int getColumnCount() const override;
            void setType(Type tp) { mType = tp; }

            /**
             * Get the type (enum) of a given item.
             *
             * @return The item's type.
             */
            Type type() const { return mType; }
            void setNetIds(const QList<u32>& nids) { mNetIds = nids; }

            /**
             * Get the connected nets for a given treeitem (represents a pin). If the
             * item is grouping type or the pin has no connected net, an empty list
             * is returned. In case of an inout pin, even multiple connected nets are possible.
             *
             * @return A list of net ids.
             */
            QList<u32> netIds() const { return mNetIds; }
    };

/**
 * @ingroup gui
 * @brief A model to display the pins of a gate.
 */
class GatePinsTreeModel : public BaseTreeModel
{
    Q_OBJECT

public:

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

private:
    int mGateId;
    QMap<std::string, BaseTreeItem*> mPinGroupToTreeItem;

};

}
