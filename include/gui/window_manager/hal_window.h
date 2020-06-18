#ifndef HAL_WINDOW_H
#define HAL_WINDOW_H

#include <QFrame>

class QAction;
class QVBoxLayout;

namespace hal{
class hal_window_layout_container;
class hal_window_toolbar;
class overlay;
class workspace; // TEMP NAME ?

class hal_window : public QFrame
{
    Q_OBJECT

public:
    explicit hal_window(QWidget* parent = nullptr);

    void lock();
    void unlock();

    void standard_view();
    void special_view(QWidget* widget);

    void repolish();

    hal_window_toolbar* get_toolbar();
    overlay* get_overlay();

protected:
    //void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
    //bool event(QEvent* event) Q_DECL_OVERRIDE;
    void closeEvent(QCloseEvent* event) Q_DECL_OVERRIDE;
//    void changeEvent(QEvent* event) Q_DECL_OVERRIDE;
//    void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

private:
    //void rearrange();

    QVBoxLayout* m_outer_layout;
    hal_window_layout_container* m_layout_container;
    QVBoxLayout* m_inner_layout;
    hal_window_toolbar* m_toolbar;
    workspace* m_workspace;

    overlay* m_overlay;
    QGraphicsEffect* m_effect;
};
}

#endif // HAL_WINDOW_H
