#include "welcome_screen/welcome_screen.h"

#include "frames/labeled_frame.h"
#include "graphics_effects/shadow_effect.h"
#include "welcome_screen/get_in_touch_widget.h"
#include "welcome_screen/open_file_widget.h"
#include "welcome_screen/recent_files_widget.h"

#include <QHBoxLayout>
#include <QStyle>

namespace hal
{
    welcome_screen::welcome_screen(QWidget* parent)
        : QFrame(parent), m_layout(new QHBoxLayout()), m_recent_files_frame(new LabeledFrame()), m_open_file_frame(new LabeledFrame()), m_get_in_touch_frame(new LabeledFrame()),
          m_recent_files_widget(new recent_files_widget()), m_open_file_widget(new open_file_widget()), m_get_in_touch_widget(new get_in_touch_widget())
    {
        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->setSpacing(0);
        m_layout->setAlignment(Qt::AlignCenter);

        m_recent_files_frame->setObjectName("recent-files-frame");
        m_open_file_frame->setObjectName("open-file-frame");
        m_get_in_touch_frame->setObjectName("get-in-touch-frame");

        m_recent_files_frame->add_content(m_recent_files_widget);
        m_open_file_frame->add_content(m_open_file_widget);
        m_get_in_touch_frame->add_content(m_get_in_touch_widget);

        m_recent_files_frame->setGraphicsEffect(new ShadowEffect());
        m_open_file_frame->setGraphicsEffect(new ShadowEffect());
        m_get_in_touch_frame->setGraphicsEffect(new ShadowEffect());

        setLayout(m_layout);
        m_layout->addWidget(m_recent_files_frame);
        m_layout->addWidget(m_open_file_frame);
        m_layout->addWidget(m_get_in_touch_frame);

        repolish();    // CALL FROM PARENT
    }

    void welcome_screen::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        m_recent_files_widget->repolish();
        m_open_file_widget->repolish();
        m_get_in_touch_widget->repolish();
    }
}
