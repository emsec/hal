#include "gui/graph_widget/contexts/layouter_task.h"

#include "gui/graph_widget/layouters/graph_layouter.h"

#include <QApplication>

layouter_task::layouter_task(graph_layouter* const layouter) :
    m_layouter(layouter)
{
    m_layouter->setParent(this);
}

void layouter_task::execute()
{
    //QThread* const original_thread = m_layouter->thread();
    m_layouter->layout();
    //m_layouter->moveToThread(original_thread);
    m_layouter->setParent(nullptr);
    m_layouter->moveToThread(QApplication::instance()->thread());
}
