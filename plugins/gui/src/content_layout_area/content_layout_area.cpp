#include "gui/content_layout_area/content_layout_area.h"

#include "gui/docking_system/dock_bar.h"
#include "gui/docking_system/splitter_anchor.h"
#include "gui/docking_system/tab_widget.h"
#include "gui/splitter/splitter.h"
#include "gui/docking_system/content_drag_relay.h"

#include <QHBoxLayout>
#include <QVBoxLayout>


namespace hal
{
    ContentLayoutArea::ContentLayoutArea(QWidget* parent)
        : QWidget(parent), mTopLevelLayout(new QVBoxLayout()), mSecondLevelLayout(new QHBoxLayout()), mThirdLevelLayout(new QVBoxLayout()), mFourthLevelLayout(new QHBoxLayout()),
          mSplitterLayout(new QVBoxLayout()), mCentralLayout(new QHBoxLayout()), mSpacerLayout(new QHBoxLayout()),

          mVerticalSplitter(new Splitter(Qt::Vertical, this)), mHorizontalSplitter(new Splitter(Qt::Horizontal, this)), mLeftSplitter(new Splitter(Qt::Vertical, this)),
          mRightSplitter(new Splitter(Qt::Vertical, this)), mBottomSplitter(new Splitter(Qt::Horizontal, this)),

          mLeftDock(new DockBar(Qt::Vertical, button_orientation::vertical_up, this)), mRightDock(new DockBar(Qt::Vertical, button_orientation::vertical_down, this)),
          mBottomDock(new DockBar(Qt::Horizontal, button_orientation::horizontal, this)),

          mBottomContainer(new QWidget(this)), mLeftSpacer(new QFrame(this)), mRightSpacer(new QFrame(this)),

          mLeftAnchor(new SplitterAnchor(mLeftDock, mLeftSplitter, this)), mRightAnchor(new SplitterAnchor(mRightDock, mRightSplitter, this)),
          mBottomAnchor(new SplitterAnchor(mBottomDock, mBottomSplitter, this)),

          mTabWidget(new TabWidget(this))
    {
        connect(mLeftAnchor, &SplitterAnchor::contentChanged, this, &ContentLayoutArea::updateLeftDockBar);
        connect(mRightAnchor, &SplitterAnchor::contentChanged, this, &ContentLayoutArea::updateRightDockBar);
        connect(mBottomAnchor, &SplitterAnchor::contentChanged, this, &ContentLayoutArea::updateBottomDockBar);

        connect(ContentDragRelay::instance(), &ContentDragRelay::dragStart, this, &ContentLayoutArea::handleDragStart);
        connect(ContentDragRelay::instance(), &ContentDragRelay::dragEnd, this, &ContentLayoutArea::handleDragEnd);

        mLeftDock->setObjectName("left-dock-bar");
        mRightDock->setObjectName("right-dock-bar");
        mBottomDock->setObjectName("bottom-dock-bar");

        mBottomContainer->setObjectName("bottom-container");
        mLeftSpacer->setObjectName("left-spacer");
        mRightSpacer->setObjectName("right-spacer");

        mLeftDock->hide();
        mRightDock->hide();
        mBottomContainer->hide();

        mLeftSpacer->hide();
        mRightSpacer->hide();

        mLeftSplitter->hide();
        mRightSplitter->hide();
        mBottomSplitter->hide();

        mLeftSplitter->setChildrenCollapsible(false);
        mLeftSplitter->setMinimumWidth(340);
        mLeftSplitter->setMinimumHeight(240);

        mRightSplitter->setChildrenCollapsible(false);
        mRightSplitter->setMinimumWidth(340);
        mRightSplitter->setMinimumHeight(240);

        mBottomSplitter->setChildrenCollapsible(false);
        mBottomSplitter->setMinimumWidth(340);
        mBottomSplitter->setMinimumHeight(240);

        mVerticalSplitter->setChildrenCollapsible(false);
        mHorizontalSplitter->setChildrenCollapsible(false);

        setLayout(mTopLevelLayout);

        mTopLevelLayout->setContentsMargins(0, 0, 0, 0);
        mTopLevelLayout->setSpacing(0);
        mTopLevelLayout->addLayout(mSecondLevelLayout);
        mBottomContainer->setLayout(mSpacerLayout);
        mTopLevelLayout->addWidget(mBottomContainer);
        mTopLevelLayout->setAlignment(Qt::AlignTop);

        mSpacerLayout->setContentsMargins(0, 0, 0, 0);
        mSpacerLayout->setSpacing(0);
        mSpacerLayout->addWidget(mLeftSpacer, Qt::AlignLeft);
        mSpacerLayout->addWidget(mBottomDock);
        mSpacerLayout->addWidget(mRightSpacer, Qt::AlignRight);

        mSecondLevelLayout->setContentsMargins(0, 0, 0, 0);
        mSecondLevelLayout->setSpacing(0);
        mSecondLevelLayout->addWidget(mLeftDock, Qt::AlignLeft);
        mSecondLevelLayout->addLayout(mThirdLevelLayout);
        mSecondLevelLayout->addWidget(mRightDock, Qt::AlignRight);

        mThirdLevelLayout->setContentsMargins(0, 0, 0, 0);
        mThirdLevelLayout->setSpacing(0);
        mThirdLevelLayout->addLayout(mFourthLevelLayout);

        mFourthLevelLayout->setContentsMargins(0, 0, 0, 0);
        mFourthLevelLayout->setSpacing(0);
        mFourthLevelLayout->addWidget(mVerticalSplitter);

        mVerticalSplitter->addWidget(mHorizontalSplitter);
        mVerticalSplitter->addWidget(mBottomSplitter);

        mHorizontalSplitter->addWidget(mLeftSplitter);
        mHorizontalSplitter->addWidget(mTabWidget);
        mHorizontalSplitter->addWidget(mRightSplitter);
    }

