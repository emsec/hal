#include "gui/welcome_screen/welcome_screen.h"

#include "gui/frames/labeled_frame.h"
#include "gui/graphics_effects/shadow_effect.h"
#include "gui/welcome_screen/get_in_touch_widget.h"
#include "gui/welcome_screen/open_file_widget.h"
#include "gui/welcome_screen/recent_files_widget.h"

#include <QHBoxLayout>
#include <QStyle>

namespace hal
{
    WelcomeScreen::WelcomeScreen(QWidget* parent)
        : QFrame(parent), mLayout(new QHBoxLayout()), mRecentFilesFrame(new LabeledFrame()), mOpenFileFrame(new LabeledFrame()), mGetInTouchFrame(new LabeledFrame()),
          mRecentFilesWidget(new RecentFilesWidget()), mOpenFileWidget(new OpenFileWidget()), mGetInTouchWidget(new GetInTouchWidget())
    {
        mLayout->setContentsMargins(0, 0, 0, 0);
        mLayout->setSpacing(0);
        mLayout->setAlignment(Qt::AlignCenter);

        mRecentFilesFrame->setObjectName("recent-files-frame");
        mOpenFileFrame->setObjectName("open-file-frame");
        mGetInTouchFrame->setObjectName("get-in-touch-frame");

        mRecentFilesFrame->addContent(mRecentFilesWidget);
        mOpenFileFrame->addContent(mOpenFileWidget);
        mGetInTouchFrame->addContent(mGetInTouchWidget);

        mRecentFilesFrame->setGraphicsEffect(new ShadowEffect());
        mOpenFileFrame->setGraphicsEffect(new ShadowEffect());
        mGetInTouchFrame->setGraphicsEffect(new ShadowEffect());

        setLayout(mLayout);
        mLayout->addWidget(mRecentFilesFrame);
        mLayout->addWidget(mOpenFileFrame);
        mLayout->addWidget(mGetInTouchFrame);

        repolish();    // CALL FROM PARENT
    }

    void WelcomeScreen::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        mRecentFilesWidget->repolish();
        mOpenFileWidget->repolish();
        mGetInTouchWidget->repolish();
    }
}
