#include "content_frame/content_frame.h"

#include "content_widget/content_widget.h"
#include "gui_utils/graphics.h"
#include "toolbar/toolbar.h"

#include <QAction>
#include <QChildEvent>
#include <QShortcut>

namespace hal
{
    ContentFrame::ContentFrame(ContentWidget* widget, bool attached, QWidget* parent)
        : QWidget(parent), m_vertical_layout(new QVBoxLayout()), m_horizontal_layout(new QHBoxLayout()), m_left_toolbar(new Toolbar()), m_right_toolbar(new Toolbar()), m_widget(widget),
          m_name_label(new QLabel())

    {
        setWindowTitle(widget->name());
        setFocusPolicy(Qt::FocusPolicy::StrongFocus);

        m_vertical_layout->setContentsMargins(0, 0, 0, 0);
        m_vertical_layout->setSpacing(0);

        m_horizontal_layout->setContentsMargins(0, 0, 0, 0);
        m_horizontal_layout->setSpacing(0);

        m_left_toolbar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        m_left_toolbar->setIconSize(QSize(18, 18));
        m_right_toolbar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_right_toolbar->setIconSize(QSize(18, 18));

        QAction* action = new QAction(this);

        m_detach_icon_style = "all->#969696";
        m_detach_icon_path  = ":/icons/detach";

        if (attached)
        {
            m_name_label->setText(widget->name());
            m_left_toolbar->addWidget(m_name_label);
            m_left_toolbar->addSeparator();
            action->setText("Detach");
            action->setIcon(gui_utility::get_styled_svg_icon(m_detach_icon_style, m_detach_icon_path));
            connect(action, &QAction::triggered, this, &ContentFrame::detach_widget);
        }
        else
        {
            action->setText("Reattach");
            connect(action, &QAction::triggered, this, &ContentFrame::reattach_widget);
        }
        widget->setup_toolbar(m_left_toolbar);
        m_right_toolbar->addAction(action);

        for (QShortcut* s : widget->create_shortcuts())
        {
            s->setParent(this);
            s->setContext(Qt::WidgetWithChildrenShortcut);
            s->setEnabled(true);
        }

        setLayout(m_vertical_layout);
        m_vertical_layout->addLayout(m_horizontal_layout, Qt::AlignTop);
        m_horizontal_layout->addWidget(m_left_toolbar);
        m_horizontal_layout->addWidget(m_right_toolbar);
        m_vertical_layout->addWidget(widget, Qt::AlignBottom);
        widget->show();
    }

    void ContentFrame::childEvent(QChildEvent* event)
    {
        if (event->removed() && event->child() == m_widget)
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
        return m_widget;
    }

    void ContentFrame::detach_widget()
    {
        hide();
        m_widget->detach();
    }

    void ContentFrame::reattach_widget()
    {
        hide();
        m_widget->reattach();
    }

    void ContentFrame::closeEvent(QCloseEvent* event)
    {
        Q_UNUSED(event)
        reattach_widget();
    }
}
