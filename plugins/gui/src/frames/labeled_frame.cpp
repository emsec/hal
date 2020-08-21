#include "frames/labeled_frame.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace hal
{
    LabeledFrame::LabeledFrame(QWidget* parent)
        : QFrame(parent), m_layout(new QVBoxLayout), m_header(new QFrame()), m_header_layout(new QHBoxLayout()), m_left_spacer(new QFrame()), m_label(new QLabel()), m_right_spacer(new QFrame())
    {
        setLayout(m_layout);
        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->setSpacing(0);
        m_layout->setAlignment(Qt::AlignTop);

        m_header->setObjectName("header");
        //m_header->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
        m_header->setLayout(m_header_layout);
        m_layout->addWidget(m_header);
        //m_layout->setAlignment(m_header, Qt::AlignTop);

        m_header_layout->setContentsMargins(0, 0, 0, 0);
        m_header_layout->setSpacing(0);

        m_left_spacer->setObjectName("left-spacer");
        m_left_spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
        m_header_layout->addWidget(m_left_spacer);

        m_label->setObjectName("label");
        m_label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        m_header_layout->addWidget(m_label);

        m_right_spacer->setObjectName("right-spacer");
        m_right_spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
        m_header_layout->addWidget(m_right_spacer);
    }

    void LabeledFrame::add_content(QWidget* content)
    {
        m_layout->addWidget(content);
    }
}
