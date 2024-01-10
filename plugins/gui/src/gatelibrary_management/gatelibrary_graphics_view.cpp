#include "gui/gatelibrary_management/gatelibrary_graphics_view.h"
#include "gui/graph_widget/graphics_factory.h"

namespace hal
{
    GatelibraryGraphicsView::GatelibraryGraphicsView(QWidget* parent)
        : QGraphicsView(parent)
    {;}

    void GatelibraryGraphicsView::wheelEvent(QWheelEvent *event)
    {
        float scalef = 1.;
        int dz = event->angleDelta().y();
        if (dz > 0)
            scalef *= 1.1;
        else if (dz < 0)
            scalef /= 1.1;
        else
            return;

        scale(scalef, scalef);
    }

    void GatelibraryGraphicsView::showGate(Gate *g)
    {
        GraphicsGate* gg = GraphicsFactory::createGraphicsGate(g,0);
        gg->setLod(1.0);
        QRectF rect = gg->boundingRect();
        scene()->clear();
        scene()->addItem(gg);
        rect.setTopLeft(rect.topLeft() + QPointF(-50,-50));
        rect.setBottomRight(rect.bottomRight() + QPointF(50,50));
        scene()->setSceneRect(rect);
    }
}
