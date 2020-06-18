#ifndef HAL_WINDOW_TOOLBAR_H
#define HAL_WINDOW_TOOLBAR_H

#include <QFrame>

class QActionEvent;
class QHBoxLayout;

namespace hal{

class hal_window_toolbar : public QFrame
{
    Q_OBJECT

public:
    explicit hal_window_toolbar(QWidget* parent = nullptr);

    void add_widget(QWidget* widget);
    void add_spacer();
    void clear();

    void repolish();

protected:
    void actionEvent(QActionEvent* event) Q_DECL_OVERRIDE;

private:
    QHBoxLayout* m_layout;
};
}

#endif // HAL_WINDOW_TOOLBAR_H
