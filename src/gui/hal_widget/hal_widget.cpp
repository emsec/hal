#include "hal_widget/hal_widget.h"
#include <QPainter>
#include <QStyleOption>
namespace hal{
hal_widget::hal_widget(QWidget* parent) : QWidget(parent)
{
}

//void hal_widget::paintEvent(QPaintEvent* /*event*/)
//{
//    QStyleOption opt;
//    opt.init(this);
//    QPainter p(this);
//    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
//}

void hal_widget::setParent(QWidget* parent)
{
    QWidget::setParent(parent);
    Q_EMIT parent_set_to(parent);
}

void hal_widget::hide()
{
    QWidget::hide();
    Q_EMIT hidden();
}

void hal_widget::show()
{
    QWidget::show();
    Q_EMIT shown();
}
}
