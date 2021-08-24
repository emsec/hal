#include "gui/new_selection_details_widget/details_frame_widget.h"

namespace hal
{
DetailsFrameWidget::DetailsFrameWidget(QWidget* widget, QWidget* parent) : QWidget(parent)
{
    m_layout = new QVBoxLayout(this);

    m_layout->setContentsMargins(6, 6, 6, 6);
    m_layout->setSpacing(6);

    m_label = new QLabel(this);

    m_label->setText("Output Ports");

    m_layout->addWidget(m_label);
    m_layout->addWidget(widget);
}

void DetailsFrameWidget::setText(const QString& text)
{
    m_label->setText(text);
}
}
