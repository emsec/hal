#include "gui/graph_widget/layouters/layouter_task.h"

#include "gui/graph_widget/layouters/graph_layouter.h"

#include <QApplication>

namespace hal
{
    LayouterTask::LayouterTask(GraphLayouter* const layouter) :
        mLayouter(layouter)
    {
        mLayouter->setParent(this);
    }

    void LayouterTask::execute()
    {
        mLayouter->layout();
        mLayouter->setParent(nullptr);
        mLayouter->moveToThread(QApplication::instance()->thread());
    }
}
