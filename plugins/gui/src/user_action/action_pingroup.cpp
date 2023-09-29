#include "gui/user_action/action_pingroup.h"
#include "gui/gui_globals.h"
#include "hal_core/netlist/grouping.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/graph_widget/contexts/graph_context.h"
#include <QMetaEnum>

namespace hal
{
    int pinGroupIndex(const Module* mod, const PinGroup<ModulePin>* pgrp)
    {
        if (!mod || !pgrp) return -1;
        int inx = 0;
        for (const PinGroup<ModulePin>* testgrp : mod->get_pin_groups())
        {
            if (testgrp == pgrp) return inx;
            ++inx;
        }
        return -1;
    }

    void dumpPingroups(Module *m)
    {
        if (!m) m = gNetlist->get_top_module();
        std::cerr << "module: " << m->get_id() << " <" << m->get_name() << ">\n";
        for (PinGroup<ModulePin>* pg : m->get_pin_groups())
        {
            std::cerr << "  grp: " << pg->get_id() << (pg->is_ascending()?" asc ": " des ") << pg->get_start_index()
                      << " <" << pg->get_name() << ">\n";
            for (ModulePin* pin : pg->get_pins())
                std::cerr << "     pin: " << pin->get_id() << " inx:" << pin->get_group().second << " pos:" << pin->get_pos() << " row:"
                          << pinIndex2Row(pin,pin->get_group().second) << " <" << pin->get_name() << ">\n";
        }
        std::cerr << "-------------" << std::endl;
        for (Module* sm : m->get_submodules())
            dumpPingroups(sm);
    }

    int pinIndex2Row(const ModulePin* pin, int index)
    {
        auto pg = pin->get_group();
        if (pg.first->is_ascending())
            return index - pg.first->get_start_index();
        return pg.first->get_start_index() - index;
    }

    int pinRow2Index(const ModulePin* pin, int row)
    {
        auto pg = pin->get_group();
        if (pg.first->is_ascending())
            return pg.first->get_start_index() + row;
        return pg.first->get_start_index() - row;
    }

    QString generateGroupName(const Module* mod, const ModulePin* pin)
    {
        QString baseName = QString::fromStdString(pin->get_name());
        QSet<QString> existingGroups;
        for (auto g : mod->get_pin_groups())
            existingGroups.insert(QString::fromStdString(g->get_name()));
        QString retval = baseName;
        int count = 1;
        while (existingGroups.contains(retval))
            retval = QString("%1_%2").arg(baseName).arg(++count);
        return retval;
    }

    QString PinActionType::toString(PinActionType::Type tp)
    {
        QMetaEnum me = QMetaEnum::fromType<Type>();
        return QString(me.key(tp));
    }

    PinActionType::Type PinActionType::fromString(const QString &s)
    {
        QMetaEnum me = QMetaEnum::fromType<Type>();
        for (int t = None; t < MaxAction; t++)
            if (s == me.key(t))
            {
                return static_cast<Type>(t);
            }
        return None;
    }

    bool PinActionType::useExistingGroup(PinActionType::Type tp)
    {
        static const QSet<Type> types = {GroupDelete, GroupMove, GroupRename, GroupTypechange, GroupDirection};
        return types.contains(tp);
    }

    bool PinActionType::useExistingPin(PinActionType::Type tp)
    {
        static const QSet<Type> types = {PinAsignGroup, PinRename, PinTypechange, PinDirection, PinSetindex};
        return types.contains(tp);
    }

    ActionPingroupFactory::ActionPingroupFactory()
        : UserActionFactory("Pingroup") {;}

    ActionPingroupFactory* ActionPingroupFactory::sFactory = new ActionPingroupFactory;

    UserAction* ActionPingroupFactory::newAction() const
    {
        return new ActionPingroup;
    }

    QString ActionPingroup::tagname() const
    {
        return ActionPingroupFactory::sFactory->tagname();
    }

