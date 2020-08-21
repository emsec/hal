#include "graphics/graphics_view.h"
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
        m_modifiers        = Qt::NoModifier;
        m_zoom_factor_base = 1.0020;
        //setAttribute(Qt::WA_TranslucentBackground);
    }

    void GraphicsView::gentle_zoom(double factor)
    {
        this->scale(factor, factor);
        this->centerOn(m_target_scene_pos);
        QPointF delta_viewport_pos = m_target_viewport_pos - QPointF(this->viewport()->width() / 2.0, this->viewport()->height() / 2.0);
        QPointF viewport_center    = this->mapFromScene(m_target_scene_pos) - delta_viewport_pos;
        this->centerOn(this->mapToScene(viewport_center.toPoint()));
        Q_EMIT zoomed();
    }

    void GraphicsView::set_modifiers(Qt::KeyboardModifiers modifiers)
    {
        m_modifiers = modifiers;
    }

    void GraphicsView::set_zoom_factor_base(double value)
    {
        m_zoom_factor_base = value;
    }

    bool GraphicsView::eventFilter(QObject* object, QEvent* event)
    {
        Q_UNUSED(object)
        if (event->type() == QEvent::Wheel)
        {
            QWheelEvent* wheel_event = static_cast<QWheelEvent*>(event);
            if (QApplication::keyboardModifiers() == m_modifiers)
            {
                if (wheel_event->orientation() == Qt::Vertical)
                {
                    m_target_viewport_pos = wheel_event->pos();
                    m_target_scene_pos    = this->mapToScene(wheel_event->pos());
                    double angle        = wheel_event->angleDelta().y();
                    double factor       = qPow(m_zoom_factor_base, angle);
                    gentle_zoom(factor);
                    return true;
                }
            }
        }
        return false;
    }
}
