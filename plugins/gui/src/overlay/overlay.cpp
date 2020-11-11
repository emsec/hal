#include "gui/overlay/overlay.h"

#include <QEvent>
#include <QResizeEvent>

namespace hal
{
    Overlay::Overlay(QWidget* parent) : QFrame(parent)
    {
    //    setAttribute(Qt::WA_NoSystemBackground);
    //    setAttribute(Qt::WA_TranslucentBackground);
        //setAttribute(Qt::WA_TransparentForMouseEvents);
        handleParentChanged();
    }

    bool Overlay::eventFilter(QObject* watched, QEvent* event)
    {
        if (watched == parent())
        {
            if (event->type() == QEvent::Resize)
                resize(static_cast<QResizeEvent*>(event)->size());
            else if (event->type() == QEvent::ChildAdded)
                raise();
        }

        // PASS UNHANDLED EVENTS TO BASE CLASS
        return QFrame::eventFilter(watched, event);
    }

    bool Overlay::event(QEvent* event)
    {
        if (event->type() == QEvent::ParentAboutToChange)
        {
            if (parent())
            {
                parent()->removeEventFilter(this);
                return true;
            }
        }
        else if (event->type() == QEvent::ParentChange)
        {
            handleParentChanged();
            return true;
        }

        // PASS UNHANDLED EVENTS TO BASE CLASS
        return QFrame::event(event);
    }

    void Overlay::mousePressEvent(QMouseEvent* event)
    {
        Q_EMIT clicked();
        event->accept(); // ACCEPT EXPLICITLY
    }

    void Overlay::handleParentChanged()
    {
        if (!parent())
            return;

        parent()->installEventFilter(this);
        raise();

        QWidget* w = qobject_cast<QWidget*>(parent());

        if (w)
            resize(w->size());
    }
}
