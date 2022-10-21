#include "gui/welcome_screen/recent_file_item.h"

#include "gui/file_manager/file_manager.h"
#include "gui/gui_utils/graphics.h"
#include "gui/user_action/action_open_netlist_file.h"

#include <QEvent>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QStyle>
#include <QVBoxLayout>
#include <QToolButton>

namespace hal
{
    RecentFileItem::RecentFileItem(const QString& file, QWidget* parent)
        : QFrame(parent), mWidget(new QWidget()), mHorizontalLayout(new QHBoxLayout()), mIconLabel(new QLabel()), mVerticalLayout(new QVBoxLayout()), mNameLabel(new QLabel()),
          mPathLabel(new QLabel()), mAnimation(new QPropertyAnimation()), mRemoveButton(new QToolButton(this)), mHover(false), mMissing(false),
          mMethod(ActionOpenNetlistFile::Undefined)
    {
        mHorizontalLayout->setContentsMargins(0, 0, 0, 0);
        mHorizontalLayout->setSpacing(0);

        mIconLabel->setObjectName("icon-label");
        mIconLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

        mVerticalLayout->setContentsMargins(0, 0, 0, 0);
        mVerticalLayout->setSpacing(0);

        mNameLabel->setObjectName("name-label");
        mPathLabel->setObjectName("path-label");


        mRemoveButton->setText("x");
        connect(mRemoveButton, &QToolButton::clicked, this, &RecentFileItem::handleCloseRequested);

        setLayout(mHorizontalLayout);
        mHorizontalLayout->addWidget(mIconLabel);
        mHorizontalLayout->setAlignment(mIconLabel, Qt::AlignTop);
        mHorizontalLayout->addLayout(mVerticalLayout);
        mHorizontalLayout->addWidget(mRemoveButton);
        mVerticalLayout->addWidget(mNameLabel);
        mVerticalLayout->addWidget(mPathLabel);

        mFile = file;
        QFileInfo info(file);
        mMethod = info.isDir() ? ActionOpenNetlistFile::OpenProject : ActionOpenNetlistFile::ImportFile;
        mNameLabel->setText(info.fileName());
        //mPath = info.canonicalPath();
        mPath = info.absolutePath();

        mPathLabel->ensurePolished();
        int width                 = mPathLabel->width();
        QFontMetrics font_metrics = mPathLabel->fontMetrics();
        mPathLabel->setText(font_metrics.elidedText(mPath, Qt::TextElideMode::ElideLeft, width));

        if (!info.exists())
            setMissing(true);

        repolish();
    }

    void RecentFileItem::enterEvent(QEvent* event)
    {
        Q_UNUSED(event)
        if(mMissing)
            return;

        mHover = true;
        repolish();
    }

    void RecentFileItem::leaveEvent(QEvent* event)
    {
        Q_UNUSED(event)
        if(mMissing)
            return;

        mHover = false;
        repolish();
    }

    void RecentFileItem::mousePressEvent(QMouseEvent* event)
    {
        Q_UNUSED(event)

        if(mMissing)
            return;

        if(event->button() == Qt::MouseButton::LeftButton)
        {
            ActionOpenNetlistFile* actOpenfile = new ActionOpenNetlistFile(mMethod,mFile);
            actOpenfile->exec();
            event->accept();
        }
    }

    QSize RecentFileItem::sizeHint() const
    {
        return mWidget->sizeHint();
    }

    bool RecentFileItem::eventFilter(QObject* object, QEvent* event)
    {
        if (object == mPathLabel && event->type() == QEvent::Resize)
        {
            QResizeEvent* resize_event = static_cast<QResizeEvent*>(event);
            int width                  = resize_event->size().width();

            mPathLabel->ensurePolished();
            QFontMetrics font_metrics = mPathLabel->fontMetrics();
            mPathLabel->setText(font_metrics.elidedText(mPath, Qt::TextElideMode::ElideLeft, width));
        }

        return false;
    }

    QString RecentFileItem::file() const
    {
        return mFile;
    }

    void RecentFileItem::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        s->unpolish(mIconLabel);
        s->polish(mIconLabel);

        s->unpolish(mNameLabel);
        s->polish(mNameLabel);

        s->unpolish(mPathLabel);
        s->polish(mPathLabel);

        if (!mIconPath.isEmpty())
            mIconLabel->setPixmap(gui_utility::getStyledSvgIcon(mIconStyle, mIconPath).pixmap(QSize(17, 17)));
    }

    bool RecentFileItem::hover() const
    {
        return mHover;
    }

    bool RecentFileItem::missing() const
    {
        return mMissing;
    }

    bool RecentFileItem::isProject() const
    {
        return mMethod == ActionOpenNetlistFile::OpenProject;
    }

    QString RecentFileItem::iconPath()
    {
        return mIconPath;
    }

    QString RecentFileItem::iconStyle()
    {
        return mIconStyle;
    }

    void RecentFileItem::setHoverActive(bool active)
    {
        mHover = active;
    }

    void RecentFileItem::setMissing(bool miss)
    {
        mMissing = miss;
        mNameLabel->setText(mNameLabel->text().append(" [Missing]"));
        mHover = false;
    }

    void RecentFileItem::setIconPath(const QString& path)
    {
        mIconPath = path;
    }

    void RecentFileItem::setIconStyle(const QString& style)
    {
        mIconStyle = style;
    }

    void RecentFileItem::handleCloseRequested()
    {
        Q_EMIT removeRequested(this);
    }
}
