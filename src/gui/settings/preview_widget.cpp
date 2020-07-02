#include "settings/preview_widget.h"

#include <QVBoxLayout>

namespace hal
{
    PreviewWidget::PreviewWidget(QWidget* parent) : QFrame(parent), m_layout(new QVBoxLayout())
    {
        setFrameStyle(QFrame::NoFrame);
        m_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->setSpacing(0);
        setLayout(m_layout);
    }
}
