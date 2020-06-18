#include "styleable_widget/styleable_widget.h"

#include "QPainter"
#include "QStyleOption"
namespace hal{
styleable_widget::styleable_widget(QWidget* parent) : QWidget(parent)
{
}

void styleable_widget::paintEvent(QPaintEvent* event)
{
   Q_UNUSED(event)

   QStyleOption opt;
   opt.init(this);
   QPainter p(this);
   style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
}
