#include "gui/thread_pool/thread_pool.h"

#include "gui/thread_pool/task.h"
#include "gui/thread_pool/worker.h"

namespace hal
{
    ThreadPool::ThreadPool(QObject* parent) : QObject(parent)
    {
        for (int i = 0; i < 4; ++i)
        {
            Worker* w = new Worker(this);
            connect(w, &Worker::finished, this, &ThreadPool::handleWorkerFinished);
            mFreeThreads.push(w);
        }
    }

    void ThreadPool::queueTask(Task* const t)
    {
        if (mFreeThreads.isEmpty())
            mTasks.enqueue(t);
        else
        {
            Worker* w = mFreeThreads.pop();
            w->assignTask(t);
            w->start();
        }
    }

    void ThreadPool::handleWorkerFinished()
    {
        Worker* w = static_cast<Worker*>(QObject::sender());

        if (mTasks.empty())
            mFreeThreads.push(w);
        else
        {
            Task* t = mTasks.dequeue();
            w->assignTask(t);
            w->start();
        }
    }
}
