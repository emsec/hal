#include "gui/graph_widget/layouters/debug_layouter.h"

#include <QSet>
#include <QTime>

debug_layouter::debug_layouter(const graph_context* const context) : graph_layouter(context)
{

}

const QString debug_layouter::name() const
{
    return "Debug Layouter";
}

const QString debug_layouter::description() const
{
    return "For testing purposes";
}

//void debug_layouter::layout()
//{
//    // ARTIFICIAL LAG FOR TESTING PURPOSES
//    QTime time = QTime::currentTime().addSecs(10);
//    while (QTime::currentTime() < time)
//    {
//        Q_EMIT status_update("I'm doing stuff (~￣▽￣)~");
//    }
//}

void debug_layouter::add(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets)
{
    Q_UNUSED(modules)
    Q_UNUSED(gates)
    Q_UNUSED(nets)
}

void debug_layouter::remove(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets)
{
    Q_UNUSED(modules)
    Q_UNUSED(gates)
    Q_UNUSED(nets)
}
