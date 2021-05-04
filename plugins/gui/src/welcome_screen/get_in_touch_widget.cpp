#include "gui/welcome_screen/get_in_touch_widget.h"

#include "gui/welcome_screen/get_in_touch_item.h"
#include "gui/main_window/about_dialog.h"

#include <QDesktopServices>
#include <QLabel>
#include <QList>
#include <QStyle>
#include <QUrl>
#include <QVBoxLayout>

namespace hal
{
    GetInTouchWidget::GetInTouchWidget(QWidget* parent)
        : QFrame(parent), mLayout(new QVBoxLayout()), mAboutItem(new GetInTouchItem("About", "Show license and version information")),
          mNewsItem(new GetInTouchItem("Stay up to date", "Subscribe to our newsfeed for the latest updates and patchnotes")),
          mForumItem(new GetInTouchItem("Get in touch", "Check out the forum and so on and so on i dont know what to write here")),
          mCppDocumentationItem(new GetInTouchItem("Open C++ Documentation", "Check out the C++ documentation of the HAL core to develop your own plugins")),
          mPyDocumentationItem(new GetInTouchItem("Open Python Documentation", "Check out the Python documentation of HAL")),
          mTicketItem(new GetInTouchItem("Found a bug ?", "Submit a bug report or feature request to our public tracker"))
    {
        connect(mAboutItem, &GetInTouchItem::clicked, this, &GetInTouchWidget::handleAboutItemClicked);
        connect(mCppDocumentationItem, &GetInTouchItem::clicked, this, &GetInTouchWidget::handleCppDocumentationItemClicked);
        connect(mPyDocumentationItem, &GetInTouchItem::clicked, this, &GetInTouchWidget::handlePyDocumentationItemClicked);
        connect(mTicketItem, &GetInTouchItem::clicked, this, &GetInTouchWidget::handleTicketItemClicked);

        mLayout->setContentsMargins(0, 0, 0, 0);
        mLayout->setSpacing(0);

        mAboutItem->setObjectName("about_item");
        mAboutItem->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        mNewsItem->setObjectName("news_item");
        mNewsItem->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        mForumItem->setObjectName("forum_item");
        mForumItem->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        mCppDocumentationItem->setObjectName("documentation_item");
        mCppDocumentationItem->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        mPyDocumentationItem->setObjectName("documentation_item");
        mPyDocumentationItem->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        mTicketItem->setObjectName("ticket_item");
        mTicketItem->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        setLayout(mLayout);
        mLayout->addWidget(mAboutItem);
        //    mLayout->addWidget(mNewsItem);
        //    mLayout->addWidget(mForumItem);
        mLayout->addWidget(mCppDocumentationItem);
        mLayout->addWidget(mPyDocumentationItem);
        mLayout->addWidget(mTicketItem);

        repolish();
    }

    void GetInTouchWidget::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        //    mAboutItem->repolish();
        //    m_documentation_item->repolish();
        //    mTicketItem->repolish();

        //    for (QObject* object : mLayout->children())
        //    {
        //        GetInTouchItem* item = qobject_cast<GetInTouchItem*>(object);

        //        if (item)
        //            item->repolish();
        //    }

        for (int i = 0; i < mLayout->count(); i++)
        {
            QWidget* widget         = mLayout->itemAt(i)->widget();
            GetInTouchItem* item = qobject_cast<GetInTouchItem*>(widget);

            if (item)
                item->repolish();
        }
    }

    void GetInTouchWidget::handleAboutItemClicked()
    {
        AboutDialog ad;
        ad.exec();
    }

    void GetInTouchWidget::handleCppDocumentationItemClicked()
    {
        QString link = "https://emsec.github.io/hal/doc/";
        QDesktopServices::openUrl(QUrl(link));
    }

    void GetInTouchWidget::handlePyDocumentationItemClicked()
    {
        QString link = "https://emsec.github.io/hal/pydoc/";
        QDesktopServices::openUrl(QUrl(link));
    }

    void GetInTouchWidget::handleTicketItemClicked()
    {
        QString link = "https://github.com/emsec/hal/issues";
        QDesktopServices::openUrl(QUrl(link));
    }
}
