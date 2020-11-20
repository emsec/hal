#include "gui/content_widget/content_widget.h"

#include "gui/gui_utils/graphics.h"
#include "gui/content_anchor/content_anchor.h"
#include "gui/toolbar/toolbar.h"

#include <QShortcut>
#include <QStyle>
#include <QVBoxLayout>

namespace hal
{
    ContentWidget::ContentWidget(QString name, QWidget* parent) : Widget(parent), mName(name), mContentLayout(new QVBoxLayout())
    {
        mContentLayout->setContentsMargins(0, 0, 0, 0);
        mContentLayout->setSpacing(0);
        setMinimumSize(100, 100);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setWindowTitle(name);

        setLayout(mContentLayout);
        //debug code, delete later
        QIcon temp;
        temp.addFile(QStringLiteral(":/icons/start"), QSize(), QIcon::Normal, QIcon::Off);
        setIcon(temp);
    }

    void ContentWidget::remove()
    {
        if (mAnchor)
        {
            mAnchor->remove(this);
            Q_EMIT removed();
        }
    }

    void ContentWidget::detach()
    {
        if (mAnchor)
        {
            mAnchor->detach(this);
            Q_EMIT detached();
        }
    }

    void ContentWidget::reattach()
    {
        if (mAnchor)
        {
            mAnchor->reattach(this);
            Q_EMIT reattached();
        }
    }

    void ContentWidget::open()
    {
        if (mAnchor)
        {
            mAnchor->open(this);
            Q_EMIT opened();
        }
    }

    void ContentWidget::close()
    {
        if (mAnchor)
        {
            mAnchor->close(this);
            Q_EMIT closed();
        }
    }

    void ContentWidget::closeEvent(QCloseEvent* event){Q_UNUSED(event)}

    QString ContentWidget::name()
    {
        return mName;
    }

    QIcon ContentWidget::icon()
    {
        return mIcon;
    }

    void ContentWidget::setAnchor(ContentAnchor* anchor)
    {
        mAnchor = anchor;
    }

    void ContentWidget::setIcon(QIcon icon)
    {
        mIcon = icon;
    }

    void ContentWidget::set_name(const QString &name)
    {
        mName = name;
        Q_EMIT name_changed(name);
    }

    void ContentWidget::setupToolbar(Toolbar* Toolbar){Q_UNUSED(Toolbar)}

    QList<QShortcut*> ContentWidget::createShortcuts()
    {
        return QList<QShortcut*>();
    }

    void ContentWidget::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        //    if (!mIconPath.isEmpty())
        //        mIconLabel->setPixmap(gui_utility::getStyledSvgIcon(mIconStyle, mIconPath).pixmap(QSize(20, 20)));
    }

    QString ContentWidget::iconStyle()
    {
        return mIconStyle;
    }

    QString ContentWidget::iconPath()
    {
        return mIconPath;
    }

    void ContentWidget::setIconStyle(const QString& style)
    {
        if (mIconStyle == style)
            return;

        mIconStyle = style;
        repolish();
    }

    void ContentWidget::setIconPath(const QString& path)
    {
        if (mIconPath == path)
            return;

        mIconPath = path;
        repolish();
    }
}
