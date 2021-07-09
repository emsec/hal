//#include "gui/graph_widget/graph_layout_spinner_widget.h"
#include "gui/spinner_widget/spinner_widget.h"
#include <QPainter>

namespace hal
{
    SpinnerWidget::SpinnerWidget(QWidget* parent) : QWidget(parent),
        mRenderer(new QSvgRenderer())
    {
        const QString string(":/images/spinner");
        mRenderer->load(string);
        mRenderer->setFramesPerSecond(10);
        connect(mRenderer, &QSvgRenderer::repaintNeeded, this, &SpinnerWidget::handleRepaintNeeded);
    }

    void SpinnerWidget::handleRepaintNeeded()
    {
        update();
    }

    void SpinnerWidget::paintEvent(QPaintEvent* event)
    {
        QWidget::paintEvent(event);

        QPainter painter(this);

        mRenderer->render(&painter, rect());
    }

    QSize SpinnerWidget::sizeHint() const
    {
        //DEBUG FUNCTION, REPLACE WITH INTERNAL LAYOUT OR SOMETHING LATER
        return QSize(260, 260);
    }

    void SpinnerWidget::start()
    {
        mRenderer->setFramesPerSecond(10);
    }

    void SpinnerWidget::stop()
    {
        mRenderer->setFramesPerSecond(0);
    }
}
