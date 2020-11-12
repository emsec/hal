#include "gui/thread_pool/worker.h"
#include <assert.h>
#include "gui/thread_pool/task.h"

namespace hal
{
    Worker::Worker(QObject* parent) : QThread(parent)
    {

    }

    void Worker::assignTask(Task* const t)
    {
        assert(t);

        t->moveToThread(this);
        mTask = t;
    }

    void Worker::run()
    {
        assert(mTask);

        mTask->execute();
        Q_EMIT mTask->finished();
        mTask->deleteLater();
    }
}
