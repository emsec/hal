#include "hal_graphics/hal_graphics_view.h"
#include <QOpenGLWidget>
#include <Qt>
namespace hal{
hal_graphics_view::hal_graphics_view()
{
    QOpenGLWidget* viewport = new QOpenGLWidget();
    viewport->setUpdateBehavior(QOpenGLWidget::PartialUpdate);
    viewport->setAutoFillBackground(true);
    this->setViewport(viewport);
    //this->viewport()->installEventFilter(this);
    _modifiers        = Qt::NoModifier;
    _zoom_factor_base = 1.0020;
    //setAttribute(Qt::WA_TranslucentBackground);
}

void hal_graphics_view::gentle_zoom(double factor)
{
    this->scale(factor, factor);
    this->centerOn(target_scene_pos);
    QPointF delta_viewport_pos = target_viewport_pos - QPointF(this->viewport()->width() / 2.0, this->viewport()->height() / 2.0);
    QPointF viewport_center    = this->mapFromScene(target_scene_pos) - delta_viewport_pos;
    this->centerOn(this->mapToScene(viewport_center.toPoint()));
    Q_EMIT zoomed();
}

void hal_graphics_view::set_modifiers(Qt::KeyboardModifiers modifiers)
{
    _modifiers = modifiers;
}

void hal_graphics_view::set_zoom_factor_base(double value)
{
    _zoom_factor_base = value;
}

bool hal_graphics_view::eventFilter(QObject* object, QEvent* event)
{
    Q_UNUSED(object)
    if (event->type() == QEvent::Wheel)
    {
        QWheelEvent* wheel_event = static_cast<QWheelEvent*>(event);
        if (QApplication::keyboardModifiers() == _modifiers)
        {
            if (wheel_event->orientation() == Qt::Vertical)
            {
                target_viewport_pos = wheel_event->pos();
                target_scene_pos    = this->mapToScene(wheel_event->pos());
                double angle        = wheel_event->angleDelta().y();
                double factor       = qPow(_zoom_factor_base, angle);
                gentle_zoom(factor);
                return true;
            }
        }
    }
    return false;
}
}
