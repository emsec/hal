#include "gui/thread_pool/worker.h"
#include <assert.h>
#include "gui/thread_pool/task.h"
namespace hal{
worker::worker(QObject* parent) : QThread(parent)
{

}

void worker::assign_task(task* const t)
{
    assert(t);

    t->moveToThread(this);
    m_task = t;
}

void worker::run()
{
    assert(m_task);

    m_task->execute();
    Q_EMIT m_task->finished();
    m_task->deleteLater();
}
}
