#include "gui/plugin_management/loaded_plugin_item.h"

#include "gui/gui_utils/graphics.h"

#include <QApplication>
#include <QDrag>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMimeData>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QStyle>
#include <QVBoxLayout>

namespace hal
{
    bool LoadedPluginItem::sDragInProgress = false;
    QPoint LoadedPluginItem::sDragStartPosition;

    LoadedPluginItem::LoadedPluginItem(const QString& name, QWidget* parent)
        : QFrame(parent), mHorizontalLayout(new QHBoxLayout()), mIconLabel(new QLabel()), mVerticalLayout(new QVBoxLayout()), mNameLabel(new QLabel()), mDescriptionLabel(new QLabel()),
          mAnimation(new QPropertyAnimation()), mHover(false)
    {
        mName = name;
        //    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        mHorizontalLayout->setContentsMargins(0, 0, 0, 0);
        mHorizontalLayout->setSpacing(0);

        //    mIconLabel->setObjectName("icon-label");
        //    mIconLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        //    mHorizontalLayout->addWidget(mIconLabel);
        //    mHorizontalLayout->setAlignment(mIconLabel, Qt::AlignTop);

        //    mVerticalLayout->setContentsMargins(0, 0, 0, 0);
        //    mVerticalLayout->setSpacing(0);

        mNameLabel->setObjectName("name-label");
        mNameLabel->setText(name);
        //    mDescriptionLabel->setObjectName("mDescription-label");
        //    mDescriptionLabel->setText(mDescription);
        //    mDescriptionLabel->setWordWrap(true);
        //    mVerticalLayout->addWidget(mDescriptionLabel);

        setLayout(mHorizontalLayout);
        mHorizontalLayout->addWidget(mNameLabel);

        repolish();
    }

    void LoadedPluginItem::enterEvent(QEvent* event)
    {
        if (event->type() == QEvent::Enter)
        {
            mHover = true;
            repolish();
        }
    }

    void LoadedPluginItem::leaveEvent(QEvent* event)
    {
        if (event->type() == QEvent::Leave)
        {
            if (sDragInProgress)
                execDrag();

            mHover = false;
            repolish();
        }
    }

    void LoadedPluginItem::mouseMoveEvent(QMouseEvent* event)
    {
        if (!sDragInProgress)
            return;
        if (!(event->buttons() & Qt::LeftButton))
            return;
        if ((event->pos() - sDragStartPosition).manhattanLength() < QApplication::startDragDistance())
            return;

        execDrag();
    }

    void LoadedPluginItem::mousePressEvent(QMouseEvent* event)
    {
        if (event->button() == Qt::LeftButton)
        {
            sDragInProgress    = true;
            sDragStartPosition = event->pos();
        }
        // SIGNAL CLICKED
        event->accept();
    }

    void LoadedPluginItem::mouseReleaseEvent(QMouseEvent* event)
    {
        if (event->button() == Qt::LeftButton)
            sDragInProgress = false;

        // SIGNAL CLICKED
        event->accept();
    }

    void LoadedPluginItem::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        s->unpolish(mIconLabel);
        s->polish(mIconLabel);

        s->unpolish(mNameLabel);
        s->polish(mNameLabel);

        s->unpolish(mDescriptionLabel);
        s->polish(mDescriptionLabel);

        if (!mIconPath.isEmpty())
            mIconLabel->setPixmap(gui_utility::getStyledSvgIcon(mIconStyle, mIconPath).pixmap(QSize(17, 17)));
    }

    void LoadedPluginItem::execDrag()
    {
        sDragInProgress   = false;
        QDrag* drag          = new QDrag(this);
        QMimeData* mime_data = new QMimeData();
        mime_data->setData("hal/plugin_name", mNameLabel->text().toUtf8());
        mime_data->setData("hal/item_height", QString::number(height()).toUtf8());
        drag->setMimeData(mime_data);
        drag->setPixmap(grab());
        drag->setHotSpot(QPoint(drag->pixmap().width() / 2, drag->pixmap().height() / 2));

        mHover = false;
        mNameLabel->setText("");
        repolish();
        drag->exec();
        mNameLabel->setText(mName);
    }

    bool LoadedPluginItem::hover()
    {
        return mHover;
    }

    QString LoadedPluginItem::iconPath()
    {
        return mIconPath;
    }

    QString LoadedPluginItem::iconStyle()
    {
        return mIconStyle;
    }

    QString LoadedPluginItem::name()
    {
        return mNameLabel->text();
    }

    void LoadedPluginItem::setHoverActive(bool active)
    {
        mHover = active;
    }

    void LoadedPluginItem::setIconPath(const QString& path)
    {
        mIconPath = path;
    }

    void LoadedPluginItem::setIconStyle(const QString& style)
    {
        mIconStyle = style;
    }
}
