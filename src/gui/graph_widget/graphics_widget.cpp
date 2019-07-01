#include "graph_widget/graphics_widget.h"

#include "graph_widget/graph_graphics_view.h"
#include "graph_widget/graphics_scene.h"
#include "gui_globals.h"

#include <QGraphicsScene>
#include <QHBoxLayout>
#include <qmath.h>
#include <QSlider>

graphics_widget::graphics_widget(QWidget* parent) : QFrame(parent),
    m_layout(new QHBoxLayout(this)),
    m_view(new graph_graphics_view(this)),
    m_zoom_slider(new QSlider(this))
{
    connect(m_view, &graph_graphics_view::zoomed_in, this, &graphics_widget::zoom_in);
    connect(m_view, &graph_graphics_view::zoomed_out, this, &graphics_widget::zoom_out);
    connect(m_zoom_slider, &QSlider::valueChanged, this, &graphics_widget::update_matrix);

    m_layout->setContentsMargins(0, 0, 0, 0);

    m_view->setFrameStyle(QFrame::NoFrame);
    m_view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    m_view->setRenderHint(QPainter::Antialiasing, false);
    m_view->setDragMode(QGraphicsView::RubberBandDrag);

    m_zoom_slider->setMinimum(0);
    m_zoom_slider->setMaximum(1000);

    m_layout->addWidget(m_view);
    m_layout->addWidget(m_zoom_slider);

    update_matrix();
}

graph_graphics_view* graphics_widget::view() const
{
    return m_view;
}

void graphics_widget::zoom_in(int level)
{
    m_zoom_slider->setValue(m_zoom_slider->value() + level);
}

void graphics_widget::zoom_out(int level)
{
    m_zoom_slider->setValue(m_zoom_slider->value() - level);
}

void graphics_widget::update_matrix()
{
    qreal scale = qPow(qreal(2), (m_zoom_slider->value() -500) / qreal(100));

    QMatrix matrix;
    matrix.scale(scale, scale);
    m_view->setMatrix(matrix);
}
