#include "gui/settings/preview_widget.h"

#include <QVBoxLayout>

namespace hal
{
    PreviewWidget::PreviewWidget(QWidget* parent) : QFrame(parent), mLayout(new QVBoxLayout())
    {
        setFrameStyle(QFrame::NoFrame);
        mLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        mLayout->setContentsMargins(0, 0, 0, 0);
        mLayout->setSpacing(0);
        setLayout(mLayout);
    }
}
