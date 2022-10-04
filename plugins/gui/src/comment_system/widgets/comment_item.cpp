#include "gui/comment_system/widgets/comment_item.h"
#include "gui/comment_system/comment_entry.h"

#include <QToolBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QAction>
#include <QToolButton>


namespace hal
{
    CommentItem::CommentItem(QWidget* parent) : QFrame(parent)
    {
        init();
    }

    CommentItem::CommentItem(CommentEntry *entry, QWidget *parent) : QFrame(parent)
    {
        initV2();
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
        mText->setText(mEntry->getText());
    }

    void CommentItem::init()
    {
        mLayout = new QVBoxLayout(this);
        mLayout->setSpacing(0);
        mLayout->setMargin(0);

        // top part
        mTopToolbar = new QToolBar(this);

        mHeader = new QLabel(this);
        mHeader->setStyleSheet("font-weight: bold;");
        mCreationDate = new QLabel(this);
        mCreationDate->setStyleSheet("font-size: 12px;");

        mModifyAction = new QAction(this); //icon, text, parent
        mDeleteAction = new QAction(this);

        // actual comment
        mText = new QLabel(this);
        mText->setWordWrap(true);

        // setting up actions
        mModifyAction->setIcon(QIcon(":/icons/pen"));
        mDeleteAction->setIcon(QIcon(":/icons/trashcan"));

        //addwidget on a toolbar returns an action -> must this action be saved so it can be deleted later?
        mTopToolbar->addWidget(mHeader);
        QWidget* spacer = new QWidget();
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
        //mTopToolbar->addSeparator();
        mTopToolbar->addWidget(spacer);
        mTopToolbar->addWidget(mCreationDate);
        QWidget* spacer2 = new QWidget();
        spacer2->setFixedWidth(20);
        //spacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
        mTopToolbar->addWidget(spacer2);
        mTopToolbar->addAction(mModifyAction);
        mTopToolbar->addAction(mDeleteAction);

        mLayout->addWidget(mTopToolbar);
        mLayout->addWidget(mText);

        setLayout(mLayout);

        //set max widthh?
    }

    void CommentItem::initV2()
    {
        // TODO: replace fixed sizes (put them in stylesheet / compute them?)
        mLayout = new QVBoxLayout(this);
        mLayout->setSpacing(0);
        mLayout->setMargin(0);

        // top part
        mTopWidget = new QWidget(this);
        mTopWidget->setFixedHeight(30);
        mTopLayout = new QHBoxLayout(this);
        mTopLayout->setSpacing(0);
        mTopLayout->setMargin(0);
        mTopWidget->setLayout(mTopLayout);

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


        // actual comment
        mText = new QLabel(this);
        mText->setWordWrap(true);

        mLayout->addWidget(mTopWidget);
        mLayout->addWidget(mText);

        setLayout(mLayout);
    }
}
