#include "gui/thread_pool/thread_pool.h"

#include "gui/thread_pool/task.h"
#include "gui/thread_pool/worker.h"

thread_pool::thread_pool(QObject* parent) : QObject(parent)
{
    for (int i = 0; i < 4; ++i)
    {
        worker* w = new worker(this);
        connect(w, &worker::finished, this, &thread_pool::handle_worker_finished);
        m_free_threads.push(w);
    }
}

void thread_pool::queue_task(task* const t)
{
    if (m_free_threads.isEmpty())
        m_tasks.enqueue(t);
    else
    {
        worker* w = m_free_threads.pop();
        w->assign_task(t);
        w->start();
    }
}

void thread_pool::handle_worker_finished()
{
    worker* w = static_cast<worker*>(QObject::sender());

    if (m_tasks.empty())
        m_free_threads.push(w);
    else
    {
        task* t = m_tasks.dequeue();
        w->assign_task(t);
        w->start();
    }
}