    ActionPingroup::ActionPingroup(PinActionType::Type tp, int id, const QString &name, int value)
    {
        mPinActions.append(AtomicAction(tp, id, name, value));
    }

    ActionPingroup::ActionPingroup(const QList<AtomicAction>& aaList)
        : mPinActions(aaList)
    {;}

    void ActionPingroup::addToHash(QCryptographicHash &cryptoHash) const
    {
        for (const AtomicAction& aa : mPinActions)
        {
            cryptoHash.addData((char*)(&aa.mType),sizeof(aa.mType));
            cryptoHash.addData((char*)(&aa.mId),sizeof(aa.mId));
            cryptoHash.addData(aa.mName.toUtf8());
            cryptoHash.addData((char*)(&aa.mValue),sizeof(aa.mValue));
        }
    }

    void ActionPingroup::writeToXml(QXmlStreamWriter& xmlOut) const
    {
        // TODO xml parent element
        for (const AtomicAction& aa : mPinActions)
        {
            if (aa.mType != PinActionType::None)
                xmlOut.writeTextElement("type", PinActionType::toString(aa.mType));
            if (aa.mId)
                xmlOut.writeTextElement("id", QString::number(aa.mId));
            if (!aa.mName.isEmpty())
                xmlOut.writeTextElement("name", aa.mName);
            if (aa.mValue)
                xmlOut.writeTextElement("value", QString::number(aa.mValue));
        }
    }

    void ActionPingroup::readFromXml(QXmlStreamReader& xmlIn)
    {
        // TODO loop xml parent element
        while (xmlIn.readNextStartElement())
        {
            PinActionType::Type tp = PinActionType::None;
            int id = 0;
            QString name;
            int val = 0;

            if (xmlIn.name() == "type")
                tp = PinActionType::fromString(xmlIn.readElementText());
            if (xmlIn.name() == "id")
                id = xmlIn.readElementText().toInt();
            if (xmlIn.name() == "name")
                name = xmlIn.readElementText();
            if (xmlIn.name() == "value")
                val = xmlIn.readElementText().toInt();
            mPinActions.append(AtomicAction(tp,id,name,val));
        }
    }
    PinGroup<ModulePin>* ActionPingroup::getGroup(ModulePin* pin) const
    {
        return pin->get_group().first;
    }

    PinGroup<ModulePin>* ActionPingroup::getGroup(int grpId) const
    {
        PinGroup<ModulePin>* pgroup = mPinGroups.value(grpId);
        if (pgroup) return pgroup;
        if (!mParentModule || grpId<=0) return nullptr;
        return mParentModule->get_pin_group_by_id(grpId);
    }

    void ActionPingroup::addUndoAction(PinActionType::Type tp, int id, const QString& name, int value)
    {
        ActionPingroup* undo = nullptr;
        if (mUndoAction)
        {
            undo = static_cast<ActionPingroup*>(mUndoAction);
            undo->mPinActions.append(AtomicAction(tp,id,name,value));
        }
        else
        {
            undo = new ActionPingroup(tp,id,name,value);
            undo->setObject(object());
        }
        mUndoAction = undo;
    }


    void ActionPingroup::prepareUndoAction()
    {
        QHash<PinGroup<ModulePin>*,int> remainingPins;
        for (const AtomicAction& aa : mPinActions)
        {
            if (aa.mType != PinActionType::PinAsignGroup)
                continue;
            ModulePin* pin = mParentModule->get_pin_by_id(aa.mId);
            PinGroup<ModulePin>* pgroup = pin->get_group().first;
            if (remainingPins.contains(pgroup))
                remainingPins[pgroup]--;
            else
                remainingPins[pgroup] = pgroup->size()-1;
        }

        // for groups that will be deleted after pin assign a create action is needed in undo
        for (auto it = remainingPins.begin(); it != remainingPins.end(); ++it)
        {
            if (it.value() > 0) continue;
            GroupRestore gr(mParentModule,it.key());
            mGroupRestore.insert(gr.mRow,gr);
        }
    }

