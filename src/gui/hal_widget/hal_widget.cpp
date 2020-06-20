#include "hal_widget/hal_widget.h"
#include <QPainter>
#include <QStyleOption>

namespace hal
{
    HalWidget::HalWidget(QWidget* parent) : QWidget(parent)
    {
    }

    //void HalWidget::paintEvent(QPaintEvent* /*event*/)
    //{
    //    QStyleOption opt;
    //    opt.init(this);
    //    QPainter p(this);
    //    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    //}

    void HalWidget::setParent(QWidget* parent)
    {
        QWidget::setParent(parent);
        Q_EMIT parent_set_to(parent);
    }

    void HalWidget::hide()
    {
        QWidget::hide();
        Q_EMIT hidden();
    }

    void HalWidget::show()
    {
        QWidget::show();
        Q_EMIT shown();
    }
}
