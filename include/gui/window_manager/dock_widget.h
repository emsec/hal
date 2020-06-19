#pragma once

#include <QFrame>
namespace hal{
class dock_widget : public QFrame
{
    Q_OBJECT

    struct drag_state
    {
            QPoint pressPos;
            bool dragging;
            //QLayoutItem *widgetItem;
            bool ownWidgetItem;
            bool nca;
            bool ctrlDrag;
    };

public:
    explicit dock_widget(QWidget* parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

private:
    void init_drag(const QPoint& pos, bool nca);
    void start_drag(bool group);
    void end_drag(bool abort);

    drag_state* m_drag_state;
};
}