    void ActionPingroup::finalizeUndoAction()
    {
        QList<AtomicAction> restoreActions;
        for (auto it = mGroupRestore.begin(); it != mGroupRestore.end(); ++it)
        {
            const GroupRestore& gr = it.value();
            restoreActions.append(AtomicAction(PinActionType::GroupCreate,gr.mId,gr.mName,gr.mStartIndex));
            restoreActions.append(AtomicAction(PinActionType::GroupMove,gr.mId,"",gr.mRow));
            if (gr.mType != PinType::none)
                restoreActions.append(AtomicAction(PinActionType::GroupTypechange,gr.mId,"",(int)gr.mType));
            if (gr.mDirection != PinDirection::none)
                restoreActions.append(AtomicAction(PinActionType::GroupDirection,gr.mId,"",(int)gr.mDirection));
        }
        if (!restoreActions.isEmpty())
        {
            if (mUndoAction)
            {
                ActionPingroup* act = static_cast<ActionPingroup*>(mUndoAction);
                restoreActions += act->mPinActions;
                act->mPinActions = restoreActions;
            }
            else
            {
                mUndoAction = new ActionPingroup(restoreActions);
            }
        }

        for (u32 grpId : mGroupToRemove)
        {
            if (mUndoAction)
            {
                ActionPingroup* act = static_cast<ActionPingroup*>(mUndoAction);
                act->mPinActions.append(AtomicAction(PinActionType::GroupDelete,grpId));
            }
            else
                mUndoAction = new ActionPingroup(PinActionType::GroupDelete,grpId);
        }

        if (mUndoAction) mUndoAction->setObject(object());
    }

    int ActionPingroup::pinGroupRow(Module *m, PinGroup<ModulePin>* pgroup)
    {
        int inx = 0;
        for (PinGroup<ModulePin>* testgroup : m->get_pin_groups())
        {
            if (testgroup == pgroup) return inx;
            ++inx;
        }
        return -1;
    }

