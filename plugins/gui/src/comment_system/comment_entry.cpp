#include "gui/comment_system/comment_entry.h"
#include <QTextStream>

namespace hal
{

    CommentEntry::CommentEntry(Node n, QString text, QString header) : mNode(n), mHeader(header), mText(text), mDirty(false)
    {
        mCreated = QDateTime::currentDateTime();
        mLastModified = mCreated;
    }

    CommentEntry::CommentEntry(const QJsonObject& jsonObj)
    {
        if (!jsonObj.contains("node") || !jsonObj["node"].isObject()
                || !jsonObj.contains("header")
                || !jsonObj.contains("created")
                || !jsonObj.contains("modified")
                || !jsonObj.contains("text")
                )
            return;
        const QJsonObject& nodeObj = jsonObj["node"].toObject();
        if (!nodeObj.contains("id") || !nodeObj.contains("type"))
            return;
        mNode = Node(nodeObj["id"].toInt(),(Node::NodeType)nodeObj["type"].toInt());
        mHeader = jsonObj["header"].toString();
        //mCreated = QDateTime::fromString(jsonObj["created"].toString());//, "ddd MMM d hh:mm::ss yyyy");
        mCreated = QDateTime::fromString(jsonObj["created"].toString(), "ddd MMM d HH:mm:ss yyyy");
        mLastModified = QDateTime::fromString(jsonObj["modified"].toString());
        mText = jsonObj["text"].toString();
        qDebug() << "Entry created with date: " << mCreated.toString();
        qDebug() << "In field: " << jsonObj["created"].toString();
        qDebug() << "Is valid: " << mCreated.isValid();
        qDebug() << QDateTime::fromString(jsonObj["created"].toString());
    }

    void CommentEntry::setHeader(QString newHeader)
    {
        mHeader = newHeader;
        mLastModified = QDateTime::currentDateTime();
        mDirty = true;
    }

    void CommentEntry::setText(QString newText)
    {
        mText = newText;
        mLastModified = QDateTime::currentDateTime();
        mDirty = true;
    }

    void CommentEntry::setDirty(bool dirty)
    {
        mDirty = dirty;
    }

    QString CommentEntry::getHeader() const
    {
        return mHeader;
    }

    QString CommentEntry::getText() const
    {
        return mText;
    }

    Node CommentEntry::getNode() const
    {
        return mNode;
    }

    QDateTime CommentEntry::getCreationTime() const
    {
        return mCreated;
    }

    QDateTime CommentEntry::getLastModifiedTime() const
    {
        return mLastModified;
    }

    QJsonObject CommentEntry::toJson() const
    {
        QJsonObject retval;
        QJsonObject node;
        node["id"]         = (int) mNode.id();
        node["type"]       = (int) mNode.type();
        retval["node"]     = node;
        retval["header"]   = mHeader;
        retval["created"]  = mCreated.toString();
        retval["modified"] = mLastModified.toString();
        retval["text"]     = mText;
        return retval;
    }

    bool CommentEntry::isInvalid() const
    {
        return mNode.isNull();
    }

    void CommentEntry::dump() const
    {
        QTextStream xout(stdout, QIODevice::WriteOnly);
        xout << (mNode.isGate() ? "Gate" : "Module") << " " << mNode.id() << "\n";
        xout <<  "   header:    <" << mHeader << ">\n";
        xout <<  "   created:   " << mCreated.toString("dd.MM.yyyy hh:mm:ss") << "\n";
        xout <<  "   modified:  " << mLastModified.toString("dd.MM.yyyy hh:mm:ss") << "\n";
        xout <<  "   text:      <" << mText.left(35) << ">\n";
    }

}
