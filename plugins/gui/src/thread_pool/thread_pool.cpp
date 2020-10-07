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
            connect(w, &Worker::finished, this, &ThreadPool::handle_worker_finished);
            m_free_threads.push(w);
        }
    }

    void ThreadPool::queue_task(Task* const t)
    {
        if (m_free_threads.isEmpty())
            m_tasks.enqueue(t);
        else
        {
            Worker* w = m_free_threads.pop();
            w->assign_task(t);
            w->start();
        }
    }

    void ThreadPool::handle_worker_finished()
    {
        Worker* w = static_cast<Worker*>(QObject::sender());

        if (m_tasks.empty())
            m_free_threads.push(w);
        else
        {
            Task* t = m_tasks.dequeue();
            w->assign_task(t);
            w->start();
        }
    }
}
