#include "gui/graph_widget/layouters/layouter_task.h"

#include "gui/graph_widget/layouters/graph_layouter.h"

#include <QApplication>

namespace hal
{
    LayouterTask::LayouterTask(GraphLayouter* const layouter) :
        m_layouter(layouter)
    {
        m_layouter->setParent(this);
    }

    void LayouterTask::execute()
    {
        m_layouter->layout();
        m_layouter->setParent(nullptr);
        m_layouter->moveToThread(QApplication::instance()->thread());
    }
}
