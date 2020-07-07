#include "gui/graph_widget/graph_layout_spinner_widget.h"

#include <QPainter>

namespace hal
{
    GraphLayoutSpinnerWidget::GraphLayoutSpinnerWidget(QWidget* parent) : QWidget(parent),
        m_renderer(new QSvgRenderer())
    {
        const QString string(":/images/spinner");
        m_renderer->load(string);
        m_renderer->setFramesPerSecond(10);
        connect(m_renderer, &QSvgRenderer::repaintNeeded, this, &GraphLayoutSpinnerWidget::handle_repaint_needed);
    }

    void GraphLayoutSpinnerWidget::handle_repaint_needed()
    {
        update();
    }

    void GraphLayoutSpinnerWidget::paintEvent(QPaintEvent* event)
    {
        QWidget::paintEvent(event);

        QPainter painter(this);

        m_renderer->render(&painter, rect());
    }

    QSize GraphLayoutSpinnerWidget::sizeHint() const
    {
        //DEBUG FUNCTION, REPLACE WITH INTERNAL LAYOUT OR SOMETHING LATER
        return QSize(260, 260);
    }

    void GraphLayoutSpinnerWidget::start()
    {
        m_renderer->setFramesPerSecond(10);
    }

    void GraphLayoutSpinnerWidget::stop()
    {
        m_renderer->setFramesPerSecond(0);
    }
}
