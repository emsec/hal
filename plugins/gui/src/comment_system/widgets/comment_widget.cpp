#include "gui/comment_system/widgets/comment_widget.h"
#include "gui/gui_globals.h"
#include "gui/comment_system/comment_entry.h"
#include "gui/comment_system/widgets/comment_item.h"
#include <QDebug>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include <QAction>
#include <QIcon>
#include <QSpacerItem>
#include <QToolBar>
#include "gui/toolbar/toolbar.h"
#include "gui/searchbar/searchbar.h"

namespace hal
{
    CommentWidget::CommentWidget(QWidget *parent) : QWidget(parent)
    {
//        QVBoxLayout* layout = new QVBoxLayout(this);
//        _mTemporaryTextOutputForDebuggingOnly_ = new QLabel(this);
//        layout->addWidget(_mTemporaryTextOutputForDebuggingOnly_);

        mTopLayout = new QVBoxLayout(this);
        mTopLayout->setMargin(0);
        mTopLayout->setSpacing(0);
        mSearchbar = new Searchbar();
        // top bar
        // 1. Option
        mHeaderLayout = new QHBoxLayout();
        mNewCommentButton = new QToolButton();
        mNewCommentButton->setIcon(QIcon(":/icons/plus"));
        mNewCommentButton->setIconSize(QSize(25,25));
        mSearchButton = new QToolButton();
        mSearchButton->setIcon(QIcon(":/icons/search"));
        mSearchButton->setIconSize(QSize(25,25));
        mHeaderLayout->addWidget(mNewCommentButton);// alignleft without spacer
        mHeaderLayout->addSpacerItem(new QSpacerItem(0,0, QSizePolicy::Expanding, QSizePolicy::Preferred));
        mHeaderLayout->addWidget(mSearchButton);// alignright without spacer
        mHeaderLayout->addWidget(mSearchbar);
        mTopLayout->addLayout(mHeaderLayout);

//        mToolbar = new QToolBar("Title?");
//        //mToolbar = new Toolbar();
//        QAction* newCommentAction = new QAction(QIcon(":/icons/plus"), "New Comment");
//        QAction* mSearchAction = new QAction(QIcon(":/icons/search"), "Search");
//        mToolbar->addAction(newCommentAction);
//        //mToolbar->addSpacer();
//        QWidget* spacer = new QWidget(); // must be used when using QToolBar
//        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
//        mToolbar->addWidget(spacer);
//        mToolbar->addAction(mSearchAction);
//        mToolbar->addWidget(mSearchbar);
//        mSearchbar->show();
//        mToolbar->setMinimumHeight(mSearchbar->height());// can be set if QToolbar is used (not custom)
//        mTopLayout->addWidget(mToolbar);

        // comment part
        mCommentsLayout = new QVBoxLayout();
        mScrollArea = new QScrollArea();
        mScrollArea->setWidgetResizable(true);//important as it seems so that the child widget will expandd if possible
        mCommentsContainer = new QWidget();
        mCommentsContainer->show();
        mCommentsContainer->setLayout(mCommentsLayout);
        mCommentsContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mScrollArea->setWidget(mCommentsContainer);
        mScrollArea->show();
        //mCommentsContainer->setMinimumSize(500,200);
        //mTopLayout->addWidget(mScrollArea);

        // test fillings
//        QLabel* commentImposter = new QLabel("Perhaps i will be a comment someday?");
//        commentImposter->setWordWrap(true);
//        //commentImposter->show();
//        //mScrollArea->setWidget(commentImposter);
//        mCommentsLayout->addWidget(commentImposter);
//        mCommentsLayout->addWidget(new QLabel("II will be another Comment (hopefully)"));
        mTopLayout->addWidget(mScrollArea);

        // testing, remove later
        setMinimumWidth(350);
        resize(350, 300);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
        // connections / logic
        connect(mSearchButton, &QAbstractButton::clicked, this, &CommentWidget::handleSearchbarTriggered);

    }

    CommentWidget::~CommentWidget()
    {
        qDebug() << "CommentWidget::~CommentWidget()";
    }

    void CommentWidget::setItem(CommentItem *item)
    {
        mCommentsLayout->addWidget(item);
    }

    void CommentWidget::nodeChanged(const Node& nd)
    {
//        QString txt;
//        QList<CommentEntry*> ceList = gCommentManager->getEntriesForNode(nd);
//        for (const CommentEntry* ce : ceList)
//           txt += ce->getHeader() + "\n  created: " +
//                   ce->getCreationTime().toString("dd.MM.yyyy hh:mm:ss\n") +
//                   ce->getText() + "\n-----------\n";
        //        _mTemporaryTextOutputForDebuggingOnly_->setText(txt);



        auto commentList = gCommentManager->getEntriesForNode(nd);

        // remove old items (only delete widgets, not actual CommentEntries
        QLayoutItem* firstItem = nullptr;
        while((firstItem = mCommentsLayout->takeAt(0)) != nullptr){
            delete firstItem->widget();
            delete firstItem;
        }


        // create new items
        for(const auto& entry : commentList)
        {
            qDebug() << "im in this loop!";
            CommentItem* item = new CommentItem(entry);
            mCommentsLayout->addWidget(item);
            item->show();
        }
        qDebug() << "Count of items: " << mCommentsLayout->count();
        for(int i = 0; i < mCommentsLayout->count(); i++)
            qDebug() << "creation: " << dynamic_cast<CommentItem*>(mCommentsLayout->itemAt(i)->widget())->getCreatedDate()->text();
    }

    void CommentWidget::init_style1()
    {

    }

    void CommentWidget::init_style2()
    {

    }

    void CommentWidget::handleSearchbarTriggered()
    {
        mSearchbar->isHidden() ? mSearchbar->show() : mSearchbar->hide();
    }

    void CommentWidget::handleNewCommentTriggered()
    {

    }

}
