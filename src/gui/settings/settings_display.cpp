#include "settings/settings_display.h"

#include <QFrame>
#include <QVBoxLayout>

namespace hal
{
    SettingsDisplay::SettingsDisplay(QWidget* parent)
        : QScrollArea(parent), m_content(new QFrame), m_content_layout(new QVBoxLayout()), m_sub_content(new QFrame()), m_sub_content_layout(new QVBoxLayout())
    {
        m_content->setFrameStyle(QFrame::NoFrame);

        m_content_layout->setAlignment(Qt::AlignCenter);
        m_content_layout->setContentsMargins(0, 0, 0, 0);
        m_content_layout->setSpacing(0);
        m_content->setLayout(m_content_layout);

        m_sub_content->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        m_sub_content->setFrameStyle(QFrame::NoFrame);
        m_content_layout->addWidget(m_sub_content);

        m_sub_content_layout->setAlignment(Qt::AlignLeft);
        m_sub_content_layout->setContentsMargins(0, 0, 0, 0);
        m_sub_content_layout->setSpacing(0);
        m_sub_content->setLayout(m_sub_content_layout);

        setWidget(m_content);
    }

    void SettingsDisplay::add_widget(QWidget* widget)
    {
        //widget->hide();
        //m_sub_content_layout->addWidget(widget);
        widget->setParent(0);
        widget->show();
    }
}
