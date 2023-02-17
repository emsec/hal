#include "gui/overlay/widget_overlay.h"

#include <QVBoxLayout>

namespace hal
{
    WidgetOverlay::WidgetOverlay(QWidget* parent) : Overlay(parent),
        mLayout(new QVBoxLayout(this)),
        mWidget(nullptr)
    {
        //setStyleSheet("background-color: rgba(0, 0, 0, 150);");
    }

    void WidgetOverlay::setWidget(QWidget* widget)
    {
        if (mWidget)
        {
            mWidget->hide();
            mWidget->setParent(nullptr);
        }

        mWidget = widget;

        widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        mLayout->addWidget(widget, Qt::AlignCenter);
        mLayout->setAlignment(widget, Qt::AlignCenter);

        mWidget->show();
        //mWidget->setStyleSheet("background-color: rgba(43, 43, 43, 255);");

        // OLD
        //    mWidget->setParent(this);
        //    mWidget->resize(mWidget->sizeHint());
    }
}

//void WidgetOverlay::resizeEvent(QResizeEvent* event)
//{

//}
