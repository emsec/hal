#include "gui/user_action/action_move_node.h"
#include <QStringList>
#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/gui_globals.h"
#include "gui/graph_widget/layout_locker.h"
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

        // This special constructor is mainly used in compound statement.
        // At construction target position might still be occupied, so don't do any checks before exec()
    }

    /*
    ActionMoveNode::ActionMoveNode(u32 ctxID, const QPoint& from, const QPoint& to, bool swap)
        : mContextId(ctxID), mTo(to), mSwap(swap)
    {
        if (!checkContextId()) return;
        GraphContext* ctx = gGraphContextManager->getContextById(mContextId);

        // get the node we want to move
        Node ndToMove = ctx->getLayouter()->positionToNodeMap().value(from);
        if (ndToMove.isNull())
        {
            mContextId = 0; // node not found, exit
            return;
        }

        mTargetNode = ctx->getLayouter()->positionToNodeMap().value(to);
        if(!mTargetNode.isNull() && !mSwap)
        {
            mContextId = 0; // move to an occupied position without swap modifier
            return;
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
*/

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
            if (xmlIn.name() == QString("to"))
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

        if (mGridPlacement.isEmpty())
        {
            // No placement given by constructor
            // load current placement and modify from and to position
            mGridPlacement = undo->mGridPlacement;

            // test whether there is a user object to move
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
            if (ndToMove.type() == Node::None) return false;

            if (!mTargetNode.isNull())
            {
                // there is a node at target position
                if (!mSwap) return false;
                mGridPlacement[mTargetNode] = ctx->getLayouter()->nodeToPositionMap().value(ndToMove);
            }
            mGridPlacement[ndToMove] = mTo;
        }

        LayoutLocker llock;
        ctx->updatePlacement(mGridPlacement);
        ctx->scheduleSceneUpdate();

        return UserAction::exec();
    }
}
