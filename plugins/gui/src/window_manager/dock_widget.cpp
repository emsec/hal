#include "gui/window_manager/dock_widget.h"

#include <QApplication>
#include <QMouseEvent>

namespace hal
{
    DockWidget::DockWidget(QWidget* parent) : QFrame(parent),
        m_drag_state(nullptr)
    {

    }

    void DockWidget::mousePressEvent(QMouseEvent* event)
    {
        // CHECK IF PRESS IS INSIDE THE RIGHT AREA
            init_drag(event->pos(), false);

    //        if (state)
    //            state->ctrlDrag = event->modifiers() & Qt::ControlModifier;
    //        return true;

    //    return false;
    }

    void DockWidget::mouseMoveEvent(QMouseEvent* event)
    {
        if (!m_drag_state)
            return;


        if (!m_drag_state->dragging /*& mwlayout->pluggingWidget == 0*/ && (event->pos() - m_drag_state->pressPos).manhattanLength() > QApplication::startDragDistance())
        {
            start_drag(false);

            grabMouse();
            return;
        }
        //    if (state->dragging && !state->nca)
        //    {
        //        QMargins windowMargins = q->window()->windowHandle()->frameMargins();
        //        QPoint windowMarginOffset = QPoint(windowMargins.left(), windowMargins.top());
        //        QPoint pos = event->globalPos() - state->pressPos - windowMarginOffset;
        //        QDockWidgetGroupWindow *floatingTab = qobject_cast<QDockWidgetGroupWindow*>(parent);
        //        if (floatingTab && !q->isFloating())
        //            floatingTab->move(pos);
        //        else
        //            q->move(pos);
        //        if (state && !state->ctrlDrag)
        //            mwlayout->hover(state->widgetItem, event->globalPos());
        //        ret = true;
        //    }
        //    return ret;
    }

    void DockWidget::mouseReleaseEvent(QMouseEvent* event)
    {
        if (event->button() == Qt::LeftButton && m_drag_state && !m_drag_state->nca)
        {
            end_drag(false);
            return; //filter out the event
        }
        event->ignore();
    }

    void DockWidget::init_drag(const QPoint& pos, bool nca)
    {
        if (m_drag_state)
            return;

        // IF DOCKING ANIMATION IN PROGRESS RETURN HERE

        m_drag_state = new drag_state;
        m_drag_state->pressPos = pos;
        m_drag_state->dragging = false;
        //m_drag_state->widgetItem = 0;
        m_drag_state->ownWidgetItem = false;
        m_drag_state->nca = nca;
        m_drag_state->ctrlDrag = false;

        // IS STATE SUPPOSED TO BE STATIC ???
    }

    void DockWidget::start_drag(bool group)
    {
        Q_UNUSED(group);
        if (!m_drag_state || m_drag_state->dragging)
            return;


    //    state->widgetItem = layout->unplug(q, group);
    //    if (state->widgetItem == 0)
    //    {
    //            state->widgetItem = new QDockWidgetItem(q);
    //        state->ownWidgetItem = true;
    //    }
    //    if (state->ctrlDrag)
    //        layout->restore();

        m_drag_state->dragging = true;
    }

    void DockWidget::end_drag(bool abort)
    {
        Q_UNUSED(abort);
        releaseMouse();

    //    if (m_drag_state->dragging)
    //    {
    //        QMainWindowLayout *mwLayout = qt_mainwindow_layout_from_dock(q);
    //        Q_ASSERT(mwLayout != 0);

    //        if (abort || !mwLayout->plug(state->widgetItem))
    //        {
    //            if (hasFeature(this, QDockWidget::DockWidgetFloatable))
    //            {
    //                // This QDockWidget will now stay in the floating state.
    //                if (state->ownWidgetItem)
    //                {
    //                    delete state->widgetItem;
    //                    state->widgetItem = nullptr;
    //                }
    //                mwLayout->restore();
    //                QDockWidgetLayout *dwLayout = qobject_cast<QDockWidgetLayout*>(layout);
    //                if (!dwLayout->nativeWindowDeco())
    //                {
    //                    // get rid of the X11BypassWindowManager window flag and activate the resizer
    //                    Qt::WindowFlags flags = q->windowFlags();
    //                    flags &= ~Qt::X11BypassWindowManagerHint;
    //                    q->setWindowFlags(flags);
    //                    setResizerActive(q->isFloating());
    //                    q->show();
    //                }
    //                else
    //                {
    //                    setResizerActive(false);
    //                }
    //                if (q->isFloating()) // Might not be floating when dragging a QDockWidgetGroupWindow
    //                    undockedGeometry = q->geometry();
    //                q->activateWindow();
    //            }
    //            else
    //            {
    //                // The tab was not plugged back in the QMainWindow but the QDockWidget cannot
    //                // stay floating, revert to the previous state.
    //                mwLayout->revert(state->widgetItem);
    //            }
    //        }
    //    }
        delete m_drag_state;
        m_drag_state = nullptr;
    }
}
