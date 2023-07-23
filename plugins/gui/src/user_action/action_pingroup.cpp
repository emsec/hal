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
        static const QSet<Type> types = {PinAddgroup, PinRename, PinTypechange, PinDirection, PinSetindex};
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

    ActionPingroup::ActionPingroup(PinActionType::Type tp, u32 id, const QString &name, int value)
    {
        mPinActions.append(AtomicAction(tp, id, name, value));
    }

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

    bool ActionPingroup::exec()
    {
        if (mObject.type() != UserActionObjectType::Module)
            return false;

        Module* parentModule = gNetlist->get_module_by_id(mObject.id());
        if (!parentModule)
            return false;

        ActionPingroup* undo = nullptr;

        QHash<int,PinGroup<ModulePin>*> pgroups;

        for (const AtomicAction& aa : mPinActions)
        {
            PinGroup<ModulePin>* pgroup = nullptr;
            ModulePin* pin              = nullptr;

            if (PinActionType::useExistingGroup(aa.mType))
            {
                auto it = pgroups.find(aa.mId);
                if (it == pgroups.end())
                {
                    pgroup = parentModule->get_pin_group_by_id(aa.mId);
                    if (!pgroup) return false;
                    pgroups.insert(aa.mId,pgroup);
                }
                else
                    pgroup = it.value();
            }

            if (PinActionType::useExistingPin(aa.mType))
            {
                pin = parentModule->get_pin_by_id(aa.mId);
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
                    if (auto res = parentModule->create_pin_group(aa.mId, aa.mName.toStdString(), {}, PinDirection::none, PinType::none,ascending,startIndex); res.is_ok())
                        pgroups.insert(aa.mId,res.get());
                    else
                        return false;
                }
                else
                {
                    if (auto res = parentModule->create_pin_group(aa.mName.toStdString(), {}, PinDirection::none, PinType::none,ascending,startIndex); res.is_ok())
                        pgroups.insert(aa.mId,res.get());
                    else
                        return false;
                }
                break;
            }
            case PinActionType::GroupDelete:
                if (parentModule->delete_pin_group(pgroup).is_error())
                    return false;
                break;
            case PinActionType::GroupMove:
                if (parentModule->move_pin_group(pgroup,aa.mValue).is_error())
                    return false;
                break;
            case PinActionType::GroupRename:
                if (!parentModule->set_pin_group_name(pgroup,aa.mName.toStdString()))
                    return false;
                break;
            case PinActionType::GroupTypechange:
                if (!parentModule->set_pin_group_type(pgroup, (PinType) aa.mValue))
                    return false;
                break;
            case PinActionType::GroupDirection:
                if (!parentModule->set_pin_group_direction(pgroup, (PinDirection) aa.mValue))
                    return false;
                break;
            case PinActionType::PinAddgroup:
                break;
            case PinActionType::PinRename:
                if (!parentModule->set_pin_name(pin, aa.mName.toStdString()))
                    return false;
                break;
            case PinActionType::PinTypechange:
                if (!parentModule->set_pin_type(pin, (PinType) aa.mValue))
                    return false;
                break;
            case PinActionType::PinSetindex:
                break;
            default:
                break;
            }
        }

        /*
        if (mPinActions.size()==1 &&
                (mPinActions.at(0) == PinAction::MovePin || mPinActions.at(0) == PinAction::RemovePin))
        {
            auto* pinToMove = parentModule->get_pin_by_id(mPinIds.at(0));
            if (!pinToMove) return false;
            auto* srcgrp = pinToMove->get_group().first;
            if (!srcgrp) return false;
            int currentIndex = pinToMove->get_group().second;
            mSourceGroupId = srcgrp->get_id();
            undo = new ActionPingroup(pinToMove->get_id(), currentIndex, mSourceGroupId, QString::fromStdString(srcgrp->get_name()));
            if (srcgrp->get_pins().size() == 1) mPinActions.append(PinAction::DeleteGroup);
            if (mPinActions.at(0) == PinAction::MovePin)
            {
                if (mTargetGroupId)
                {
                    auto* tgtgrp = parentModule->get_pin_group_by_id(mTargetGroupId);
                    if (!tgtgrp)
                        tgtgrp = parentModule->get_pin_group_by_name(mName.toStdString());
                    if (!tgtgrp)
                        mTargetGroupId = 0;
                }
                else
                {
                    auto* tgtgrp = parentModule->get_pin_group_by_name(mName.toStdString());
                    if (tgtgrp)
                        mTargetGroupId = tgtgrp->get_id();
                }
                if (!mTargetGroupId)
                {
                    // target group does not exist, create it
                    if (mName.isEmpty())
                        mName = QString::fromStdString(pinToMove->get_name());
                    mPinActions.replace(0,PinAction::Create); // create will place pins in new group
                }
            }
        }


        for (PinAction::Type act : mPinActions)
        {
            switch (act)
            {
            case PinAction::None:
                break;
            case PinAction::Create:
            {
                auto res = parentModule->create_pin_group(mName.toStdString());
                if(res.is_error())
                    return false;
                mTargetGroupId = res.get()->get_id();
                if (!mPinIds.isEmpty())
                {
                    auto* pgrp = parentModule->get_pin_group_by_id(mTargetGroupId);
                    for (u32 pinId : mPinIds)
                    {
                        auto pin = parentModule->get_pin_by_id(pinId);
                        if (pin) pgrp->assign_pin(pin).is_ok();
                    }
                }
                if (mGroupOrderNo >= 0)
                    parentModule->move_pin_group(res.get(),mGroupOrderNo).is_ok();
                if (!undo)
                    undo = new ActionPingroup(PinAction::DeleteGroup, mTargetGroupId);
                break;
            }
            case PinAction::DeleteGroup:
            {
                auto* pgrp = parentModule->get_pin_group_by_id(mSourceGroupId);
                if (!pgrp) return false;
                int currentIndex = pinGroupIndex(parentModule,pgrp);
                QList<u32> pins;
                for (const auto& pin : pgrp->get_pins())
                {
                    pins.append(pin->get_id());
                }
                if (parentModule->delete_pin_group(pgrp).is_error()) return false;
                if (!undo)
                {
                    undo = new ActionPingroup(PinAction::Create, pgrp->get_id(), QString::fromStdString(pgrp->get_name()));
                    undo->setPinIds(pins);
                    undo->setGroupOrderNo(currentIndex);
                }
                break;
            }
            case PinAction::MovePin:
            {
                PinGroup<ModulePin>* tgtgrp = parentModule->get_pin_group_by_id(mTargetGroupId);
                ModulePin* pinToMove = parentModule->get_pin_by_id(mPinIds.at(0));
                if (!tgtgrp || !pinToMove) return false;
                auto* srcgrp = pinToMove->get_group().first;

                if (srcgrp != tgtgrp)
                {
                    if (tgtgrp->assign_pin(pinToMove).is_ok()) return false;
                }
                tgtgrp->move_pin(pinToMove,mPinOrderNo).is_ok();
                break;
            }
            case PinAction::RemovePin:
            {
                PinGroup<ModulePin>* srcgrp = parentModule->get_pin_group_by_id(mSourceGroupId);
                ModulePin* pinToRemove = parentModule->get_pin_by_id(mPinIds.at(0));
                if (!srcgrp || !pinToRemove) return false;
                srcgrp->remove_pin(pinToRemove).is_ok();
                break;
            }
            case PinAction::MoveGroup:
            {
                PinGroup<ModulePin>* srcgrp = nullptr;
                int currentIndex = 0;
                for (PinGroup<ModulePin>* pgrp : parentModule->get_pin_groups())
                {
                    if (pgrp->get_id() == mSourceGroupId)
                    {
                        srcgrp = pgrp;
                        break;
                    }
                    ++ currentIndex;
                }
                if (!srcgrp) return false;
                if (parentModule->move_pin_group(srcgrp,mPinOrderNo).is_error()) return  false;
                if (!undo)
                {
                    undo = new ActionPingroup(PinAction::MoveGroup, mSourceGroupId);
                    undo->setPinOrderNo(currentIndex);
                }
            }
            case PinAction::RenamePin:
            {
                if (mPinIds.isEmpty()) return false;
                auto* pin = parentModule->get_pin_by_id(mPinIds.first());
                if (!pin) return false;
                QString oldName = QString::fromStdString(pin->get_name());
                if (!parentModule->set_pin_name(pin,mName.toStdString())) return false; // RenamePin
                if (!undo)
                {
                    undo = new ActionPingroup(PinAction::RenamePin, pin->get_id(), oldName);
                }
            }
            case PinAction::RenameGroup:
            {
                auto* pgrp = parentModule->get_pin_group_by_id(mTargetGroupId);
                if (!pgrp) return false;
                QString oldName = QString::fromStdString(pgrp->get_name());
                if (!parentModule->set_pin_group_name(pgrp,mName.toStdString())) return false; // RenameGroup
                if (!undo)
                {
                    undo = new ActionPingroup(PinAction::RenameGroup, pgrp->get_id(), oldName);
                }
                break;
            }
            case PinAction::TypeChange:
            {
                if (mPinIds.isEmpty()) return false;
                auto* pin = parentModule->get_pin_by_id(mPinIds.first());
                if (!pin) return false;
                PinType ptype = enum_from_string<PinType>(mName.toStdString(),PinType::none);
                PinType oldPtype = pin->get_type();
                if (!parentModule->set_pin_type(pin,ptype)) return false; // TypeChange
                if (!undo)
                {
                    undo = new ActionPingroup(PinAction::TypeChange, 0, QString::fromStdString(enum_to_string<PinType>(oldPtype)));
                    undo->setPinId(pin->get_id());
                }
                break;
            }
            default:
                break;
            }
        }
        */
        if (undo)
            undo->setObject(object());
        mUndoAction = undo;
        return UserAction::exec();
    }
}
