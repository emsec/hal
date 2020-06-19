#pragma once

#include <QObject>
namespace hal{
class task : public QObject
{
    Q_OBJECT

public:
    task();

    virtual void execute() = 0;

Q_SIGNALS:
    void finished();
};
}
