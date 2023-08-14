#include "gui/user_action/action_move_node.h"
#include <QStringList>
#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/gui_globals.h"

namespace hal
{
    ActionMoveNodeFactory::ActionMoveNodeFactory()
       : UserActionFactory("MoveNode") {;}

    ActionMoveNodeFactory* ActionMoveNodeFactory::sFactory = new ActionMoveNodeFactory;

    UserAction* ActionMoveNodeFactory::newAction() const
    {
        return new ActionMoveNode;
    }

    ActionMoveNode::ActionMoveNode(u32 ctxID, const QPoint& from, const QPoint& to)
    {
        mContextId = ctxID;
        GraphContext* ctx = mContextId >= 0
                    ? gGraphContextManager->getContextById(mContextId)
                    : gGraphContextManager->getContextById(mObject.id());

        mGridPlacement = GuiApiClasses::View::getGridPlacement(ctxID);
        mFrom = from;
        mTo = to;

        /*GraphContext* context = gGraphContextManager->getContextById(mContextId);
        Node nd = context->getLayouter()->positionToNodeMap().find(from).value();*/
        qInfo() << "mFrom: " << mFrom.isNull();
        qInfo() << "mTo: " << mTo.isNull();
        if(!mFrom.isNull() && !mTo.isNull())
        {

            UserActionObject undoObject(mObject);
            if (mContextId != 0)
            {
                undoObject = UserActionObject(ctx->id(),UserActionObjectType::Context);
            }
            GridPlacement oldGp = *mGridPlacement;
            Node ndToMove = mGridPlacement->key(mFrom); // get the node we want to move
            mGridPlacement->insert(ndToMove, mTo); // set the position of the node to the new one
            ActionMoveNode* undo = new ActionMoveNode(mContextId);
            undo->mGridPlacement = &oldGp;
            undo->setObject(undoObject);
            mUndoAction = undo;
        }

    }

    QString ActionMoveNode::tagname() const
    {
        return ActionMoveNodeFactory::sFactory->tagname();
    }

    void ActionMoveNode::addToHash(QCryptographicHash& cryptoHash) const
    {
        cryptoHash.addData((char*)(&mFrom), sizeof(QPoint));
        cryptoHash.addData((char*)(&mTo)  , sizeof(QPoint));
    }

    void ActionMoveNode::writeToXml(QXmlStreamWriter& xmlOut) const
    {
        xmlOut.writeTextElement("from", QString("%1,%2").arg(mFrom.x()).arg(mFrom.y()));
        xmlOut.writeTextElement("to", QString("%1,%2").arg(mTo.x()).arg(mTo.y()));
    }

    void ActionMoveNode::readFromXml(QXmlStreamReader& xmlIn)
    {
        while (xmlIn.readNextStartElement())
        {
            if (xmlIn.name() == "from")
                mFrom = parseFromString(xmlIn.readElementText());
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
        qInfo() << "1";
        GraphContext* ctx = gGraphContextManager->getContextById(mContextId);
        if(ctx == nullptr) return false;


        ctx->clear();
        QSet<u32> modIds;
        QSet<u32> gateIds;

        qInfo() << "2";
        QList<Node> nodes = mGridPlacement->keys();

        qInfo() << "3";
        for (Node node : nodes)
        {
            if(node.isModule()) modIds.insert(node.id());
            else gateIds.insert(node.id());
        }

        ctx->add(modIds, gateIds);

        /*if (!ctx) return false;
        if (mContextId >= 0)
        {
            Node nd(mObject.id(),UserActionObjectType::toNodeType(mObject.type()));
            NodeBox* box = ctx->getLayouter()->boxes().boxForNode(nd);
            if (!box) return false;
            mFrom.setX(box->x());
            mFrom.setY(box->y());

        }
        ctx->moveNodeAction(mFrom,mTo);*/

        return UserAction::exec();
    }
}
