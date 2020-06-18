#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <QFrame>
namespace hal{
class workspace : public QFrame
{
    Q_OBJECT

public:
    explicit workspace(QWidget* parent = nullptr);

    void repolish();
};
}

#endif // WORKSPACE_H
