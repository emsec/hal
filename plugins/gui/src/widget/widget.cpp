#include "gui/widget/widget.h"
#include <QPainter>
#include <QStyleOption>

namespace hal
{
    Widget::Widget(QWidget* parent) : QWidget(parent)
    {
    }

    //void Widget::paintEvent(QPaintEvent* /*event*/)
    //{
    //    QStyleOption opt;
    //    opt.init(this);
    //    QPainter p(this);
    //    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    //}

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
