#include "gui/plugin_management/no_scheduled_plugins_widget.h"

#include "gui/file_manager/file_manager.h"
#include "gui/gui_utils/graphics.h"

#include <QDragEnterEvent>
#include <QLabel>
#include <QMimeData>
#include <QStyle>
#include <QVBoxLayout>

namespace hal
{
    NoScheduledPluginsWidget::NoScheduledPluginsWidget(QWidget* parent) : QFrame(parent), mLayout(new QVBoxLayout()), mTextLabel(new QLabel()), mIconLabel(new QLabel()), mDragActive(false)
    {
        setAcceptDrops(true);

        mTextLabel->setObjectName("text-label");
        mTextLabel->setText("Drag & drop plugins here to create a new schedule, select a scheduled plugin to edit its arguments");
        mTextLabel->setWordWrap(true);

        mIconLabel->setObjectName("icon-label");
        mIconLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        mIconLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        setLayout(mLayout);
        mLayout->setContentsMargins(0, 0, 0, 0);
        mLayout->setSpacing(0);
        mLayout->addWidget(mTextLabel);
        mLayout->addWidget(mIconLabel);
    }

    void NoScheduledPluginsWidget::dragEnterEvent(QDragEnterEvent* event)
    {
        const QMimeData* mime_data = event->mimeData();
        QString name               = QString(mime_data->data("hal/plugin_name"));
        if (name.isEmpty())
            return;

        event->acceptProposedAction();
        mDragActive = true;
        repolish();
    }

    void NoScheduledPluginsWidget::dragLeaveEvent(QDragLeaveEvent* event)
    {
        Q_UNUSED(event)

        mDragActive = false;
        repolish();
    }

    void NoScheduledPluginsWidget::dropEvent(QDropEvent* event)
    {
        const QMimeData* mime_data = event->mimeData();
        QString name               = QString(mime_data->data("hal/plugin_name"));
        if (name.isEmpty())
            return;

        event->acceptProposedAction();
        mDragActive = false;
        repolish();

        hide();
        Q_EMIT appendPlugin(name);
    }

    bool NoScheduledPluginsWidget::dragActive()
    {
        return mDragActive;
    }

    QString NoScheduledPluginsWidget::iconStyle()
    {
        return mIconStyle;
    }

    QString NoScheduledPluginsWidget::iconPath()
    {
        return mIconPath;
    }

    void NoScheduledPluginsWidget::setIconStyle(const QString& style)
    {
        mIconStyle = style;
    }

    void NoScheduledPluginsWidget::setIconPath(const QString& path)
    {
        mIconPath = path;
    }

    void NoScheduledPluginsWidget::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        if (!mIconPath.isEmpty())
            mIconLabel->setPixmap(gui_utility::getStyledSvgIcon(mIconStyle, mIconPath).pixmap(QSize(120, 120)));
    }
}
