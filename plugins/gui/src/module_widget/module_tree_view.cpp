#include "gui/module_widget/module_tree_view.h"
#include <QMouseEvent>
#include <QHeaderView>

namespace hal
{
    ModuleTreeView::ModuleTreeView(QWidget *parent)
        : QTreeView(parent), mToggleStateExpanded(true)
    {
    }

    void ModuleTreeView::expandAllModules()
    {
        mToggleStateExpanded = true;
        expandAll();
    }

    void ModuleTreeView::collapseAllModules()
    {
        mToggleStateExpanded = false;
        collapseAll();
    }

    bool ModuleTreeView::toggleStateExpanded() const
    {
        return mToggleStateExpanded;
    }

    void ModuleTreeView::setDefaultColumnWidth()
    {
        setColumnWidth(0, 240);
        setColumnWidth(1, 40);
        setColumnWidth(2, 80);
        header()->setStretchLastSection(true);
    }

    void ModuleTreeView::mousePressEvent(QMouseEvent *event)
    {
        if(event->button() != Qt::RightButton)
            QTreeView::mousePressEvent(event);
    }
}
