#include "gui/settings/fontsize_preview_widget.h"

#include <QVBoxLayout>
#include <QVariant>

namespace hal
{
    FontsizePreviewWidget::FontsizePreviewWidget(const QString& text, const QFont& font, QWidget* parent) : PreviewWidget(parent), mLabel(new QLabel()), mFont(font)
    {
        setFrameStyle(QFrame::Box);
        mLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        mLayout->setContentsMargins(0, 0, 0, 0);
        mLayout->setSpacing(0);
        setLayout(mLayout);
        mLabel->setText(text);
        mLabel->setFont(font);
        mLayout->addWidget(mLabel);
    }

    void FontsizePreviewWidget::update(const QVariant& value)
    {
        mFont.setPointSize(value.toInt());
        mLabel->setFont(mFont);
    }
}
