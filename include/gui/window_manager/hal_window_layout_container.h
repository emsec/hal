#ifndef HAL_WINDOW_LAYOUT_CONTAINER_H
#define HAL_WINDOW_LAYOUT_CONTAINER_H

#include <QFrame>
namespace hal{
class QKeyEvent;

class hal_window_layout_container : public QFrame
{
    Q_OBJECT

public:
    explicit hal_window_layout_container(QWidget* parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent* event);
};
}

#endif // HAL_WINDOW_LAYOUT_CONTAINER_H
