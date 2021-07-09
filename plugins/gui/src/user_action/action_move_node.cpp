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
        GraphContext* ctx = mContextId >= 0
                    ? gGraphContextManager->getContextById(mContextId)
                    : gGraphContextManager->getContextById(mObject.id());
        UserActionObject undoObject(mObject);
        if (!ctx) return false;
        if (mContextId >= 0)
        {
            Node nd(mObject.id(),UserActionObjectType::toNodeType(mObject.type()));
            NodeBox* box = ctx->getLayouter()->boxes().boxForNode(nd);
            if (!box) return false;
            mFrom.setX(box->x());
            mFrom.setY(box->y());
            undoObject = UserActionObject(ctx->id(),UserActionObjectType::Context);
        }
        ActionMoveNode* undo = new ActionMoveNode(mTo,mFrom);
        undo->setObject(undoObject);
        mUndoAction = undo;
        ctx->moveNodeAction(mFrom,mTo);
        return UserAction::exec();
    }
}
