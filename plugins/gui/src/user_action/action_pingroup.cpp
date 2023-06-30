#include "gui/user_action/action_pingroup.h"
#include "gui/gui_globals.h"
#include "hal_core/netlist/grouping.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/graph_widget/contexts/graph_context.h"
#include <QMetaEnum>

namespace hal
{
    QString PinAction::toString(PinAction::Type tp)
    {
        QMetaEnum me = QMetaEnum::fromType<Type>();
        return QString(me.key(tp));
    }

    PinAction::Type PinAction::fromString(const QString &s)
    {
        QMetaEnum me = QMetaEnum::fromType<Type>();
        for (int t = None; t < MaxAction; t++)
            if (s == me.key(t))
            {
                return static_cast<Type>(t);
            }
        return None;
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

    ActionPingroup::ActionPingroup(PinAction::Type action, u32 pingroupId, const QString &name)
      : mSourceGroupId(pingroupId), mTargetGroupId(pingroupId), mPinOrderNo(-1), mGroupOrderNo(-1), mName(name)
    {
        switch (action)
        {
        case PinAction::None:
            break;
        case PinAction::MovePin:
            // use move constructor
            break;
        default:
            mPinActions.append(action);
        }
    }

    ActionPingroup::ActionPingroup(u32 pinId, int pinIndex, u32 tgtgroupId, const QString& name, int grpIndex)
        :  mSourceGroupId(0), mTargetGroupId(tgtgroupId),  mPinOrderNo(pinIndex), mGroupOrderNo(grpIndex), mName(name)
    {
        mPinIds.append(pinId);
        mPinActions.append(PinAction::MovePin);
    }

    void ActionPingroup::addToHash(QCryptographicHash &cryptoHash) const
    {
        for (PinAction::Type tp : mPinActions)
            cryptoHash.addData((char*)(&tp),sizeof(tp));
        for (u32 pid: mPinIds)
            cryptoHash.addData((char*)(&pid),sizeof(pid));
        cryptoHash.addData((char*)(&mSourceGroupId),sizeof(mSourceGroupId));
        cryptoHash.addData((char*)(&mTargetGroupId),sizeof(mTargetGroupId));
        cryptoHash.addData((char*)(&mPinOrderNo),sizeof(mPinOrderNo));
        cryptoHash.addData((char*)(&mGroupOrderNo),sizeof(mGroupOrderNo));
    }

    void ActionPingroup::writeToXml(QXmlStreamWriter& xmlOut) const
    {
        //todo: remove parentId, switch entirely to parentObject
        if (!mPinActions.isEmpty())
        {
            QString s;
            for (PinAction::Type tp : mPinActions)
            {
                if (!s.isEmpty()) s += ',';
                s += PinAction::toString(tp);
            }
            xmlOut.writeTextElement("pinactions", s);
        }
        if (!mPinIds.isEmpty())
        {
            QString s;
            for (u32 pinId : mPinIds)
            {
                if (!s.isEmpty()) s += ',';
                s += QString::number(pinId);
            }
            xmlOut.writeTextElement("pinids", s);
        }
        if (mSourceGroupId)
            xmlOut.writeTextElement("srcgroupid", QString::number(mSourceGroupId));
        if (mTargetGroupId)
            xmlOut.writeTextElement("tgtgroupid", QString::number(mTargetGroupId));
        if (mPinOrderNo >= 0)
            xmlOut.writeTextElement("pinorder", QString::number(mPinOrderNo));
        if (mGroupOrderNo >= 0)
            xmlOut.writeTextElement("grporder", QString::number(mGroupOrderNo));
    }

    void ActionPingroup::readFromXml(QXmlStreamReader& xmlIn)
    {
        while (xmlIn.readNextStartElement())
        {
            //todo: emove parentId, switch entirely to parentObject
            readParentObjectFromXml(xmlIn);
            if (xmlIn.name() == "pinactions")
            {
                for (QString pinact : xmlIn.readElementText().split(','))
                    mPinActions.append(PinAction::fromString(pinact));
            }
            if (xmlIn.name() == "pinids")
            {
                for (QString pinid : xmlIn.readElementText().split(','))
                    mPinIds.append(pinid.toInt());
            }
            if (xmlIn.name() == "srcgroupid")
                mSourceGroupId = xmlIn.readElementText().toInt();
            if (xmlIn.name() == "tgtgroupid")
                mTargetGroupId = xmlIn.readElementText().toInt();
            if (xmlIn.name() == "pinorder")
                mPinOrderNo = xmlIn.readElementText().toInt();
            if (xmlIn.name() == "grporder")
                mGroupOrderNo = xmlIn.readElementText().toInt();
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

        if (mPinActions.size()==1 && mPinActions.at(0) == PinAction::MovePin)
        {
            auto* pinToMove = parentModule->get_pin_by_id(mPinIds.at(0));
            if (!pinToMove) return false;
            auto* srcgrp = pinToMove->get_group().first;
            if (!srcgrp) return false;
            int currentIndex = pinToMove->get_group().second;
            mSourceGroupId = srcgrp->get_id();
            undo = new ActionPingroup(pinToMove->get_id(), currentIndex, mSourceGroupId, QString::fromStdString(srcgrp->get_name()));
            if (srcgrp->get_pins().size() == 1) mPinActions.append(PinAction::Delete);
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
                if (!undo)
                    undo = new ActionPingroup(PinAction::Delete, mTargetGroupId);
                break;
            }
            case PinAction::Delete:
            {
                auto* pgrp = parentModule->get_pin_group_by_id(mSourceGroupId);
                QList<u32> pins;
                for (const auto& pin : pgrp->get_pins())
                {
                    pins.append(pin->get_id());
                }
                if (!undo)
                {
                    undo = new ActionPingroup(PinAction::Create, pgrp->get_id(), QString::fromStdString(pgrp->get_name()));
                    undo->setPinIds(pins);
                }
                break;
            }
            case PinAction::MovePin:
            {
                auto* tgtgrp = parentModule->get_pin_group_by_id(mTargetGroupId);
                auto* pinToMove = parentModule->get_pin_by_id(mPinIds.at(0));
                if (!tgtgrp || !pinToMove) return false;
                auto* srcgrp = pinToMove->get_group().first;

                if (srcgrp != tgtgrp)
                {
                    if (tgtgrp->assign_pin(pinToMove).is_ok()) return false;
                }
                tgtgrp->move_pin(pinToMove,mPinOrderNo).is_ok();
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
            case PinAction::Rename:
            {
                auto* pgrp = parentModule->get_pin_group_by_id(mTargetGroupId);
                if (!pgrp) return false;
                QString oldName = QString::fromStdString(pgrp->get_name());
                pgrp->set_name(mName.toStdString());
                if (undo)
                {
                    undo->addPinAction(PinAction::Rename);
                    undo->setName(oldName);
                }
                else
                    undo = new ActionPingroup(PinAction::Rename, pgrp->get_id(), oldName);
                break;
            }
            case PinAction::TypeChange:
                break;
            default:
                break;
            }
        }
        mUndoAction = undo;
        return UserAction::exec();
    }
}
