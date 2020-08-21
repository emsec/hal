#include "module_widget/module_tree_view.h"
#include <QMouseEvent>

namespace hal
{
    ModuleTreeView::ModuleTreeView(QWidget *parent) : QTreeView(parent)
    {
    }

    void ModuleTreeView::mousePressEvent(QMouseEvent *event)
    {
        if(event->button() != Qt::RightButton)
            QTreeView::mousePressEvent(event);
    }
}