    bool ActionPingroup::exec()
    {
        mPinGroups.clear();
        mGroupRestore.clear();
        mPinsMoved.clear();
        mGroupToRemove.clear();
        if (mObject.type() != UserActionObjectType::Module)
            return false;

        mParentModule = gNetlist->get_module_by_id(mObject.id());
        if (!mParentModule)
            return false;

        prepareUndoAction(); // create pingroups in case we are going to delete some while assigning

        for (const AtomicAction& aa : mPinActions)
        {
            PinGroup<ModulePin>* pgroup = nullptr;
            ModulePin* pin              = nullptr;

            if (PinActionType::useExistingGroup(aa.mType))
            {
                auto it = mPinGroups.find(aa.mId);
                if (it == mPinGroups.end())
                {
                    pgroup = mParentModule->get_pin_group_by_id(aa.mId);
                    if (!pgroup) return false;
                    mPinGroups.insert(aa.mId,pgroup);
                }
                else
                    pgroup = it.value();
            }

            if (PinActionType::useExistingPin(aa.mType))
            {
                pin = mParentModule->get_pin_by_id(aa.mId);
            }

            switch (aa.mType)
            {
            case PinActionType::GroupCreate:
            {
                int startIndex = aa.mValue;
                bool ascending = true;
                if (aa.mValue < 0)
                {
                    ascending = false;
                    startIndex = -aa.mValue;
                }
                if (aa.mId > 0)
                {
                    if (auto res = mParentModule->create_pin_group(aa.mId, aa.mName.toStdString(), {}, PinDirection::none, PinType::none,ascending,startIndex); res.is_ok())
                        pgroup = res.get();
                    else
                        return false;
                }
                else
                {
                    if (auto res = mParentModule->create_pin_group(aa.mName.toStdString(), {}, PinDirection::none, PinType::none,ascending,startIndex); res.is_ok())
                        pgroup = res.get();
                    else
                        return false;
                }
                if (pgroup)
                {
                    mPinGroups[aa.mId] = pgroup;
                    mGroupToRemove.insert(pgroup->get_id());
                }
                break;
            }
            case PinActionType::GroupDelete:
            {
                int v = pgroup->get_start_index();
                if (!pgroup->is_ascending()) v = -v;
                u32 id = pgroup->get_id();
                int ptype = (int) pgroup->get_type();
                int pdir  = (int) pgroup->get_direction();
                QString name = QString::fromStdString(pgroup->get_name());
                if (!mParentModule->delete_pin_group(pgroup))
                    return false;
                addUndoAction(PinActionType::GroupCreate,id,name,v);
                addUndoAction(PinActionType::GroupTypechange,id,"",ptype);
                addUndoAction(PinActionType::GroupDirection,id,"",pdir);
                break;
            }
            case PinActionType::GroupMove:
            {
                int inx = pinGroupRow(mParentModule,pgroup);
                if (inx < 0) return false;
                addUndoAction(PinActionType::GroupMove,pgroup->get_id(),"",inx);
                if (!mParentModule->move_pin_group(pgroup,aa.mValue))
                    return false;
                break;
            }
            case PinActionType::GroupRename:
                addUndoAction(PinActionType::GroupRename,pgroup->get_id(),QString::fromStdString(pgroup->get_name()));
                if (!mParentModule->set_pin_group_name(pgroup,aa.mName.toStdString()))
                    return false;
                break;
            case PinActionType::GroupTypechange:
                addUndoAction(PinActionType::GroupTypechange,pgroup->get_id(),"",(int)pgroup->get_type());
                if (!mParentModule->set_pin_group_type(pgroup, (PinType) aa.mValue))
                    return false;
                break;
            case PinActionType::GroupDirection:
                addUndoAction(PinActionType::GroupDirection,pgroup->get_id(),"",(int)pgroup->get_direction());
                if (!mParentModule->set_pin_group_direction(pgroup, (PinDirection) aa.mValue))
                    return false;
                break;
            case PinActionType::PinAsignGroup:
                addUndoAction(PinActionType::PinAsignGroup,aa.mId,"",pin->get_group().first->get_id());
                addUndoAction(PinActionType::PinSetindex,aa.mId,"",pinIndex2Row(pin,pin->get_group().second));
                mPinsMoved.insert(aa.mId);
                pgroup = getGroup(aa.mValue);
                if (!pgroup) return false;
                if (!mParentModule->assign_pin_to_group(pgroup,pin))
                {
                    qDebug() << "assign_pin_to_group failed";
                    return false;
                }
                dumpPingroups();
                break;
            case PinActionType::PinRename:
                addUndoAction(PinActionType::PinRename,aa.mId, QString::fromStdString(pin->get_name()));
                if (!mParentModule->set_pin_name(pin, aa.mName.toStdString()))
                    return false;
                break;
            case PinActionType::PinTypechange:
                addUndoAction(PinActionType::PinTypechange,aa.mId,"",(int)pin->get_type());
                if (!mParentModule->set_pin_type(pin, (PinType) aa.mValue))
                    return false;
                break;
            case PinActionType::PinSetindex:
                if (!mPinsMoved.contains(aa.mId))
                    addUndoAction(PinActionType::PinSetindex,aa.mId,"",pinIndex2Row(pin,pin->get_group().second));
                pgroup = pin->get_group().first;
                if (!mParentModule->move_pin_within_group(pgroup,pin,pinRow2Index(pin,aa.mValue)))
                {
                    qDebug() << "move_pin_within_group failed";
                    return false;
                }
                dumpPingroups();
                break;
            default:
                break;
            }
        }

        finalizeUndoAction();

        return UserAction::exec();
    }

