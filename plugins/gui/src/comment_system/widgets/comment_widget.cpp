#include "gui/comment_system/widgets/comment_widget.h"
#include "gui/gui_globals.h"
#include "gui/comment_system/comment_entry.h"
#include <QVBoxLayout>
#include <QDebug>

namespace hal
{
    CommentWidget::CommentWidget(QWidget *parent) : QWidget(parent)
    {
        QVBoxLayout* layout = new QVBoxLayout(this);
        _mTemporaryTextOutputForDebuggingOnly_ = new QLabel(this);
        layout->addWidget(_mTemporaryTextOutputForDebuggingOnly_);
    }

    CommentWidget::~CommentWidget()
    {
        qDebug() << "CommentWidget::~CommentWidget()";
    }

    void CommentWidget::nodeChanged(const Node& nd)
    {
        QString txt;
        QList<CommentEntry*> ceList = gCommentManager->getEntriesForNode(nd);
        for (const CommentEntry* ce : ceList)
           txt += ce->getHeader() + "\n  created: " +
                   ce->getCreationTime().toString("dd.MM.yyyy hh:mm:ss\n") +
                   ce->getText() + "\n-----------\n";
        _mTemporaryTextOutputForDebuggingOnly_->setText(txt);
    }

}
