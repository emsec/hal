#include "overlay/overlay.h"

#include <QEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QStyleOption>

overlay::overlay(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    //setAttribute(Qt::WA_TransparentForMouseEvents);
    handle_parent_changed();
}

void overlay::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

bool overlay::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == parent())
    {
        if (event->type() == QEvent::Resize)
            resize(static_cast<QResizeEvent*>(event)->size());
        else if (event->type() == QEvent::ChildAdded)
            raise();
    }
    return QWidget::eventFilter(watched, event);
}

bool overlay::event(QEvent* event)
{
    if (event->type() == QEvent::ParentAboutToChange)
    {
        if (parent())
            parent()->removeEventFilter(this);
    }
    else if (event->type() == QEvent::ParentChange)
        handle_parent_changed();
    return QWidget::event(event);
}

void overlay::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event)

    Q_EMIT clicked();
    event->accept();
}

void overlay::handle_parent_changed()
{
    if (!parent())
        return;

    parent()->installEventFilter(this);
    raise();
}
