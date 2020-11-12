#include "gui/graphics/graphics_view.h"
#include <QOpenGLWidget>
#include <Qt>
namespace hal
{
    GraphicsView::GraphicsView()
    {
        QOpenGLWidget* viewport = new QOpenGLWidget();
        viewport->setUpdateBehavior(QOpenGLWidget::PartialUpdate);
        viewport->setAutoFillBackground(true);
        this->setViewport(viewport);
        //this->viewport()->installEventFilter(this);
        mModifiers        = Qt::NoModifier;
        mZoomFactorBase = 1.0020;
        //setAttribute(Qt::WA_TranslucentBackground);
    }

    void GraphicsView::gentleZoom(double factor)
    {
        this->scale(factor, factor);
        this->centerOn(mTargetScenePos);
        QPointF delta_viewport_pos = mTargetViewportPos - QPointF(this->viewport()->width() / 2.0, this->viewport()->height() / 2.0);
        QPointF viewport_center    = this->mapFromScene(mTargetScenePos) - delta_viewport_pos;
        this->centerOn(this->mapToScene(viewport_center.toPoint()));
        Q_EMIT zoomed();
    }

    void GraphicsView::setModifiers(Qt::KeyboardModifiers modifiers)
    {
        mModifiers = modifiers;
    }

    void GraphicsView::setZoomFactorBase(double value)
    {
        mZoomFactorBase = value;
    }

    bool GraphicsView::eventFilter(QObject* object, QEvent* event)
    {
        Q_UNUSED(object)
        if (event->type() == QEvent::Wheel)
        {
            QWheelEvent* wheel_event = static_cast<QWheelEvent*>(event);
            if (QApplication::keyboardModifiers() == mModifiers)
            {
                if (wheel_event->orientation() == Qt::Vertical)
                {
                    mTargetViewportPos = wheel_event->pos();
                    mTargetScenePos    = this->mapToScene(wheel_event->pos());
                    double angle        = wheel_event->angleDelta().y();
                    double factor       = qPow(mZoomFactorBase, angle);
                    gentleZoom(factor);
                    return true;
                }
            }
        }
        return false;
    }
}
