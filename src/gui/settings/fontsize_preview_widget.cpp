#include "settings/fontsize_preview_widget.h"

#include <QVBoxLayout>
#include <QVariant>

namespace hal
{
    FontsizePreviewWidget::FontsizePreviewWidget(const QString& text, const QFont& font, QWidget* parent) : PreviewWidget(parent), m_label(new QLabel()), m_font(font)
    {
        setFrameStyle(QFrame::Box);
        m_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->setSpacing(0);
        setLayout(m_layout);
        m_label->setText(text);
        m_label->setFont(font);
        m_layout->addWidget(m_label);
    }

    void FontsizePreviewWidget::update(const QVariant& value)
    {
        m_font.setPointSize(value.toInt());
        m_label->setFont(m_font);
    }
}
