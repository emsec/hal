#include "overlay/widget_overlay.h"

#include <QVBoxLayout>

namespace hal
{
    WidgetOverlay::WidgetOverlay(QWidget* parent) : Overlay(parent),
        m_layout(new QVBoxLayout(this)),
        m_widget(nullptr)
    {

    }

    void WidgetOverlay::set_widget(QWidget* widget)
    {
        if (m_widget)
        {
            m_widget->hide();
            m_widget->setParent(nullptr);
        }

        m_widget = widget;

        widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_layout->addWidget(widget, Qt::AlignCenter);
        m_layout->setAlignment(widget, Qt::AlignCenter);

        m_widget->show();

        // OLD
        //    m_widget->setParent(this);
        //    m_widget->resize(m_widget->sizeHint());
    }
}

//void WidgetOverlay::resizeEvent(QResizeEvent* event)
//{

//}
