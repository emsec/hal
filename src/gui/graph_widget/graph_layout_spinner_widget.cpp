#include "gui/graph_widget/graph_layout_spinner_widget.h"

#include <QPainter>
namespace hal{
graph_layout_spinner_widget::graph_layout_spinner_widget(QWidget* parent) : QWidget(parent),
    m_renderer(new QSvgRenderer())
{
    const QString string(":/images/spinner");
    m_renderer->load(string);
    m_renderer->setFramesPerSecond(10);
    connect(m_renderer, &QSvgRenderer::repaintNeeded, this, &graph_layout_spinner_widget::handle_repaint_needed);
}

void graph_layout_spinner_widget::handle_repaint_needed()
{
    update();
}

void graph_layout_spinner_widget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);

    m_renderer->render(&painter, rect());
}

QSize graph_layout_spinner_widget::sizeHint() const
{
    //DEBUG FUNCTION, REPLACE WITH INTERNAL LAYOUT OR SOMETHING LATER
    return QSize(260, 260);
}

void graph_layout_spinner_widget::start()
{
    m_renderer->setFramesPerSecond(10);
}

void graph_layout_spinner_widget::stop()
{
    m_renderer->setFramesPerSecond(0);
}
}
