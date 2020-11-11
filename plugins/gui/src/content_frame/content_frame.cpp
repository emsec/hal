#include "gui/content_frame/content_frame.h"

#include "gui/content_widget/content_widget.h"
#include "gui/gui_utils/graphics.h"
#include "gui/toolbar/toolbar.h"

#include <QAction>
#include <QChildEvent>
#include <QShortcut>

namespace hal
{
    ContentFrame::ContentFrame(ContentWidget* widget, bool attached, QWidget* parent)
        : QWidget(parent), mVerticalLayout(new QVBoxLayout()), mHorizontalLayout(new QHBoxLayout()), mLeftToolbar(new Toolbar()), mRightToolbar(new Toolbar()), mWidget(widget),
          mNameLabel(new QLabel())

    {
        setWindowTitle(widget->name());
        setFocusPolicy(Qt::FocusPolicy::StrongFocus);

        mVerticalLayout->setContentsMargins(0, 0, 0, 0);
        mVerticalLayout->setSpacing(0);

        mHorizontalLayout->setContentsMargins(0, 0, 0, 0);
        mHorizontalLayout->setSpacing(0);

        mLeftToolbar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        mLeftToolbar->setIconSize(QSize(18, 18));
        mRightToolbar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        mRightToolbar->setIconSize(QSize(18, 18));

        QAction* action = new QAction(this);

        mDetachIconStyle = "all->#969696";
        mDetachIconPath  = ":/icons/detach";

        if (attached)
        {
            mNameLabel->setText(widget->name());
            mLeftToolbar->addWidget(mNameLabel);
            mLeftToolbar->addSeparator();
            action->setText("Detach");
            action->setIcon(gui_utility::getStyledSvgIcon(mDetachIconStyle, mDetachIconPath));
            connect(action, &QAction::triggered, this, &ContentFrame::detachWidget);
        }
        else
        {
            action->setText("Reattach");
            connect(action, &QAction::triggered, this, &ContentFrame::reattachWidget);
        }
        widget->setupToolbar(mLeftToolbar);
        mRightToolbar->addAction(action);

        for (QShortcut* s : widget->createShortcuts())
        {
            s->setParent(this);
            s->setContext(Qt::WidgetWithChildrenShortcut);
            s->setEnabled(true);
        }

        setLayout(mVerticalLayout);
        mVerticalLayout->addLayout(mHorizontalLayout, Qt::AlignTop);
        mHorizontalLayout->addWidget(mLeftToolbar);
        mHorizontalLayout->addWidget(mRightToolbar);
        mVerticalLayout->addWidget(widget, Qt::AlignBottom);
        widget->show();
        connect(widget, &ContentWidget::name_changed, this, &ContentFrame::handleNameChanged);
    }

    void ContentFrame::childEvent(QChildEvent* event)
    {
        if (event->removed() && event->child() == mWidget)
        {
            hide();
            setParent(nullptr);
            deleteLater();
        }

        if (event->FocusIn != 0)
        {
            // add debug code to show focus
        }

        if (event->FocusOut != 0)
        {
            // add debug code to show focus
        }
    }

    ContentWidget* ContentFrame::content()
    {
        return mWidget;
    }

    void ContentFrame::detachWidget()
    {
        hide();
        mWidget->detach();
    }

    void ContentFrame::reattachWidget()
    {
        hide();
        mWidget->reattach();
    }

    void ContentFrame::handleNameChanged(const QString &name)
    {
        mNameLabel->setText(name);
        setWindowTitle(name);
        //mNameLabel->update();
    }

    void ContentFrame::closeEvent(QCloseEvent* event)
    {
        Q_UNUSED(event)
        reattachWidget();
    }
}
