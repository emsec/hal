#include "gui/graph_widget/graph_layout_spinner_widget.h"

#include <QPainter>

namespace hal
{
    GraphLayoutSpinnerWidget::GraphLayoutSpinnerWidget(QWidget* parent) : QWidget(parent),
        mRenderer(new QSvgRenderer())
    {
        const QString string(":/images/spinner");
        mRenderer->load(string);
        mRenderer->setFramesPerSecond(10);
        connect(mRenderer, &QSvgRenderer::repaintNeeded, this, &GraphLayoutSpinnerWidget::handleRepaintNeeded);
    }

    void GraphLayoutSpinnerWidget::handleRepaintNeeded()
    {
        update();
    }

    void GraphLayoutSpinnerWidget::paintEvent(QPaintEvent* event)
    {
        QWidget::paintEvent(event);

        QPainter painter(this);

        mRenderer->render(&painter, rect());
    }

    QSize GraphLayoutSpinnerWidget::sizeHint() const
    {
        //DEBUG FUNCTION, REPLACE WITH INTERNAL LAYOUT OR SOMETHING LATER
        return QSize(260, 260);
    }

    void GraphLayoutSpinnerWidget::start()
    {
        mRenderer->setFramesPerSecond(10);
    }

    void GraphLayoutSpinnerWidget::stop()
    {
        mRenderer->setFramesPerSecond(0);
    }
}
