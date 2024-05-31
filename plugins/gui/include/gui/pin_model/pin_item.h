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

#include "gui/basic_tree_model/base_tree_model.h"
#include "hal_core/defines.h"
#include "hal_core/netlist/gate_library/enums/pin_direction.h"
#include "hal_core/netlist/gate_library/enums/pin_type.h"
#include "hal_core/netlist/pins/gate_pin.h"

#include <QColor>
#include <QList>
#include <QString>
#include <QVariant>

namespace hal
{
    /**
     * @ingroup gui
     * @brief An item in the PinModel.
     *
     * The PinItem is one item in the PinModel item model.
     */
    class PinItem : public BaseTreeItem
    {
    public:
        enum class TreeItemType {PinGroup, Pin, GroupCreator, PinCreator, InvalidPinGroup, InvalidPin};

        void setData(QList<QVariant> data) override;
        void setDataAtIndex(int index, QVariant& data) override;
        void appendData(QVariant data) override;
        int getColumnCount() const override;

        /**
         * Constructor.
         *
         */
        PinItem(TreeItemType type);

        QVariant getData(int column) const override;

        int row() const;

        u32 getId() const;
        QString getName() const;
        QString getType() const;
        QString getDirection() const;

        TreeItemType getItemType() const;

        /**
         * Sets the name of this PinItem
         *
         * @param name - The new name
         */
        void setName(const QString& name);
        void setDirection(const QString& direction);
        void setId(u32 newId);
        void setType(const QString& type);

        void setDirection(PinDirection direction);
        void setType(PinType type);

        void setFields(GatePin* pin);
        void setFields(const QString& name, u32 id, PinDirection direction, PinType type);
        void setItemType(TreeItemType type);


    private:
        TreeItemType mItemType;
        u32 mId;
        QString mName;
        PinDirection mDirection;
        PinType mType;

    };
}
