#include "gui/user_action/action_move_item.h"
#include "gui/user_action/user_action_object.h"
#include <QStringList>
#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/gui_globals.h"
#include "gui/graph_widget/layout_locker.h"
#include "gui/implementations/qpoint_extension.h"
#include "hal_core/utilities/log.h"
#include "gui/context_manager_widget/models/context_tree_model.h"

namespace hal
{
    ActionMoveItemFactory::ActionMoveItemFactory()
       : UserActionFactory("MoveNode") {;}

    ActionMoveItemFactory* ActionMoveItemFactory::sFactory = new ActionMoveItemFactory;

    UserAction* ActionMoveItemFactory::newAction() const
    {
        return new ActionMoveItem;
    }

    ActionMoveItem::ActionMoveItem(u32 tgtId, u32 srcId, int tgtRow)
        : mSourceParentId(srcId), mTargetParentId(tgtId), mTargetRow(tgtRow)
    {;}

    QString ActionMoveItem::tagname() const
    {
        return ActionMoveItemFactory::sFactory->tagname();
    }

    void ActionMoveItem::addToHash(QCryptographicHash& cryptoHash) const
    {
        cryptoHash.addData((char*)(&mTargetParentId), sizeof(u32));
    }

    void ActionMoveItem::writeToXml(QXmlStreamWriter& xmlOut) const
    {
        xmlOut.writeTextElement("target_parent_id", QString("%1").arg(mTargetParentId));
        if (mSourceParentId)
            xmlOut.writeTextElement("source_parent_id", QString("%1").arg(mSourceParentId));
    }

    void ActionMoveItem::readFromXml(QXmlStreamReader& xmlIn)
    {
        while (xmlIn.readNextStartElement())
        {
            if (xmlIn.name() == "target_parent_id")
                mTargetParentId = xmlIn.readElementText().toUInt();
            if (xmlIn.name() == "source_parent_id")
                mSourceParentId = xmlIn.readElementText().toUInt();
        }
    }

    bool ActionMoveItem::exec()
    {
        if (!mObject.id()) return false;

        BaseTreeItem* bti = nullptr;
        bool isDirectory = false;

        switch (mObject.type()) {
        case UserActionObjectType::ContextView:
            bti = gGraphContextManager->getContextTreeModel()->getContext(mObject.id());
            break;
        case UserActionObjectType::ContextDir:
            bti = gGraphContextManager->getContextTreeModel()->getDirectory(mObject.id());
            isDirectory = true;
            break;
        default:
            break;
        }
        if (!bti) return false;

        if (!mSourceParentId)
        {
            ContextTreeItem* parCti = dynamic_cast<ContextTreeItem*>(bti->getParent());
            if (parCti) mSourceParentId = parCti->getId();
        }

        int sourceRow = 0;
        QModelIndex sourceIndex = gGraphContextManager->getContextTreeModel()->getIndexFromItem(bti);
        if (sourceIndex.isValid())
            sourceRow = sourceIndex.row();
        mUndoAction = new ActionMoveItem(mSourceParentId, mTargetParentId, sourceRow);
        mUndoAction->setObject(mObject);

        gGraphContextManager->moveItem(mObject.id(), isDirectory, mTargetParentId, mTargetRow);

        return UserAction::exec();
    }
}
