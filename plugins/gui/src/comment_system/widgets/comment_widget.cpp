#include "gui/comment_system/widgets/comment_widget.h"
#include "gui/gui_globals.h"
#include "gui/comment_system/comment_entry.h"
#include "gui/comment_system/widgets/comment_item.h"
#include "gui/comment_system/widgets/comment_dialog.h"
#include "gui/hal_qt_compat/hal_qt_compat.h"
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
#include "gui/gui_utils/graphics.h"

namespace hal
{
    CommentWidget::CommentWidget(QWidget *parent) : QWidget(parent)
    {
        // take properties out of the stylesheet
        style()->unpolish(this);
        style()->polish(this);

        //mTopLayout = new QVBoxLayout(this);
        mTopLayout = new QGridLayout(this);
        QtCompat::setMarginWidth(mTopLayout,0);
        mTopLayout->setSpacing(0);
        mSearchbar = new Searchbar();
        mSearchbar->setEmitTextWithFlags(false);
        mSearchbar->hide();

        QSize iconSize(20, 20);

        // top bar
        // 1. Option
        mHeaderLayout = new QHBoxLayout();
        mNewCommentButton = new QToolButton();
        mNewCommentButton->setIcon(gui_utility::getStyledSvgIcon(mNewCommentIconStyle, mNewCommentIconPath));
        mNewCommentButton->setIconSize(iconSize);
        mSearchButton = new QToolButton();
        mSearchButton->setIcon(gui_utility::getStyledSvgIcon(mSearchIconStyle, mSearchIconPath));
        mSearchButton->setIconSize(iconSize);
        mHeaderLayout->addWidget(mNewCommentButton);// alignleft without spacer
        //mHeaderLayout->addSpacerItem(new QSpacerItem(0,0, QSizePolicy::Expanding, QSizePolicy::Preferred));
        QWidget* spacerWidget = new QWidget; // spacer items are in the overlay transparent, widgets not
        spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        mHeaderLayout->addWidget(spacerWidget);
        mHeaderLayout->addWidget(mSearchButton);// alignright without spacer
        mHeaderLayout->addWidget(mSearchbar);

        // comment part
        mCommentsLayout = new QVBoxLayout();
        mCommentsLayout->setSpacing(0);
        QtCompat::setMarginWidth(mCommentsLayout,0);
        mScrollArea = new QScrollArea();
        mScrollArea->setWidgetResizable(true);//important as it seems so that the child widget will expandd if possible
        mCommentsContainer = new QWidget();
        mCommentsContainer->show();
        mCommentsContainer->setLayout(mCommentsLayout);
        mCommentsContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mScrollArea->setWidget(mCommentsContainer);
        mScrollArea->show();

        mTopLayout->addLayout(mHeaderLayout,0,0);
        mTopLayout->addWidget(mScrollArea, 1, 0);
        //mTopLayout->addItem(new QSpacerItem(0,0, QSizePolicy::Expanding, QSizePolicy::Expanding),2,0);
        mTopLayout->setRowStretch(1, 0);


        // testing, remove later
        setMinimumWidth(350);
        resize(350, 300);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

        // connections / logic
        connect(mSearchButton, &QAbstractButton::clicked, this, &CommentWidget::handleSearchbarTriggered);
        connect(mSearchbar, &Searchbar::searchIconClicked, this, &CommentWidget::handleSearchbarTriggered);
        connect(mSearchbar, &Searchbar::triggerNewSearch, this, &CommentWidget::handleSearchbarTextEdited);
        connect(mNewCommentButton, &QAbstractButton::clicked, this, &CommentWidget::handleNewCommentTriggered);
        connect(gCommentManager, &CommentManager::entryAboutToBeDeleted, this, &CommentWidget::handleCommentAboutToBeDeleted);
        connect(gCommentManager, &CommentManager::entryAdded, this, &CommentWidget::handleCommentAdded);
        connect(gCommentManager, &CommentManager::entryModified, this, &CommentWidget::handleEntryModified);
    }

