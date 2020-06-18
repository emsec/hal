#include "module_widget/module_tree_view.h"
#include <QMouseEvent>
namespace hal{
module_tree_view::module_tree_view(QWidget *parent) : QTreeView(parent)
{
}

void module_tree_view::mousePressEvent(QMouseEvent *event)
{
    if(event->button() != Qt::RightButton)
        QTreeView::mousePressEvent(event);
}
}
