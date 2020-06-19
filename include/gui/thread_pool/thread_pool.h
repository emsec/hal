#pragma once

#include <QObject>
#include <QQueue>
#include <QStack>
namespace hal{
class task;
class worker;

class thread_pool : public QObject
{
    Q_OBJECT

public:
    explicit thread_pool(QObject* parent = nullptr);

    void queue_task(task* const t);

private Q_SLOTS:
    void handle_worker_finished();

private:
    QStack<worker*> m_free_threads;
    QQueue<task*> m_tasks;
};
}