    ActionPingroup* ActionPingroup::addPinsToExistingGroup(const Module *m, u32 grpId, QList<u32> pinIds, int pinRow)
    {
        ActionPingroup* retval = nullptr;
        for (u32 pinId : pinIds)
        {
            if (retval)
                retval->mPinActions.append(AtomicAction(PinActionType::PinAsignGroup,pinId,"",grpId));
            else
                retval = new ActionPingroup(PinActionType::PinAsignGroup,pinId,"",grpId);
            if (pinRow >= 0)
                retval->mPinActions.append(AtomicAction(PinActionType::PinSetindex,pinId,"",pinRow++));
        }
        retval->setObject(UserActionObject(m->get_id(),UserActionObjectType::Module));
        return retval;
    }

    ActionPingroup* ActionPingroup::addPinToExistingGroup(const Module* m, u32 grpId, u32 pinId, int pinRow)
    {
        QList<u32> pinIds;
        pinIds << pinId;
        return addPinsToExistingGroup(m,grpId,pinIds,pinRow);
    }

    ActionPingroup* ActionPingroup::addPinsToNewGroup(const Module* m, const QString& name, QList<u32> pinIds, int grpRow)
    {
        static int vid = -9;
        ActionPingroup* retval = new ActionPingroup(PinActionType::GroupCreate,vid,name);
        if (!pinIds.isEmpty())
        {
            ModulePin* pin = m->get_pin_by_id(pinIds.first());
            if (pin)
            {
                retval->mPinActions.append(AtomicAction(PinActionType::GroupDirection,vid,"",(int)pin->get_direction()));
                retval->mPinActions.append(AtomicAction(PinActionType::GroupTypechange,vid,"",(int)pin->get_type()));
            }
        }
        for (u32 pinId : pinIds)
            retval->mPinActions.append(AtomicAction(PinActionType::PinAsignGroup,pinId,"",vid));

        if (grpRow >= 0)
            retval->mPinActions.append(AtomicAction(PinActionType::GroupMove,vid,"",grpRow));
        retval->setObject(UserActionObject(m->get_id(),UserActionObjectType::Module));
        return retval;
    }

    ActionPingroup* ActionPingroup::addPinToNewGroup(const Module *m, const QString& name, u32 pinId, int grpRow)
    {
        QList<u32> pinIds;
        pinIds << pinId;
        return addPinsToNewGroup(m,name,pinIds, grpRow);
    }

    ActionPingroup* ActionPingroup::removePinsFromGroup(const Module* m, QList<u32> pinIds)
    {
        ActionPingroup* retval = nullptr;
        QSet<QString> existingGroupNames;
        for (PinGroup<ModulePin>* pgroup : m->get_pin_groups())
            existingGroupNames.insert(QString::fromStdString(pgroup->get_name()));
        int vid = -1;
        QString basename;
        for (u32 pinId : pinIds)
        {
            ModulePin* pin = m->get_pin_by_id(pinId);
            if (!pin) return nullptr;
            int count = 2;
            QString name = basename = QString::fromStdString(pin->get_name());
            while (existingGroupNames.contains(name))
                name = QString("%1_%2").arg(basename).arg(count++);
            if (retval)
                retval->mPinActions.append(AtomicAction(PinActionType::GroupCreate,vid,name));
            else
                retval = new ActionPingroup(PinActionType::GroupCreate,vid,name);
            retval->mPinActions.append(AtomicAction(PinActionType::PinAsignGroup,pinId,"",vid));
            --vid;
        }
        retval->setObject(UserActionObject(m->get_id(),UserActionObjectType::Module));
        return retval;
    }

    ActionPingroup::GroupRestore::GroupRestore(Module* m, PinGroup<ModulePin>* pgroup)
        : mId(pgroup->get_id()),
          mName(QString::fromStdString(pgroup->get_name())),
          mRow(pinGroupRow(m,pgroup)),
          mStartIndex(pgroup->get_start_index()),
          mDirection(pgroup->get_direction()),
          mType(pgroup->get_type())
    {
        if (!pgroup->is_ascending()) mStartIndex = -mStartIndex;
    }
}
