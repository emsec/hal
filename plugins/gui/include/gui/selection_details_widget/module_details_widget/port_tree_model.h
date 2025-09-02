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
#include "hal_core/netlist/gate_library/enums/pin_direction.h"
#include "hal_core/netlist/gate_library/enums/pin_event.h"
#include "hal_core/netlist/gate_library/enums/pin_type.h"
#include <QMap>

namespace hal
{
    class Module;
    class Net;


    class PortTreeItem : public BaseTreeItem
    {
    public:
        enum Type {None, Pin, Group};

        private:
            Type mItemType;
            u32 mId;
            QString mPinName;
            PinDirection mPinDirection;
            PinType mPinType;
            QString mNetName;
            int mIndex;

        public:

            PortTreeItem(Type itype, u32 id_, QString pinName, PinDirection dir, PinType ptype, int inx, QString netName = QString());
            PortTreeItem() : mItemType(None), mId(0) {;}
            QVariant getData(int column) const override;
            void setData(QList<QVariant> data) override;
            void setDataAtIndex(int index, QVariant& data) override;
            void appendData(QVariant data) override;
            int getColumnCount() const override;
            void setItemType(Type tp) { mItemType = tp; }
            Type itemType() const { return mItemType; }
            QString name() const { return mPinName; }
            void setName(const QString& nam) { mPinName = nam; }
            void setPinType(PinType ptype) { mPinType = ptype; }
            void setPinDirection(PinDirection dir) { mPinDirection = dir; }
            void setIndex(int inx) { mIndex = inx; }

            /**
             * Returns the pin-id if the item represents a pin or the pingroup-id
             * if the item represents a pingroup.
             *
             * @param item - The item.
             * @return The pin- or pingroup-id.
             */
            u32 id() const { return mId; }
    };

    /**
     * @brief A model to represent the ports of a module.
     */
    class ModulePinsTreeModel : public BaseTreeModel
    {
        Q_OBJECT
    public:

        /**
         * The constructor.
         *
         * @param parent - THe model's parent.
         */
        ModulePinsTreeModel(QObject* parent = nullptr);

        /**
          * The destructor.
          */
        ~ModulePinsTreeModel();

        //DRAG AND DROP THINGS!
        Qt::ItemFlags flags(const QModelIndex& index) const override;
        QStringList mimeTypes() const override;
        QMimeData* mimeData(const QModelIndexList &indexes) const override;
        bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
        bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;


        /**
         *  Overwritten BaseTreeModel function.
         */
        void clear() override;

        /**
         * Sets up the model to display the module's ports.
         *
         * @param m - The module for which to display the ports.
         */
        void setModule(Module* m);

        /**
         * Get the underlying net from an (port) item.
         * If this model does not represent a module or
         * an invalid (port) item is given a nullptr is returned.
         *
         * @param item - The (port) item.
         * @return The net or nullptr.
         */
        Net* getNetFromItem(PortTreeItem* item);

        /**
         * Get the id of the module that is currently represented.
         * If no module is represented, -1 is returned.
         *
         * @return The module id.
         */
        int getRepresentedModuleId();

        /** @name Event Handler Functions
         */
        ///@{
        void handleModulePortsChanged(Module* m, PinEvent pev, u32 pgid);
        ///@}

        //column identifier
        static const int sNameColumn = 0;
        static const int sDirectionColumn = 1;
        static const int sTypeColumn = 2;
        static const int sNetColumn = 3;

    Q_SIGNALS:
        /**
         * Q_SIGNAL that is emitted when the number of the model's port changed.
         * (Primarily used to update 'the' section header in the details widget)
         *
         * @param newNumber - The new port count.
         */
        void numberOfPortsChanged(const int newNumber);

    private:        
        Module* mModule;
        //name is (hopefully) enough to identify
        QMap<QString, BaseTreeItem*> mNameToTreeItem;
        QMap<int, PortTreeItem*> mIdToPinItem;
        QMap<int, PortTreeItem*> mIdToGroupItem;

        void insertItem(PortTreeItem* item, BaseTreeItem* parent, int index);
        void removeItem(PortTreeItem* item);
        void updateGroupIndex(PortTreeItem* groupItem);

        // helper functions for dnd for more clarity
        void dndGroupOnGroup(BaseTreeItem* droppedGroup, BaseTreeItem* onDroppedGroup);
        void dndGroupBetweenGroup(PortTreeItem* droppedGroup, int row);
        void dndPinOnGroup(PortTreeItem* droppedPin, BaseTreeItem* onDroppedGroup);
        void dndPinBetweenPin(PortTreeItem* droppedPin, BaseTreeItem* onDroppedParent, int row);
        void dndPinBetweenGroup(PortTreeItem* droppedPin, int row);
    };
}
