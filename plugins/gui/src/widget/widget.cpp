#include "gui/widget/widget.h"

namespace hal
{
    Widget::Widget(QWidget* parent) : QWidget(parent)
    {
    }

    void Widget::setParent(QWidget* parent)
    {
        QWidget::setParent(parent);
        Q_EMIT parentSetTo(parent);
    }

    void Widget::hide()
    {
        QWidget::hide();
        Q_EMIT hidden();
    }

    void Widget::show()
    {
        QWidget::show();
        Q_EMIT shown();
    }
}
