#include "gui/comment_system/comment_entry.h"

namespace hal
{

    CommentEntry::CommentEntry(Node n, QString text, QString header) : mNode(n), mText(text), mHeader(header), mDirty(false)
    {
        mCreated = QDateTime::currentDateTime();
        mLastModified = mCreated;
    }

    void CommentEntry::setHeader(QString newHeader)
    {
        mHeader = newHeader;
        mLastModified = QDateTime::currentDateTime();
        //perhaps check here of old header = new header and set dirty accordingly?
        mDirty = true;
    }

    void CommentEntry::setText(QString newText)
    {
        mText = newText;
        mLastModified = QDateTime::currentDateTime();
        //perhaps check here of old text = new text and set dirty accordingly?
        mDirty = true;
    }

    void CommentEntry::setDirty(bool dirty)
    {
        mDirty = dirty;
    }

    QString CommentEntry::getHeader()
    {
        return mHeader;
    }

    QString CommentEntry::getText()
    {
        return mText;
    }

    QDateTime CommentEntry::getCreationTime()
    {
        return mCreated;
    }
}