    CommentWidget::~CommentWidget()
    {
    }

    void CommentWidget::nodeChanged(const Node& nd)
    {
        mCurrentNode = nd;
        mScrollArea->setWidget(createAndFillCommentContainerFactory(nd));
    }

    QWidget *CommentWidget::createAndFillCommentContainerFactory(const Node &nd)
    {
        mEntryItems.clear();
        QWidget* container = new QWidget();
        QVBoxLayout* containerLayout = new QVBoxLayout(container);
        containerLayout->setSpacing(0);
        QtCompat::setMarginWidth(containerLayout,0);

        auto commentList = gCommentManager->getEntriesForNode(nd);

        // create new items
        for(const auto& entry : commentList)
        {
            CommentItem* item = new CommentItem(entry, container);
            connect(item, &CommentItem::delete_requested, this, &CommentWidget::handleCommentEntryDeleteRequest);
            //item->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
            containerLayout->addWidget(item);
            item->show();
            mEntryItems.append(item);
        }

        containerLayout->addStretch();
        return container;
    }

    void CommentWidget::handleSearchbarTriggered()
    {
        if(mSearchbar->isHidden())
        {
            mSearchButton->hide();
            mSearchbar->show();
        }
        else
        {
            mSearchbar->hide();;
            mSearchButton->show();
        }
    }

    void CommentWidget::handleNewCommentTriggered()
    {
        CommentDialog commentDialog("New Comment");
        if(commentDialog.exec() == QDialog::Accepted)
        {
            gCommentManager->addComment(new CommentEntry(mCurrentNode, commentDialog.getText(), commentDialog.getHeader()));
        }
        commentDialog.close();
    }

    void CommentWidget::handleCommentEntryDeleteRequest(CommentItem *item)
    {
        // remove item from local list and layout
        mEntryItems.removeOne(item);
        mScrollArea->widget()->layout()->removeWidget(item);

        // notify global manager to delete the entry
        gCommentManager->deleteComment(item->getEntry());

        // delete commentitem
        item->deleteLater();
    }

    void CommentWidget::handleCommentAboutToBeDeleted(CommentEntry *entry)
    {
        // search for corresponding commentitem
        CommentItem* commentItem = nullptr;
        for(const auto& item : mEntryItems)
        {
            if(item->getEntry() == entry)
            {
                commentItem = item;
                break;
            }
        }

        if(commentItem == nullptr)
            return;

        // remove and delete item
        mEntryItems.removeOne(commentItem);
        mScrollArea->widget()->layout()->removeWidget(commentItem);
        commentItem->deleteLater();
    }

    void CommentWidget::handleEntryModified(CommentEntry *entry)
    {
        if(entry->getNode() != mCurrentNode)
            return;

        for(const auto& item : mEntryItems)
        {
            if(item->getEntry() == entry)
            {
                item->updateCurrentEntry();
                break;
            }
        }
    }

    void CommentWidget::handleCommentAdded(CommentEntry *entry)
    {
        if(entry->getNode() != mCurrentNode)
            return;

        // 1. just update all...
        nodeChanged(mCurrentNode);

        // 2. manually just create new commentitem and insert it
//        CommentItem* item = new CommentItem(entry);
//        mEntryItems.insert(0, item);
//        auto layout = dynamic_cast<QBoxLayout*>(mScrollArea->widget()->layout());
//        layout->insertWidget(0, item);
    }

    void CommentWidget::handleSearchbarTextEdited(const QString &text, SearchOptions opts)
    {
        for(const auto &item : mEntryItems)
        {
            if(item->search(text, opts))
            {
                // jump to last location
                // if one wants to jump to the first location and cicle through the hits with the enter-key,
                // the keypressedevent event must be overriden and an index list with the indices corresponding
                // to the commentitems must be saved here together with a currentIndex var
                mScrollArea->ensureWidgetVisible(item);
            }
        }
    }

}
