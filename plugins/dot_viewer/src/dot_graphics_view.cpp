#include "dot_viewer/dot_graphics_view.h"
#include "gui/gui_globals.h"
#include "gui/settings/settings_items/settings_item_checkbox.h"
#include <qmath.h>
#include <QScrollBar>

namespace hal {
    DotGraphicsView::DotGraphicsView(QWidget* parent)
        : QGraphicsView(parent), mPanModifier(Qt::KeyboardModifier::ShiftModifier)
    {;}

    void DotGraphicsView::wheelEvent(QWheelEvent* event)
    {
        qreal scaleFactor = qPow(2.0, event->angleDelta().y() / 240.0); //How fast we zoom
        scaleWithinLimits(scaleFactor);

    }

    void DotGraphicsView::handleZoomInShortcut()
    {
        scaleWithinLimits(1.2);
    }

    void DotGraphicsView::handleZoomOutShortcut()
    {
        scaleWithinLimits(0.8);
    }

    void DotGraphicsView::scaleWithinLimits(qreal scaleFactor)
    {
        qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
        if(0.1 < factor && factor < 100) scale(scaleFactor, scaleFactor);
    }

    void DotGraphicsView::mousePressEvent(QMouseEvent* event)
    {
        if ((event->modifiers() == mPanModifier && event->button() == Qt::LeftButton) ||
            (event->button() == Qt::MidButton && gGraphContextManager->sSettingPanOnMiddleButton->value().toBool()))
        {
            mMovePosition = event->pos();
        }
    }

    void DotGraphicsView::mouseMoveEvent(QMouseEvent* event)
    {
        if (!scene())
            return;

        if ((event->buttons().testFlag(Qt::LeftButton) && event->modifiers() == mPanModifier) ||
            (event->buttons().testFlag(Qt::MidButton) && gGraphContextManager->sSettingPanOnMiddleButton->value().toBool()))
        {
            QScrollBar* hBar  = horizontalScrollBar();
            QScrollBar* vBar  = verticalScrollBar();
            QPoint delta_move = event->pos() - mMovePosition;
            mMovePosition   = event->pos();
            if (hBar) hBar->setValue(hBar->value() + (isRightToLeft() ? delta_move.x() : -delta_move.x()));
            if (vBar) vBar->setValue(vBar->value() - delta_move.y());
        }
    }
}
