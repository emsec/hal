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
#include "user_action.h"
#include "hal_core/netlist/module.h"
#include <QString>
#include <QObject>

namespace hal
{

    class PinAction : public QObject
    {
        Q_OBJECT
    public:
        enum Type { None, Create, DeleteGroup, RemovePin, MovePin, MoveGroup, RenamePin, RenameGroup, TypeChange, MaxAction };
        Q_ENUM(Type)
    public:
        static QString toString(Type tp);
        static Type fromString(const QString& s);
    };

    int pinGroupIndex(const Module* mod, const PinGroup<Module>* pgrp);
    /**
     * @ingroup user_action
     * @brief Pingroup user actions
     *
     * Action depends on PinAction::Type:
     *
     * Create:
     *     Pingroup with given name gets created.
     *     Pins listed in pinIds get moved into new group
     *     Id of created group returned as targetGroupId()
     *
     * DeleteGroup:
     *     Pingroup ID=sourceGroupId gets deleted.
     *     Pins in group are stored for undo command
     *
     * RemovePin:
     *     Remove pin from sourceGroup
     *
     * MovePin:
     *     Must use move constructor with mandatory arguments pinId and pinOrderNo
     *     One out of targetIndex (existing pingroup) or name (create new pingroup)
     *       must be given to indicate destination
     *
     * MoveGroup:
     *     Move Group identified by sourceGroupId to position groupOrderNo
     * RenamePin:
     *     Set new name to first pin in pinIds
     * RenameGroup:
     *     Set new name to group identified by targetGroupId
     */
    class ActionPingroup : public UserAction
    {
    private:
        QList<PinAction::Type> mPinActions;
        QList<u32> mPinIds;
        u32 mSourceGroupId;
        u32 mTargetGroupId;
        int mPinOrderNo;
        int mGroupOrderNo;
        QString mName;
    public:
        /**
         * Action Constructor.
         *
         * @param type - The UserActionObjectType of the item that should be created (default type: None)
         * @param objName - The name of the object to create (default name: "").
         */
        ActionPingroup(PinAction::Type action = PinAction::None, u32 pingroupId = 0, const QString& name=QString());
        ActionPingroup(u32 pinId, int pinIndex, u32 tgtgroupId=0, const QString& name=QString(), int grpIndex=-1); // action = MovePin
        bool exec() override;
        QString tagname() const override;
        void writeToXml(QXmlStreamWriter& xmlOut) const override;
        void readFromXml(QXmlStreamReader& xmlIn) override;
        void addToHash(QCryptographicHash& cryptoHash) const override;
        void setPinIds(const QList<u32>& ids) { mPinIds = ids; }
        void setPinId(u32 id) { mPinIds.clear(); mPinIds.append(id); }
        void addPinAction(PinAction::Type action) { mPinActions.prepend(action); }
        void setSourceGroupId(u32 id) { mSourceGroupId = id; }
        void setTargetGroupId(u32 id) { mTargetGroupId = id; }
        void setPinOrderNo(int inx)   { mPinOrderNo   = inx; }
        void setGroupOrderNo(int inx) { mGroupOrderNo = inx; }
        void setName(const QString& name) { mName = name; }
        u32 targetGroupId() const { return mTargetGroupId; }
    };

    /**
     * @ingroup user_action
     * @brief UserActionFactory for ActionPingroup
     */
    class ActionPingroupFactory : public UserActionFactory
    {
    public:
        ActionPingroupFactory();
        UserAction* newAction() const;
        static ActionPingroupFactory* sFactory;
    };
}
