#include "gui/thread_pool/worker.h"
#include <assert.h>
#include "gui/thread_pool/task.h"

namespace hal
{
    Worker::Worker(QObject* parent) : QThread(parent)
    {

    }

    void Worker::assign_task(Task* const t)
    {
        assert(t);

        t->moveToThread(this);
        m_task = t;
    }

    void Worker::run()
    {
        assert(m_task);

        m_task->execute();
        Q_EMIT m_task->finished();
        m_task->deleteLater();
    }
}
