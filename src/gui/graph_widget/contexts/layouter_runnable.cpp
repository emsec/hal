#include "gui/graph_widget/contexts/layouter_runnable.h"

#include "gui/graph_widget/layouters/graph_layouter.h"

#include <QApplication>

layouter_runnable::layouter_runnable(graph_layouter* const layouter) : QObject(), QRunnable(),
    m_layouter(layouter)
{

}

void layouter_runnable::run()
{
    //QThread* const original_thread = m_layouter->thread();
    m_layouter->moveToThread(thread());
    m_layouter->layout();
    //m_layouter->moveToThread(original_thread);
    m_layouter->moveToThread(QApplication::instance()->thread());
}
