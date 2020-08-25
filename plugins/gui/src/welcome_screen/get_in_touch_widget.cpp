#include "gui/welcome_screen/get_in_touch_widget.h"

#include "gui/welcome_screen/get_in_touch_item.h"

#include <QDesktopServices>
#include <QLabel>
#include <QList>
#include <QStyle>
#include <QUrl>
#include <QVBoxLayout>

namespace hal
{
    GetInTouchWidget::GetInTouchWidget(QWidget* parent)
        : QFrame(parent), m_layout(new QVBoxLayout()), m_about_item(new GetInTouchItem("About", "Show license and version information")),
          m_news_item(new GetInTouchItem("Stay up to date", "Subscribe to our newsfeed for the latest updates and patchnotes")),
          m_forum_item(new GetInTouchItem("Get in touch", "Check out the forum and so on and so on i dont know what to write here")),
          m_cpp_documentation_item(new GetInTouchItem("Open C++ Documentation", "Check out the C++ documentation of the HAL core to develop your own plugins")),
          m_py_documentation_item(new GetInTouchItem("Open Python Documentation", "Check out the Python documentation of HAL")),
          m_ticket_item(new GetInTouchItem("Found a bug ?", "Submit a bug report or feature request to our public tracker")), m_core_version_label(new QLabel()),
          m_gui_version_label(new QLabel())
    {
        connect(m_about_item, &GetInTouchItem::clicked, this, &GetInTouchWidget::handle_about_item_clicked);
        connect(m_cpp_documentation_item, &GetInTouchItem::clicked, this, &GetInTouchWidget::handle_cpp_documentation_item_clicked);
        connect(m_py_documentation_item, &GetInTouchItem::clicked, this, &GetInTouchWidget::handle_py_documentation_item_clicked);
        connect(m_ticket_item, &GetInTouchItem::clicked, this, &GetInTouchWidget::handle_ticket_item_clicked);

        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->setSpacing(0);

        m_about_item->setObjectName("about_item");
        m_about_item->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        m_news_item->setObjectName("news_item");
        m_news_item->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        m_forum_item->setObjectName("forum_item");
        m_forum_item->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        m_cpp_documentation_item->setObjectName("documentation_item");
        m_cpp_documentation_item->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        m_py_documentation_item->setObjectName("documentation_item");
        m_py_documentation_item->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        m_ticket_item->setObjectName("ticket_item");
        m_ticket_item->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        // LOOKS DUMB THINK OF SOMETHING BETTER OR DISCARD
        m_core_version_label->setObjectName("core-version-label");
        m_core_version_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        m_core_version_label->setAlignment(Qt::AlignCenter);
        //m_core_version_label->setText("HAL Core Version 1.36.14 - <font color='#84d300'>89 Days old</font>");
        m_core_version_label->setText("<font color='#292b2d'>HAL Core Version 1.36.14 - 03.08.2018</font>");

        m_gui_version_label->setObjectName("gui-version-label");
        m_gui_version_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        m_gui_version_label->setAlignment(Qt::AlignCenter);
        m_gui_version_label->setText("<font color='#292b2d'>HAL GUI Version 1.33.7 - 03.08.2018</font>");

        setLayout(m_layout);
        m_layout->addWidget(m_about_item);
        //    m_layout->addWidget(m_news_item);
        //    m_layout->addWidget(m_forum_item);
        m_layout->addWidget(m_cpp_documentation_item);
        m_layout->addWidget(m_py_documentation_item);
        m_layout->addWidget(m_ticket_item);
        //    m_layout->addWidget(m_core_version_label, Qt::AlignBottom);
        //    m_layout->setAlignment(m_core_version_label, Qt::AlignBottom);
        //    m_layout->addWidget(m_gui_version_label);

        repolish();
    }

    void GetInTouchWidget::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        //    m_about_item->repolish();
        //    m_documentation_item->repolish();
        //    m_ticket_item->repolish();

        //    for (QObject* object : m_layout->children())
        //    {
        //        GetInTouchItem* item = qobject_cast<GetInTouchItem*>(object);

        //        if (item)
        //            item->repolish();
        //    }

        for (int i = 0; i < m_layout->count(); i++)
        {
            QWidget* widget         = m_layout->itemAt(i)->widget();
            GetInTouchItem* item = qobject_cast<GetInTouchItem*>(widget);

            if (item)
                item->repolish();
        }
    }

    void GetInTouchWidget::handle_about_item_clicked()
    {

    }

    void GetInTouchWidget::handle_cpp_documentation_item_clicked()
    {
        QString link = "https://emsec.github.io/hal/doc/";
        QDesktopServices::openUrl(QUrl(link));
    }

    void GetInTouchWidget::handle_py_documentation_item_clicked()
    {
        QString link = "https://emsec.github.io/hal/pydoc/";
        QDesktopServices::openUrl(QUrl(link));
    }

    void GetInTouchWidget::handle_ticket_item_clicked()
    {
        QString link = "https://github.com/emsec/hal/issues";
        QDesktopServices::openUrl(QUrl(link));
    }
}
