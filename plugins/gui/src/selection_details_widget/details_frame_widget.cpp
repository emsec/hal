#include "gui/selection_details_widget/details_frame_widget.h"

namespace hal
{
    DetailsFrameWidget::DetailsFrameWidget(QWidget* widget, const QString& text, QWidget* parent) : QWidget(parent)
    {
        m_layout = new QVBoxLayout(this);
        m_layout->setContentsMargins(6, 6, 6, 6);
        m_layout->setSpacing(6);
        m_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

        m_button = new QPushButton(text, this);
        m_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        m_button->setCheckable(true);
        m_button->setChecked(true);

        m_widget= widget;
        m_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        m_layout->addWidget(m_button);
        m_layout->addWidget(m_widget);

        connect(m_button, &QPushButton::clicked, this, &DetailsFrameWidget::handleButtonClicked);
    }

    void DetailsFrameWidget::setText(const QString& text)
    {
        m_button->setText(text);
    }

    void DetailsFrameWidget::handleButtonClicked(bool checked)
    {
        if (checked)
            m_widget->show();
        else
            m_widget->hide();
    }
}
