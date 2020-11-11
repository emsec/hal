#include "gui/plugin_management/scheduled_plugin_item.h"

#include "gui/gui_utils/graphics.h"

#include <QApplication>
#include <QDrag>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMimeData>
#include <QMouseEvent>
#include <QStyle>

namespace hal
{
    bool ScheduledPluginItem::sDragInProgress = false;
    QPoint ScheduledPluginItem::sDragStartPosition;

    ScheduledPluginItem::ScheduledPluginItem(const QString& name, QWidget* parent) : QFrame(parent), mLayout(new QHBoxLayout()), mLabel(new QLabel()), mHover(false)
    {
        mName = name;

        mLayout->setContentsMargins(0, 0, 0, 0);
        mLayout->setSpacing(0);

        mLabel->setObjectName("name-label");
        mLabel->setText(name);

        setLayout(mLayout);
        mLayout->addWidget(mLabel);

        repolish();
    }

    void ScheduledPluginItem::enterEvent(QEvent* event)
    {
        if (event->type() == QEvent::Enter)
        {
            mHover = true;
            repolish();
        }
    }

    void ScheduledPluginItem::leaveEvent(QEvent* event)
    {
        if (event->type() == QEvent::Leave)
        {
            if (sDragInProgress)
                execDrag();

            mHover = false;
            repolish();
        }
    }

    void ScheduledPluginItem::mouseMoveEvent(QMouseEvent* event)
    {
        if (!sDragInProgress)
            return;
        if (!(event->buttons() & Qt::LeftButton))
            return;
        if ((event->pos() - sDragStartPosition).manhattanLength() < QApplication::startDragDistance())
            return;

        execDrag();
    }

    void ScheduledPluginItem::mousePressEvent(QMouseEvent* event)
    {
        if (event->button() == Qt::LeftButton)
        {
            sDragInProgress    = true;
            sDragStartPosition = event->pos();
        }

        event->accept();
        Q_EMIT clicked(this);
    }

    void ScheduledPluginItem::mouseReleaseEvent(QMouseEvent* event)
    {
        if (event->button() == Qt::LeftButton)
            sDragInProgress = false;

        event->accept();
    }

    void ScheduledPluginItem::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        s->unpolish(mLabel);
        s->polish(mLabel);
    }

    void ScheduledPluginItem::execDrag()
    {
        sDragInProgress   = false;
        QDrag* drag          = new QDrag(this);
        QMimeData* mime_data = new QMimeData();
        mime_data->setData("hal/plugin_name", mLabel->text().toUtf8());
        mime_data->setData("hal/item_height", QString::number(height()).toUtf8());
        drag->setMimeData(mime_data);
        drag->setPixmap(grab());
        drag->setHotSpot(QPoint(drag->pixmap().width() / 2, drag->pixmap().height() / 2));

        mHover = false;
        mLabel->setText(mName);
        repolish();

        Q_EMIT dragStarted(this);
        Qt::DropAction dropAction = drag->exec(Qt::MoveAction);

        if (!(dropAction & Qt::MoveAction))
            Q_EMIT removed(this);
    }

    bool ScheduledPluginItem::hover()
    {
        return mHover;
    }

    QString ScheduledPluginItem::iconPath()
    {
        return mIconPath;
    }

    QString ScheduledPluginItem::iconStyle()
    {
        return mIconStyle;
    }

    QString ScheduledPluginItem::name()
    {
        return mLabel->text();
    }

    void ScheduledPluginItem::setHoverActive(bool active)
    {
        mHover = active;
    }

    void ScheduledPluginItem::setIconPath(const QString& path)
    {
        mIconPath = path;
    }

    void ScheduledPluginItem::setIconStyle(const QString& style)
    {
        mIconStyle = style;
    }
}
