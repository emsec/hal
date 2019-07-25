#ifndef TASK_H
#define TASK_H

#include <QObject>

class task : public QObject
{
    Q_OBJECT

public:
    task();

    virtual void execute() = 0;

Q_SIGNALS:
    void finished();
};

#endif // TASK_H
