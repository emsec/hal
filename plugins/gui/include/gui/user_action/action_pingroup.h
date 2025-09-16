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
    class PinActionType : public QObject
    {
        Q_OBJECT
    public:
        enum Type { None, GroupCreate, GroupDelete, GroupMoveToRow, GroupRename, GroupTypeChange, GroupDirChange,
                    PinAsignToGroup, PinRename, PinTypeChange, PinDirChange, PinMoveToRow, MaxAction };
        Q_ENUM(Type);

    public:
        static QString toString(Type tp);
        static Type fromString(const QString& s);
        static bool useExistingGroup(Type tp);
        static bool useExistingPin(Type tp);
    };

    void dumpPingroups(Module* m = nullptr);
    /**
     * @ingroup user_action
     * @brief Pingroup user actions
     *
     * Arguments depends on PinActionType::Type:
     *
     * GroupCreate:
     *     ID       : ID of group to create
     *                   negative ID: call constructor without ID, however,
     *                   ID will be used internally for subsequent commands related to crated group
     *     name     : name of group
     *     value    : start index, assume ascending
     *                   negative value: descending order starting with (-value-1)
     *
     * GroupDelete
     *     ID       : ID of group to delete
     *
     * GroupMoveToRow
     *     ID       : ID of group to move
     *     value    : row to which group get moved within vector of pin groups
     *
     * GroupRename
     *     ID       : ID of group to rename
     *     name     : new name
     *
     * GroupTypeChange
     *     ID       : ID of group to modifiy
     *     value    : (int) PinType   as of hal_core/netlist/gate_library/enums/pin_type.h
     *
     * GroupDirChange
     *     ID       : ID of group to modifiy
     *     value    : (int) PinDirection    as of hal_core/netlist/gate_library/enums/pin_direction.h
     *
     * PinAsignToGroup
     *     ID       : ID of pin
     *     value    : ID of group, might be negative if group recently created
     *
     * PinRename
     *     ID       : ID of pin to rename
     *     name     : new name
     *
     * PinTypeChange
     *     ID       : ID of pin to modify
     *     value    : (int) PinType
     *
     * PinDirChange
     *     ID       : ID of pin to modify
     *     value    : (int) PinDirection
     *
     * PinMoveToRow
     *     ID       : ID of pin
     *     value    : row to which pin gets moved in pingroup
     */
    class ActionPingroup : public UserAction
    {
    private:
        class AtomicAction
        {
        public:
            PinActionType::Type mType;
            int mId;
            QString mName;
            int mValue;
            AtomicAction(PinActionType::Type tp, int id, const QString& name = QString(), int v=0) : mType(tp), mId(id), mName(name), mValue(v) {;}
        };

        class GroupRestore
        {
        public:
            int mId;
            QString mName;
            int mRow;
            int mStartIndex;
            PinDirection mDirection;
            PinType mType;
            GroupRestore(Module* m, PinGroup<ModulePin>* pgroup);
        };

        QHash<int,PinGroup<ModulePin>*> mPinGroups;
        QList<AtomicAction> mPinActions;
        Module* mParentModule;
        QMap<int,GroupRestore> mGroupRestore;
        QSet<u32> mPinsMoved;
        QSet<int> mGroupToRemove;

        PinGroup<ModulePin>* getGroup(ModulePin* pin) const;
        PinGroup<ModulePin>* getGroup(int grpId) const;
        void prepareUndoAction();
        void finalizeUndoAction();
        void addUndoAction(PinActionType::Type tp, int id = 0, const QString& name=QString(), int value=0);
        static int pinGroupRow(const Module *m, PinGroup<ModulePin>* pgroup);
    public:
        ActionPingroup(PinActionType::Type tp = PinActionType::None, int id = 0, const QString& name=QString(), int value=0);
        ActionPingroup(const QList<AtomicAction>& aaList);
        bool exec() override;
        QString tagname() const override;
        void writeToXml(QXmlStreamWriter& xmlOut) const override;
        void readFromXml(QXmlStreamReader& xmlIn) override;
        void addToHash(QCryptographicHash& cryptoHash) const override;

        static ActionPingroup* addPinsToExistingGroup(const Module* m, u32 grpId, QList<u32> pinIds, int pinRow = -1);
        static ActionPingroup* addPinToExistingGroup(const Module* m, u32 grpId, u32 pinId, int pinRow = -1);
        static ActionPingroup* addPinsToNewGroup(const Module* m, const QString& name, QList<u32> pinIds, int grpRow = -1);
        static ActionPingroup* addPinToNewGroup(const Module* m, const QString& name, u32 pinId, int grpRow = -1);
        static ActionPingroup* removePinsFromGroup(const Module* m, QList<u32> pinIds);
        static ActionPingroup* deletePinGroup(const Module* m, u32 grpId);
        static ActionPingroup* toggleAscendingGroup(const Module* m, u32 grpId);
        static ActionPingroup* changePinGroupType(const Module* m, u32 grpId, int ptype);

        static int pinGroupIndex(const Module* mod, const PinGroup<ModulePin>* pgrp);
        static int pinIndex2Row(const ModulePin* pin, int index);
        static int pinRow2Index(const ModulePin* pin, int row);
        static QString generateGroupName(const Module* mod, const ModulePin* pin);
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

    uint qHash(PinEvent pev);
}