    void ContentLayoutArea::addContent(ContentWidget* widget, int index, content_anchor anchor)
    {
        switch (anchor)
        {
            case content_anchor::center:
                mTabWidget->add(widget, index);
                break;
            case content_anchor::left:
                mLeftAnchor->add(widget, index);
                break;
            case content_anchor::right:
                mRightAnchor->add(widget, index);
                break;
            case content_anchor::bottom:
                mBottomAnchor->add(widget, index);
                break;
        }
    }

    void ContentLayoutArea::removeContent(ContentWidget* widget)
    {
        Q_UNUSED(widget);
    }

    void ContentLayoutArea::clear()
    {
        mTabWidget->clear();
        mLeftAnchor->clear();
        mRightAnchor->clear();
        mBottomAnchor->clear();
    }

    void ContentLayoutArea::initSplitterSize(const QSize& size)
    {
        int height = size.height() - 240;
        if (height > 0)
            mVerticalSplitter->setSizes(QList<int>{height, 240});
        else
            mVerticalSplitter->setSizes(QList<int>{0, 240});

        int width = size.width() - 600;
        if (width > 0)
            mHorizontalSplitter->setSizes(QList<int>{300, width, 300});
        else
            mHorizontalSplitter->setSizes(QList<int>{240, 120, 240});
    }

    void ContentLayoutArea::updateLeftDockBar()
    {
        if (mLeftDock->count())
        {
            mLeftDock->show();
            mLeftSpacer->show();
        }
        else
        {
            mLeftDock->hide();
            mLeftSpacer->hide();
        }
    }

    void ContentLayoutArea::updateRightDockBar()
    {
        if (mRightDock->count())
        {
            mRightDock->show();
            mRightSpacer->show();
        }
        else
        {
            mRightDock->hide();
            mRightSpacer->hide();
        }
    }

    void ContentLayoutArea::updateBottomDockBar()
    {
        if (mBottomDock->count())
            mBottomContainer->show();
        else
            mBottomContainer->hide();
    }

    void ContentLayoutArea::handleDragStart()
    {
        mWasBottomContainerHidden = mBottomContainer->isHidden();
        mBottomContainer->show();

    }

    void ContentLayoutArea::handleDragEnd()
    {
        if(mWasBottomContainerHidden && mBottomDock->count() == 0)
            mBottomContainer->hide();
    }
}
