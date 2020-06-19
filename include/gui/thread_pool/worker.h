#pragma once

#include <QThread>
namespace hal{
class task;

class worker : public QThread
{
    Q_OBJECT

public:
    worker(QObject* parent = nullptr);

    void assign_task(task* const t);

    void run() Q_DECL_OVERRIDE;

private:
    task* m_task;
};
}
