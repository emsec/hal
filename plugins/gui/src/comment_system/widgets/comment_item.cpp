#include "gui/comment_system/widgets/comment_item.h"
#include "gui/comment_system/comment_entry.h"
#include "gui/comment_system/widgets/comment_dialog.h"

#include <QToolBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QAction>
#include <QToolButton>
#include <QTextEdit>
#include <QDebug>


namespace hal
{
    CommentItem::CommentItem(QWidget* parent) : QFrame(parent)
    {
        init();
    }

    CommentItem::CommentItem(CommentEntry *entry, QWidget *parent) : QFrame(parent)
    {
        init();
        mEntry = entry;
        updateCurrentEntry();
    }

    CommentItem::~CommentItem()
    {
    }

    void CommentItem::setComment(CommentEntry *entry)
    {
        mEntry = entry;
        updateCurrentEntry();
    }

    void CommentItem::updateCurrentEntry()
    {
        if(!mEntry) return;
        mHeader->setText(mEntry->getHeader());
        mCreationDate->setText("  " + mEntry->getCreationTime().toString("dd.MM.yy hh:mm"));
        mTextEdit->setHtml(mEntry->getText());
        //setFixedHeight(mTopWidget->height()+mText->height());
    }

    CommentEntry *CommentItem::getEntry()
    {
        return mEntry;
    }

    void CommentItem::init()
    {
        mEntry = nullptr;
        // TODO: replace fixed sizes (put them in stylesheet / compute them?)
        mLayout = new QVBoxLayout(this);
        mLayout->setSpacing(0);
        mLayout->setMargin(0);

        // top part
        mTopWidget = new QWidget(this);
        mTopWidget->setFixedHeight(30);
        mTopLayout = new QHBoxLayout(mTopWidget);
        mTopLayout->setSpacing(0);
        mTopLayout->setMargin(0);
        //mTopWidget->setLayout(mTopLayout);

        mHeader = new QLabel(this);
        mHeader->setStyleSheet("font-weight: bold;");
        mCreationDate = new QLabel(this);
        mCreationDate->setStyleSheet("font-size: 12px;");
        mModifyButton = new QToolButton(this);
        mModifyButton->setIcon(QIcon(":/icons/pen"));
        mModifyButton->setIconSize(QSize(30,30));
        mModifyButton->setAutoRaise(true);
        mDeleteButton = new QToolButton(this);
        mDeleteButton->setIcon(QIcon(":/icons/trashcan"));
        mDeleteButton->setIconSize(QSize(30,30));
        mDeleteButton->setAutoRaise(true);

        //perhaps with alignments (header left, date normal, small spacer, fixed buttons without stretch)?
        mTopLayout->addWidget(mHeader);
        mTopLayout->addWidget(mCreationDate);
        mTopLayout->addWidget(mModifyButton);
        mTopLayout->addWidget(mDeleteButton);
        mTopWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        // actual comment
        mTextEdit = new QTextEdit(this);
        mTextEdit->setReadOnly(true);

        mLayout->addWidget(mTopWidget);
        //mLayout->addWidget(mText);
        mLayout->addWidget(mTextEdit);
        mLayout->addStretch();

//        QWidget* hackySpacer = new QWidget();
//        hackySpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//        mLayout->addWidget(hackySpacer);

        //setLayout(mLayout);

        // connections / logic
        connect(mDeleteButton, &QAbstractButton::clicked, this, &CommentItem::handleDeleteButtonTriggered);
        connect(mModifyButton, &QAbstractButton::clicked, this, &CommentItem::handleModifyButtonTriggered);
    }

    void CommentItem::handleDeleteButtonTriggered()
    {
        qDebug() << "I want to be deleted!";
        Q_EMIT delete_requested(this);
    }

    void CommentItem::handleModifyButtonTriggered()
    {
        qDebug() << "I want to be modified!";
        CommentDialog dialog("Modify Comment", mEntry);
        dialog.exec();
    }
}
