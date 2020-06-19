#pragma once

#include <QFrame>

class QKeyEvent;

namespace hal{

class hal_window_layout_container : public QFrame
{
    Q_OBJECT

public:
    explicit hal_window_layout_container(QWidget* parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent* event);
};
}
