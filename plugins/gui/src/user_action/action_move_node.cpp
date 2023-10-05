#include "gui/user_action/action_move_node.h"
#include <QStringList>
#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/gui_globals.h"
#include "gui/implementations/qpoint_extension.h"
#include "hal_core/utilities/log.h"

namespace hal
{
    ActionMoveNodeFactory::ActionMoveNodeFactory()
       : UserActionFactory("MoveNode") {;}

    ActionMoveNodeFactory* ActionMoveNodeFactory::sFactory = new ActionMoveNodeFactory;

    UserAction* ActionMoveNodeFactory::newAction() const
    {
        return new ActionMoveNode;
    }

    bool ActionMoveNode::checkContextId()
    {
        if (!mContextId)
        {
            log_warning("gui", "ActionMoveNode invoked without context ID.");
            return false;
        }
        GraphContext* ctx = gGraphContextManager->getContextById(mContextId);
        if (!ctx)
        {
            log_warning("gui", "ActionMoveNode invoked with illegal context ID {}.", mContextId);
            mContextId = 0;
            return false;
        }
        return true;
    }

    ActionMoveNode::ActionMoveNode(u32 ctxId, const GridPlacement* gridPlc)
        : mContextId(ctxId), mSwap(false)
    {
        if (!checkContextId()) return;
        if (gridPlc)
            mGridPlacement = *gridPlc;
    }

    ActionMoveNode::ActionMoveNode(u32 ctxID, const QPoint& to)
        : mContextId(ctxID), mTo(to), mSwap(false)
    {
        if (!checkContextId()) return;
    }

    ActionMoveNode::ActionMoveNode(u32 ctxID, const QPoint& from, const QPoint& to, bool swap)
        : mContextId(ctxID), mTo(to), mSwap(swap)
    {
        if (!checkContextId()) return;
        GraphContext* ctx = gGraphContextManager->getContextById(mContextId);
        auto it = ctx->getLayouter()->positionToNodeMap().find(from);
        if (it == ctx->getLayouter()->positionToNodeMap().constEnd())
        {
            mContextId = 0; // node not found, exit
            return;
        }
        Node ndToMove = it.value(); // get the node we want to move

        if(mSwap)
        {

        }

        switch (ndToMove.type())
        {
        case Node::Module:
            mObject = UserActionObject(ndToMove.id(),UserActionObjectType::Module);
            break;
        case Node::Gate:
            mObject = UserActionObject(ndToMove.id(),UserActionObjectType::Gate);
            break;
        default:
            mContextId = 0; // node type None, exit
            return;
        }
    }

    QString ActionMoveNode::tagname() const
    {
        return ActionMoveNodeFactory::sFactory->tagname();
    }

    void ActionMoveNode::addToHash(QCryptographicHash& cryptoHash) const
    {
        cryptoHash.addData((char*)(&mTo)  , sizeof(QPoint));
    }

    void ActionMoveNode::writeToXml(QXmlStreamWriter& xmlOut) const
    {
        xmlOut.writeTextElement("to", QString("%1,%2").arg(mTo.x()).arg(mTo.y()));
    }

    void ActionMoveNode::readFromXml(QXmlStreamReader& xmlIn)
    {
        while (xmlIn.readNextStartElement())
        {
            if (xmlIn.name() == "to")
                mTo = parseFromString(xmlIn.readElementText());
        }
    }

    QPoint ActionMoveNode::parseFromString(const QString& s)
    {
        QStringList sl = s.split(',');
        Q_ASSERT(sl.size()==2);
        return QPoint(sl.at(0).toInt(),sl.at(1).toInt());
    }

    bool ActionMoveNode::exec()
    {
        if (!mContextId) return false;
        GraphContext* ctx = gGraphContextManager->getContextById(mContextId);
        if (!ctx) return false;

        // current placement for undo
        ActionMoveNode* undo = new ActionMoveNode(mContextId, GuiApiClasses::View::getGridPlacement(mContextId));
        mUndoAction = undo;

        // test whether there is a user object
        Node ndToMove;
        switch (mObject.type()) {
        case UserActionObjectType::Gate:
            ndToMove = Node(mObject.id(),Node::Gate);
            break;
        case UserActionObjectType::Module:
            ndToMove = Node(mObject.id(),Node::Module);
            break;
        default:
            break;
        }
        if (ndToMove.type() != Node::None)
        {
            mGridPlacement = undo->mGridPlacement;
            auto it = ctx->getLayouter()->positionToNodeMap().find(mTo);
            if (it != ctx->getLayouter()->positionToNodeMap().constEnd())
                mGridPlacement[ndToMove] = mTo;
        }

        ctx->clear();

        QSet<u32> modIds;
        QSet<u32> gateIds;

        for (Node node : mGridPlacement.keys())
        {
            if(node.isModule()) modIds.insert(node.id());
            else gateIds.insert(node.id());
        }

        ctx->add(modIds, gateIds, PlacementHint(mGridPlacement));
        ctx->scheduleSceneUpdate();

        return UserAction::exec();
    }
}
